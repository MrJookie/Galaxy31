#include "Network.hpp"
#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include "GameState.hpp"
#include <chrono>

//gui
#include "controls/TextBox.hpp"
#include "controls/Terminal.hpp"

//crypto
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <cryptopp/secblock.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

using namespace ng;

using std::cout;
using std::endl;
#include "server/network.hpp"

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
			cout << "failed to resolve ip address" << endl;
			return false;
		}
		address.port = port;
		
		client = enet_host_create (NULL, 1, Channel::num_channels, 0, 0);
		if (client == NULL)
		{
			cout << "An error occurred while trying to create an ENet client host." << endl;
			return false;
		}
		
		ENetPeer* peer;
		ENetEvent event;
		/* Initiate the connection, allocating the two channels 0 and 1. */
		peer = enet_host_connect(client, &address, Channel::num_channels, 0);
		if (peer == NULL)
		{
		   cout << "No available peers for initiating an ENet connection." << endl;
		   return false;
		}
		host = peer;
		/* Wait up to 5 seconds for the connection attempt to succeed. */
		if (enet_host_service (client, &event, timeout) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT)
		{
			cout << "Connection to " << ip << ":" << port << " succeeded." << endl;
			enet_host_flush (client);
			enet_peer_ping_interval(host, 50);
		} else {
			enet_peer_reset (peer);
			cout << "Connection to " << ip << ":" << port << " failed." << endl;
			return false;
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
		
		if(now - last_time_state_sent > std::chrono::milliseconds(10)) {
			last_time_state_sent = now;
		} else {
			return;
		}
			
		Object* obj = GameState::player;
		obj->UpdateTicks();
		int len = sizeof(Packet::update_objects) + sizeof(Object);
		ENetPacket* pkt = enet_packet_create(nullptr, len, 0);
		Packet::update_objects *p = new (pkt->data) Packet::update_objects;
		p->num_objects = 1;
		memcpy(pkt->data+sizeof(Packet::update_objects), obj, sizeof(Object));
		enet_peer_send(host, Channel::data, pkt);
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
		
		ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::authenticate), ENET_PACKET_FLAG_RELIABLE);
		Packet::authenticate *p = new (pkt->data) Packet::authenticate();
		
		// && user_password.length() < sizeof(p->user_password)) // not needed, since sha1 is 40 chars always, thus will fit 40+1 null terminator
		if(user_email.length() < sizeof(p->user_email)) {
			strcpy(p->user_email.data(), user_email.c_str());
			strcpy(p->user_password.data(), user_password.c_str());
						
			enet_peer_send(host, Channel::control, pkt);
			flush();
		} else {
			enet_packet_destroy(pkt);
		}
	}
	
	void SendRegistration(std::string user_email, std::string user_name, std::string user_password) {
		CryptoPP::SHA1 sha1;
		std::string source = user_password;
		std::string hash = "";
		CryptoPP::StringSource(source, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), false)));
		user_password = hash;
		
		std::string plain_cut = user_password.substr(0, MAX_PLAIN_LEN); 
		std::string encrypted;
		
		CryptoPP::AutoSeededRandomPool rng;
		
		CryptoPP::RSA::PublicKey loadPublicKey;
		loadPublicKey.Load(CryptoPP::StringSource(GameState::serverPublicKeyStr, true, new CryptoPP::HexDecoder()).Ref());
		
		CryptoPP::RSAES_OAEP_SHA_Encryptor e(loadPublicKey);
		CryptoPP::StringSource ss1(plain_cut, true, new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::StringSink(encrypted)));

		ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::signup), ENET_PACKET_FLAG_RELIABLE);
		Packet::signup *p = new (pkt->data) Packet::signup();
		
		if(user_email.length() < sizeof(p->user_email)) {
			strcpy(p->user_email.data(), user_email.c_str());
			strcpy(p->user_name.data(), user_name.c_str());
			memcpy(p->user_password.data(), encrypted.c_str(), encrypted.length() + 1);
						
			enet_peer_send(host, Channel::control, pkt);
			flush();
		} else {
			enet_packet_destroy(pkt);
		}
	}
	
	void cleanup() {
		enet_host_destroy(client);
	}
	
	int max_queue = 10000;
	Ship::Chassis *chassis = nullptr;
	void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
		Packet::Packet *bp = (Packet::Packet *)pkt->data;
		switch(bp->type) {
			case PacketType::new_client: {
				Packet::new_client *p = (Packet::new_client *)pkt->data;
				cout << "your client id is: " << p->new_id << ", challenge: " << p->challenge << endl;
				GameState::player->SetId(p->new_id);
				GameState::account_challenge = p->challenge; //move this?
				GameState::serverPublicKeyStr = p->public_key.data(); 
							
				break;
			}
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
			case PacketType::authorize: {
				Packet::authorize *p = (Packet::authorize *)pkt->data;
				
				GameState::user_id = p->user_id;
				GameState::user_name = p->user_name.data();
				
				if(p->status_code == 0) { //login ok after registration login
					NetworkChat::connect("89.177.76.215", 54301); //connect to chat server
					NetworkChat::SendChatLogin(GameState::user_id, GameState::user_name);
					
					TextBox* tb_game_account = (TextBox*)GameState::gui.GetControlById("game_account"); //move this?
					tb_game_account->SetText("Logged in as: " + std::to_string(GameState::user_id) + " | " + GameState::user_name);
					
					GameState::activePage = "game";
				} else if(p->status_code == 1) {
					TextBox* tb_register_status = (TextBox*)GameState::gui.GetControlById("register_status"); //move this?
					tb_register_status->SetText("Error email exists!");
					
					GameState::activePage = "register";
				} else if(p->status_code == 2) {
					TextBox* tb_register_status = (TextBox*)GameState::gui.GetControlById("register_status"); //move this?
					tb_register_status->SetText("Username taken!");
					
					GameState::activePage = "register";
				} else if(p->status_code == 3) { //login ok after just login ////REMOVE?
					TextBox* tb_game_account = (TextBox*)GameState::gui.GetControlById("game_account"); //move this?
					tb_game_account->SetText("Logged in as: " + std::to_string(GameState::user_id) + " | " + GameState::user_name);
					
					GameState::activePage = "game";
				} else if(p->status_code == 4) {
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
			cout << "failed to resolve ip address" << endl;
			return false;
		}
		address.port = port;
		
		client = enet_host_create (NULL, 1, Channel::num_channels, 0, 0);
		if (client == NULL)
		{
			cout << "An error occurred while trying to create an ENet client host." << endl;
			return false;
		}
		
		ENetPeer* peer;
		ENetEvent event;
		/* Initiate the connection, allocating the two channels 0 and 1. */
		peer = enet_host_connect(client, &address, Channel::num_channels, 0);
		if (peer == NULL)
		{
		   cout << "No available peers for initiating an ENet connection." << endl;
		   return false;
		}
		host = peer;
		/* Wait up to 5 seconds for the connection attempt to succeed. */
		if (enet_host_service (client, &event, timeout) > 0 &&
			event.type == ENET_EVENT_TYPE_CONNECT)
		{
			cout << "Connection to " << ip << ":" << port << " succeeded." << endl;
			enet_host_flush (client);
			enet_peer_ping_interval(host, 50);
		} else {
			enet_peer_reset (peer);
			cout << "Connection to " << ip << ":" << port << " failed." << endl;
			return false;
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
		ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_login), ENET_PACKET_FLAG_RELIABLE);
		Packet::chat_login *p = new (pkt->data) Packet::chat_login();
		
		p->user_id = user_id;
		strcpy(p->user_name.data(), user_name.c_str());
		strcpy(p->public_key.data(), GameState::clientPublicKeyStr.c_str());
						
		enet_peer_send(host, Channel::control, pkt);
		flush();
	}
	
	void SendChatMessage(std::string to_user_name, std::string message) {
		ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_message), ENET_PACKET_FLAG_RELIABLE);
		Packet::chat_message *p = new (pkt->data) Packet::chat_message();
		
		if(to_user_name.length() < 11 && message.length() < 101) {
			strcpy(p->to_user_name.data(), to_user_name.c_str());
			strcpy(p->message.data(), message.c_str());
							
			enet_peer_send(host, Channel::msg, pkt);
			flush();
		} else {
			enet_packet_destroy(pkt);
		}
	}
	
	void cleanup() {
		enet_host_destroy(client);
	}
	
	int max_queue = 10000;
	Ship::Chassis *chassis = nullptr;
	void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
		Packet::Packet *bp = (Packet::Packet *)pkt->data;
		switch(bp->type) {
			case PacketType::chat_login_response: {
				Packet::chat_login_response *p = (Packet::chat_login_response *)pkt->data;
				
				//decryption
				CryptoPP::AutoSeededRandomPool rng;
					
				std::string encrypted(p->AES_key.data(), MAX_ENCRYPTED_LEN);
				std::string decrypted;
				
				if(encrypted.length() == MAX_ENCRYPTED_LEN) {
					CryptoPP::RSA::PrivateKey privateKey;
					privateKey.Load(CryptoPP::StringSource(GameState::clientPrivateKeyStr, true, new CryptoPP::HexDecoder()).Ref());
					
					CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
					CryptoPP::StringSource ss2(encrypted, true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::StringSink(decrypted)));
					
					std::cout << "decrypted.size(): " << decrypted.size() << " -- " << decrypted << std::endl;
					
					std::string nonHexKey;
        
					CryptoPP::ArraySource(reinterpret_cast<const unsigned char*>(decrypted.data()), decrypted.size(), true, new CryptoPP::HexDecoder(new CryptoPP::StringSink(nonHexKey)));

					unsigned char AESkey[17];
					memcpy(AESkey, nonHexKey.data(), nonHexKey.length() + 1);
					
					//send msg with aes + iv
					std::string plain = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed eget suscipit libero. Maecenas fringilla lacinia lacus, eget metus.";
					std::string plainMessage = plain.substr(0, MAX_AES_MESSAGE_LEN - 1); //must be -1 (127B, otherwise 128 padds to +16 bytes -> 144)

					std::string encryptedMessage;
					
					
					//remove iv?
					//unsigned char iv[CryptoPP::AES::BLOCKSIZE];
					//rng.GenerateBlock(iv, sizeof(iv));
					
					CryptoPP::ECB_Mode< CryptoPP::AES >::Encryption e;
					e.SetKey( AESkey, sizeof(AESkey) - 1 );

					CryptoPP::StringSource ss(plainMessage, true, new CryptoPP::StreamTransformationFilter(e, new CryptoPP::StringSink(encryptedMessage)));
					
					std::cout << "encryptedMessage.length(): " << encryptedMessage.length() << std::endl;
					
					ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_message), ENET_PACKET_FLAG_RELIABLE);
					Packet::chat_message *p = new (pkt->data) Packet::chat_message();
					
					memcpy(p->message.data(), encryptedMessage.c_str(), encryptedMessage.length() + 1);
					//memcpy(p->AESiv.data(), iv, sizeof(iv) + 1);
					
					enet_peer_send(peer, Channel::control, pkt);
					
				} else {
					break;
				}
									
				break;
			}
			case PacketType::chat_message: {
				Packet::chat_message *p = (Packet::chat_message *)pkt->data;
				
				Terminal* tm_game_chat = (Terminal*)GameState::gui.GetControlById("game_terminal"); //move this?
				if(p->message_type == 0) {
					tm_game_chat->WriteLog( std::string(p->from_user_name.data()) + ": " + std::string(p->message.data()) );
				} else if(p->message_type == 1) {
					tm_game_chat->WriteLog( "^y" + std::string(p->from_user_name.data()) + "^w: " + std::string(p->message.data()) );
				}
											
				break;
			}
		}
	}
}
