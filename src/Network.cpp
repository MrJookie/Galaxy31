#include "Network.hpp"
#include <enet/enet.h>
#include <iostream>
#include <chrono>
#include "GameState.hpp"

//gui
#include "controls/TextBox.hpp"
#include "controls/Terminal.hpp"

#include "EventSystem/Event.hpp"
#include "commands/commands.hpp"


#include "server/network.hpp"
#include "Packet.hpp"
#include "Object.hpp"

using std::cout;
using std::endl;

namespace Network {
	ENetHost *client;
	ENetPeer *host;
	
	std::vector<Object> objects_to_send;
	
	uint32_t server_ticks;
	
	// TODO: implement
	struct Frame {
		uint32_t tick;
		// contains all objects that have changed its state in given frame
		std::vector<Object> objects;
	};
	std::vector<Frame> frames;
	// ----------
	
	const int ping_interval = 50;
	
	// private forwards
	void parse_packet(ENetPeer* peer, ENetPacket* pkt);
	void display_packets();
	//
	int evt_lag;
	int evt_disconnected;
	int lst_packets;
	const int timeout = 5000;
	void initialize() {
		if (enet_initialize () != 0) {
			cout << "An error occurred while initializing ENet." << endl;
			return;
		}
		atexit (enet_deinitialize);
		
		evt_lag = Event::Register("lag");
		evt_disconnected = Event::Register("disconnected");
		lst_packets = Event::Listen("timer", display_packets, 1.0);
	}
	
	
	bool connect(const char* ip, int port) {
		cout << "Connecting to " << ip << ":" << port << endl;
		
		ENetAddress address;
		if(enet_address_set_host(&address, ip) != 0) {
			throw std::string("Failed to resolve ip address.");
		}
		address.port = port;
		
		client = enet_host_create (NULL, 1, Channel::num_channels, 0, 0);
		if(client == nullptr) {
			throw std::string("An error occurred while trying to create an ENet client host.");
		}
		
		ENetPeer* peer;
		ENetEvent event;
		peer = enet_host_connect(client, &address, Channel::num_channels, 0);
		if(peer == NULL) {
		   throw std::string("No available peers for initiating an ENet connection.");
		}
		host = peer;

		if(enet_host_service (client, &event, timeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
			cout << "Connection to succeeded." << endl;

			enet_host_flush (client);
			enet_peer_ping_interval(host, ping_interval);
		} else {
			enet_peer_reset (peer);
			
			throw std::string("Connection failed.");
		}
		return true;
	}
	
	void flush() {
		enet_host_flush(client);
	}
	
	// process n events (set n to big number to process all events)
	void handle_events(int n) {
		flush();
		ENetEvent event;
		for(int i=0; i < n && enet_host_service(client, &event, 1) > 0; i++) {
			switch(event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				parse_packet(event.peer, event.packet);
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT: 
				Event::Emit(evt_disconnected);
				enet_peer_reset(event.peer);
				break;
			default:
				cout << "event: " << event.type << endl;
			}
		}
	}
	
	std::chrono::high_resolution_clock::time_point last_time_state_sent = std::chrono::high_resolution_clock::now();
	auto send_states_period = std::chrono::milliseconds(15);
	void SendOurState() {
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		
		if(now - last_time_state_sent > send_states_period) {
			last_time_state_sent = now;
		} else {
			return;
		}
		
		
		Packet p;
		p.put("type", PacketType::update_objects);
		p.put("resource_money", GameState::resource_money);
		p.put("num_objects", 1);
		
		// send ship state
		Object* obj = GameState::player;
		GameState::player->SetOwner(GameState::user_id);
		obj->UpdateTicks();
		memcpy(p.allocate("objects",sizeof(Object)), obj, sizeof(Object));
		
		// send new projectiles
		uint32_t owner = GameState::player->GetId();
		p.put("num_static_objects", objects_to_send.size());
		if(objects_to_send.size() > 0) {
			Object* objects = (Object*)p.allocate("static_objects", sizeof(Object)*objects_to_send.size());
			for(auto& o : objects_to_send) {
				o.SetOwner(owner);
				*objects++ = o;
			}
			objects_to_send.clear();
		}
		
		
		p.send(host, Channel::data, 0);
	}
	
	void SendAuthentication(std::string user_email, std::string user_password) {
		CryptoPP::SHA1 sha1;
		std::string source = user_password;
		std::string hash = "";
		CryptoPP::StringSource(source, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), false)));
		
		std::string hashChallenge = hash + std::to_string(GameState::account_challenge);
		std::string finalHash = "";
		CryptoPP::StringSource(hashChallenge, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(finalHash), false)));
		user_password = finalHash;
		
		Packet s;
		s.put("type", PacketType::authenticate);
		s.put("user_email", user_email);
		s.put("user_password", user_password);
		s.send(host, Channel::control, ENET_PACKET_FLAG_RELIABLE);
	}
	
	void SendRegistration(std::string user_email, std::string user_name, std::string user_password) {
		CryptoPP::SHA1 sha1;
		std::string source = user_password;
		std::string hash = "";
		CryptoPP::StringSource(source, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), false)));
		user_password = hash;
		
		std::string plain_cut = user_password.substr(0, RSA_MAX_PLAIN_LEN); 
		std::string encrypted;
		
		CryptoPP::AutoSeededRandomPool rng;
		
		CryptoPP::RSA::PublicKey loadPublicKey;
		loadPublicKey.Load(CryptoPP::StringSource(GameState::serverPublicKeyStr, true, new CryptoPP::HexDecoder()).Ref());
		
		CryptoPP::RSAES_OAEP_SHA_Encryptor e(loadPublicKey);
		CryptoPP::StringSource ss1(plain_cut, true, new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::StringSink(encrypted)));
		
		Packet s;
		s.put("type", PacketType::signup);
		s.put("user_email", user_email);
		s.put("user_name", user_name);
		s.put("user_password", user_password);
		s.send(host, Channel::control, ENET_PACKET_FLAG_RELIABLE);
	}
	
	void SendGoodBye() {
		enet_peer_disconnect_now(host, 0);
	}
	
	
	
	void cleanup() {
		enet_host_destroy(client);
	}
	
	
	void QueueObject(Object* o) {
		o->UpdateTicks();
		objects_to_send.push_back(*o);
	}
	
	
	
	// ------------------ [ PROCESSING ] -------------------
	
	// auto tp = std::chrono::high_resolution_clock::now();
	/*
	int n_dtime = 0;
	std::array<double, 100> dtime_arr;
	double mean;
	*/
	bool wait_for_packets = false;
	int min_packets = 5;
	int max_packets_delayed = 10;
	
	COMMAND(void, packets_range, (int min, int max)) {
		max_packets_delayed = max;
		min_packets = min;
	}
	COMMAND(void, send_states_frequency, (int fq)) {
		send_states_period = std::chrono::milliseconds( 1000/fq );
	}
	
	void Process() {
		
		// auto now = std::chrono::high_resolution_clock::now();
		// uint32_t dtime = std::chrono::duration_cast<std::chrono::microseconds>(now - tp).count();
		// tp = now;
		
		// extern std::map< unsigned int, std::pair<Ship*, std::queue<Object>> > ships;
		// (id, (current, next states queue))
		double dtime = GameState::deltaTime*1000000.0;
		/*
		if(n_dtime >= dtime_arr.size()) {
			double s = 0;
			for(auto d : dtime_arr) {
				s += d;
			}
			mean = s / (double)dtime_arr.size();
			
			n_dtime = 0;
		} else {
			dtime_arr[n_dtime++] = dtime;
		}
		GameState::debug_string += "dtime mean: " + std::to_string(mean) + "\n";
		*/
		
		for(auto& obj : GameState::enemyShips) {
			auto& p = obj.second;
			auto &queue = p.second;
			auto &current = p.first;
			// cout << "id: " << obj.first << ", packets: " << p.second.size() << endl;
			GameState::debug_fields["Packets[" + std::to_string(obj.first) + "]"] = std::to_string(queue.size());
			
			if(wait_for_packets) {
				if(queue.size() >= max_packets_delayed/2)
					wait_for_packets = false;
				break;
			}
			else if(queue.size() < min_packets)
				wait_for_packets = true;
			
			while(queue.size() > max_packets_delayed) {
				while(queue.size() > max_packets_delayed/2) queue.pop();
				current->CopyObjectState(queue.front());
				queue.pop();
				Event::Emit("lag", true);
			}
			
			while(queue.size() > 1 && queue.front().GetTicks() <= current->GetTicks()) {
				// std::cout << "poping : " << p.second.size() << std::endl;
				queue.pop();
			}
			
			if(!p.second.empty() && queue.front().GetTicks() <= current->GetTicks()) {
				// std::cout << "copy state\n";
				current->CopyObjectState(queue.front());
				queue.pop();
			}
			
			// cout << "dtime: " << dtime << endl;
			if(queue.empty()) {
				// no packets, try predict, but should trigger lag effect
				Event::Emit("lag", false);
				// ((Object*)p.first)->Process();
			} else {
				double diff = queue.front().GetTicks()+dtime - current->GetTicks();
				if(diff > dtime)
					current->InterpolateToState(p.second.front(), dtime / diff );
				current->AddTicks( dtime+1 );
			}
			
		}
	}
	
	
	
	// ------------------ [ PARSING PACKETS ] --------------------------
	
	
	int num_packets = 0;
	int data_size = 0;
	void display_packets() {
		GameState::debug_fields["incoming packets/s"] = std::to_string(num_packets);
		GameState::debug_fields["incoming B/s"] = std::to_string(data_size);
		num_packets = 0;
		data_size = 0;
	}
	
	int max_queue = 10000;
	Ship::Chassis *chassis = nullptr;
	void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
		num_packets++;
		Packet p(pkt);
		data_size += pkt->dataLength;
		switch(p.get_int("type")) {
			case PacketType::new_client: {
				cout << "your client id is: " << p.get_int("new_id") << ", challenge: " << p.get_int("challenge") << endl;
				GameState::player->SetId(p.get_int("new_id"));
				GameState::account_challenge = p.get_int("challenge"); //move this?
				GameState::serverPublicKeyStr = p.get_string("public_key"); 
					
				break;
			}
			case PacketType::update_objects: {
				int num_objects = p.get_int("num_objects");
				Object* objs = (Object*)p.get_pair("objects").first;
				
				//cout << "updating objects " << p->num_objects << endl;
				for(int i=0; i < num_objects; i++) {
					Object &o = objs[i];
					if(GameState::player->GetId() == o.GetId())  {
						//cout << "id: " << o.GetId() << endl;
						continue;
					}

					if(GameState::enemyShips.find(o.GetId()) == GameState::enemyShips.end()) {
						cout << "added new ship" << endl;
						if(!chassis)
							chassis = new Ship::Chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
						Ship *s = new Ship({0,0}, 0, *chassis);
						s->CopyObjectState(o);
						//cout << "id: " << s->GetId() << ", owner: " << s->GetOwner() << endl;
						GameState::enemyShips[o.GetId()] = std::pair<Ship*, std::queue<Object>>(s, std::queue<Object>());
					} else {
						std::queue<Object> &q = GameState::enemyShips[o.GetId()].second;
						while(q.size() > max_queue)
							q.pop();
						
						q.push(o);
					}
				}
				
				// add new projectiles fired
				int num_static_objects = p.get_int("num_static_objects");
				if(num_static_objects > 0) {
					Object* obj = (Object*)p.get_pair("static_objects").first;
					for(int i=0; i < num_static_objects; i++) {
						Object& o = obj[i];
						if(o.GetOwner() != GameState::player->GetId()) {
							const Asset::Texture& texture = GameState::asset.GetTexture("projectile.png");
							Projectile proj(texture);
							proj.CopyObjectState(o);
							GameState::projectiles.push_back(proj);
						}
					}
				}
				// cout << "ping: " << peer->roundTripTime << endl;
				GameState::debug_string += "ping: " + std::to_string(peer->roundTripTime) + "\n";
				break;
			}
			case PacketType::authorize: {
				GameState::client_id = p.get_int("client_id");
				GameState::user_id = p.get_int("user_id");
				GameState::user_name = p.get_string("user_name");
				GameState::resource_money = p.get_int("resource_money");
				
				ng::Label* lb_bar_basic_money = (ng::Label*)GameState::gui.GetControlById("game_bar_basic_money"); //move this?
				lb_bar_basic_money->SetText(std::to_string(GameState::resource_money));
				
				GameState::player->SetOwner(GameState::user_id);
				if(p.get_int("status_code") == status_code::login_ok) { //login ok after registration login
					NetworkChat::connect(p.get_string("chat_ip").c_str(), p.get_int("chat_port")); //connect to chat server
					NetworkChat::SendChatLogin(GameState::user_id, GameState::user_name);
					GameState::set_gui_page("game");
				} else if(p.get_int("status_code") == status_code::email_exist) {
					ng::TextBox* tb_register_status = (ng::TextBox*)GameState::gui.GetControlById("register_status"); //move this?
					tb_register_status->SetText("Error email exists!");
					
					GameState::set_gui_page("register");
				} else if(p.get_int("status_code") == status_code::username_taken) {
					ng::TextBox* tb_register_status = (ng::TextBox*)GameState::gui.GetControlById("register_status"); //move this?
					tb_register_status->SetText("Username taken!");
					
					GameState::set_gui_page("register");
				} else if(p.get_int("status_code") == 3) { //login ok after just login ////REMOVE?
					GameState::set_gui_page("game");
				} else if(p.get_int("status_code") == status_code::error_logging_in) {
					ng::TextBox* tb_login_status = (ng::TextBox*)GameState::gui.GetControlById("login_status"); //move this?
					tb_login_status->SetText("Error logging in!");
					GameState::set_gui_page("login");
				}
				break;
			}
			case PacketType::player_removed: {
				unsigned int client_id = p.get_int("client_id");
				unsigned int user_id = p.get_int("user_id");
				
				cout << "removing player: " << client_id << endl;
				
				for(const auto& control : GameState::enemyShipsHUD[client_id]) {
					GameState::gui.RemoveControl(control);
				}
				
				GameState::enemyShips[client_id].first->Destroy(); //removes propulsion and ship from drawing
				
				//cleanup all user's projectiles (What if ship is erased and projectiles are flying and hit player, then user_id is not allocated anymore)
				//remove from drawing aswell
				for(auto it = GameState::projectiles.begin(); it != GameState::projectiles.end(); it++) {
					if(it->GetOwner() == user_id) {
						it->GetSprite()->RemoveFromDrawing();
						
						it = GameState::projectiles.erase(it);
						if(it == GameState::projectiles.end()) break;
					}
				}
				
				GameState::enemyShips.erase(client_id);
				GameState::enemyShipsHUD.erase(client_id);
				
				break;
			}
		}
	}
}






// ----------------------------[ Chat ]---------------------
namespace NetworkChat {
	ENetHost *client;
	ENetPeer *host;
	
	// private forwards
	void parse_packet(ENetPeer* peer, ENetPacket* pkt);
	//
	
	const int timeout = 5000;
	void initialize() {
		if (enet_initialize () != 0) {
			cout << "An error occurred while initializing ENet." << endl;
			return;
		}
		atexit (enet_deinitialize);
	}
	
	bool connect(const char* ip, int port) {
		cout << "Connecting to " << ip << ":" << port << endl;
		
		ENetAddress address;
		if(enet_address_set_host(&address, ip) != 0) {
			throw std::string("Failed to resolve ip address.");
		}
		address.port = port;
		
		client = enet_host_create (NULL, 1, Channel::num_channels, 0, 0);
		if(client == nullptr) {
			throw std::string("An error occurred while trying to create an ENet client host.");
		}
		
		ENetPeer* peer;
		ENetEvent event;
		peer = enet_host_connect(client, &address, Channel::num_channels, 0);
		if(peer == NULL) {
		   throw std::string("No available peers for initiating an ENet connection.");
		}
		host = peer;

		if(enet_host_service (client, &event, timeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
			cout << "Connection to succeeded." << endl;

			enet_host_flush (client);
			enet_peer_ping_interval(host, 50);
		} else {
			enet_peer_reset (peer);
			
			throw std::string("Connection failed.");
		}
		return true;
	}

	void flush() {
		enet_host_flush(client);
	}
	
	// process n events (set n to big number to process all events)
	void handle_events(int n) {
		flush();
		ENetEvent event;
		for(int i=0; i < n && enet_host_service(client, &event, 1) > 0; i++) {
			switch(event.type) {
			case ENET_EVENT_TYPE_RECEIVE:
				parse_packet(event.peer, event.packet);
				enet_packet_destroy(event.packet);
				break;
			default:
				cout << "event: " << event.type << endl;
			}
		}
	}

	void SendChatLogin(unsigned int user_id, std::string user_name) {
		try {
			Packet s;
			s.put("type", PacketType::chat_login);
			s.put("user_id", user_id);
			s.put("user_name", user_name);
			s.put("public_key", GameState::clientPublicKeyStr);
			s.send(host, Channel::control, ENET_PACKET_FLAG_RELIABLE);
			
			flush();
		} catch(...) {}
	}
	
	void SendChatMessage(std::string to_user_name, std::string message) {
		if(to_user_name.length() < 11 /*&& message.length() < 101*/) {
			try {
				std::string plainMessage = message.substr(0, AES_MAX_MESSAGE_LEN - 1); //must be -1 (127B, otherwise 128 padds to +16 bytes -> 144), so if msg is max 100 chars, it encrypts to 112, which fits to p->message
				std::string encryptedMessage;
				
				CryptoPP::ECB_Mode< CryptoPP::AES >::Encryption e;
				e.SetKey(GameState::server_chatAESkey.data(), GameState::server_chatAESkey.size() - 1);

				CryptoPP::StringSource ss(plainMessage, true, new CryptoPP::StreamTransformationFilter(e, new CryptoPP::StringSink(encryptedMessage)));
				
				Packet p;
				p.put("message", encryptedMessage);
				p.put("to_username", to_user_name);
				p.put("type", PacketType::chat_message);
				p.send(host, Channel::msg, ENET_PACKET_FLAG_RELIABLE);
				
				flush();
			} catch(...) {}
		}
	}
	
	void cleanup() {
		enet_host_destroy(client);
	}
	
	int max_queue = 10000;
	//Ship::Chassis *chassis = nullptr;
	void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
		Packet p(pkt);
		
		switch(p.get_int("type")) {
			case PacketType::chat_login_response: {
				CryptoPP::AutoSeededRandomPool rng;
					
				std::string encrypted = p.get_string("AES_key");
				std::string decrypted;
				
				if(encrypted.length() == RSA_MAX_ENCRYPTED_LEN) {
					try {
						CryptoPP::RSA::PrivateKey privateKey;
						privateKey.Load(CryptoPP::StringSource(GameState::clientPrivateKeyStr, true, new CryptoPP::HexDecoder()).Ref());
						
						CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
						CryptoPP::StringSource ss2(encrypted, true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::StringSink(decrypted)));
						
						std::string nonHexKey;
						CryptoPP::ArraySource(reinterpret_cast<const unsigned char*>(decrypted.data()), decrypted.size(), true, new CryptoPP::HexDecoder(new CryptoPP::StringSink(nonHexKey)));
						
						if(nonHexKey.length() == AES_KEY_SIZE) {
							memcpy(GameState::server_chatAESkey.data(), nonHexKey.data(), nonHexKey.length() + 1);
						}
					} catch(...) {}
				}
								
				break;
			}
			case PacketType::chat_message: {
				std::string encrypted = p.get_string("message");
				std::string decrypted;
				
				if(encrypted.length() % AES_KEY_SIZE == 0) {
					try {
						CryptoPP::ECB_Mode< CryptoPP::AES >::Decryption d;
						d.SetKey(GameState::server_chatAESkey.data(), GameState::server_chatAESkey.size() - 1);

						CryptoPP::StringSource ss(encrypted, true, new CryptoPP::StreamTransformationFilter(d, new CryptoPP::StringSink(decrypted)));
						
						ng::Terminal* tm_game_chat = (ng::Terminal*)GameState::gui.GetControlById("game_terminal"); //move this?
						if(p.get_int("message_type") == 0) {
							tm_game_chat->WriteLog( p.get_string("from_username") + ": " + decrypted + "^w" );
						} else if(p.get_int("message_type") == 1) {
							tm_game_chat->WriteLog( "^y[pm from " + p.get_string("from_username") + "]^w: " + decrypted + "^w" );
						}
					} catch(...) {}
				}
				
				break;
			}
		}
	}
}
