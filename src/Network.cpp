#include "Network.hpp"
#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include "GameState.hpp"
#include <chrono>

//gui
#include "controls/TextBox.hpp"
#include "controls/Terminal.hpp"

using namespace ng;

using std::cout;
using std::endl;
#include "server/network.hpp"
#include "Packet.hpp"

namespace Network {
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
	
	std::chrono::high_resolution_clock::time_point last_time_state_sent = std::chrono::high_resolution_clock::now();
	void SendOurState() {
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		
		if(now - last_time_state_sent > std::chrono::milliseconds(15)) {
			last_time_state_sent = now;
		} else {
			return;
		}
		
		Packet p;
		p.put("type", PacketType::update_objects);
		p.put("num_objects", 1);
		
		Object* obj = GameState::player;
		obj->UpdateTicks();
		memcpy(p.allocate("objects",sizeof(Object)), obj, sizeof(Object));
		p.send(host, Channel::data, 0);
		// int len = sizeof(Packet::update_objects) + sizeof(Object);
		// ENetPacket* pkt = enet_packet_create(nullptr, len, 0);
		// Packet::update_objects *p = new (pkt->data) Packet::update_objects;
		// p->num_objects = 1;
		// memcpy(pkt->data+sizeof(Packet::update_objects), obj, sizeof(Object));
		// enet_peer_send(host, Channel::data, pkt);
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
	
	void cleanup() {
		enet_host_destroy(client);
	}
	
	int max_queue = 10000;
	Ship::Chassis *chassis = nullptr;
	void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
		Packet p(pkt);
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
						
						cout << "id: " << o.GetId() << endl;
						continue;
					}

					if(GameState::ships.find(o.GetId()) == GameState::ships.end()) {
						cout << "added new ship" << endl;
						if(!chassis)
							chassis = new Ship::Chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
						Ship *s = new Ship({0,0}, 0, *chassis);
						s->CopyObjectState(o);
						GameState::ships[o.GetId()] = std::pair<Ship*, std::queue<Object>>(s, std::queue<Object>());
					} else {
						std::queue<Object> &q = GameState::ships[o.GetId()].second;
						while(q.size() > max_queue)
							q.pop();
						
						q.push(o);
					}
				}
				// cout << "ping: " << peer->roundTripTime << endl;
				break;
			}
		/*
			case PacketType::update_objects: {
				Packet::update_objects *p = (Packet::update_objects *)pkt->data;
				if(p->num_objects * sizeof(Object) > pkt->dataLength) return;
				Object* objs = (Object*)(pkt->data+sizeof(Packet::update_objects));
				//cout << "updating objects " << p->num_objects << endl;
				for(int i=0; i < p->num_objects; i++) {
					Object &o = objs[i];
					
					if(GameState::player->GetId() == o.GetId()) continue;
					
					
					if(GameState::ships.find(o.GetId()) == GameState::ships.end()) {
						cout << "added new ship" << endl;
						if(!chassis)
							chassis = new Ship::Chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
						Ship *s = new Ship({0,0}, 0, *chassis);
						s->CopyObjectState(o);
						GameState::ships[o.GetId()] = std::pair<Ship*, std::queue<Object>>(s, std::queue<Object>());
					} else {
						std::queue<Object> &q = GameState::ships[o.GetId()].second;
						while(q.size() > max_queue)
							q.pop();
						
						q.push(o);
					}
				}
				// cout << "ping: " << peer->roundTripTime << endl;
				break;
			}
			*/
			case PacketType::authorize: {
				GameState::user_id = p.get_int("user_id");
				GameState::user_name = p.get_string("user_name");
				
				if(p.get_int("status_code") == 0) { //login ok after registration login
					NetworkChat::connect("89.177.76.215", 54301); //connect to chat server
					NetworkChat::SendChatLogin(GameState::user_id, GameState::user_name);
					
					TextBox* tb_game_account = (TextBox*)GameState::gui.GetControlById("game_account"); //move this?
					tb_game_account->SetText("Logged in as: " + std::to_string(GameState::user_id) + " | " + GameState::user_name);
					
					GameState::activePage = "game";
				} else if(p.get_int("status_code") == 1) {
					TextBox* tb_register_status = (TextBox*)GameState::gui.GetControlById("register_status"); //move this?
					tb_register_status->SetText("Error email exists!");
					
					GameState::activePage = "register";
				} else if(p.get_int("status_code") == 2) {
					TextBox* tb_register_status = (TextBox*)GameState::gui.GetControlById("register_status"); //move this?
					tb_register_status->SetText("Username taken!");
					
					GameState::activePage = "register";
				} else if(p.get_int("status_code") == 3) { //login ok after just login ////REMOVE?
					TextBox* tb_game_account = (TextBox*)GameState::gui.GetControlById("game_account"); //move this?
					tb_game_account->SetText("Logged in as: " + std::to_string(GameState::user_id) + " | " + GameState::user_name);
					
					GameState::activePage = "game";
				} else if(p.get_int("status_code") == 4) {
					TextBox* tb_login_status = (TextBox*)GameState::gui.GetControlById("login_status"); //move this?
					tb_login_status->SetText("Error logging in!");
					
					GameState::activePage = "login";
				}
				break;
			}
		}
	}
}

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
						
						Terminal* tm_game_chat = (Terminal*)GameState::gui.GetControlById("game_terminal"); //move this?
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
