#include <iostream>

#include <queue>
#include <utility>
#include <glm/glm.hpp>

// threading
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <map>

#include <enet/enet.h>
#include "network.hpp"

#include "../Packet.hpp"

using std::cout;
using std::endl;

const int timeout = 1000;
int nthread = 0;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);

struct Player {
	unsigned int client_id;
	unsigned int user_id;
	std::string user_name;
	std::string public_key;
};

std::array<unsigned char, 17> server_chatAESkey;

// local data (statics)
static ENetHost* host;
static unsigned int last_id = 0;
static std::map<ENetPeer*, Player*> players;

void generate_AES_key() {
	try {
		CryptoPP::AutoSeededRandomPool prng;

		CryptoPP::SecByteBlock key(AES_KEY_SIZE);
		prng.GenerateBlock(key, key.size());
		
		memcpy(server_chatAESkey.data(), key, key.size() + 1);
	} catch(...) {}
}

void server_wait_for_packet() {
    ENetEvent event;
    
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            handle_new_client(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            parse_packet(event.peer, event.packet);
			enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
			remove_client(event.peer);
            enet_peer_reset(event.peer);
            break;
        default:
			cout << "event: " << event.type << endl;
        }
    }
    
    //enet_host_flush(host);
}

void server_start(unsigned short port) {
	generate_AES_key();
	
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    host = enet_host_create(&address,32,Channel::num_channels,0,0);
    if(host == nullptr) {
        cout << "An error occurred while trying to create an ENet server host." << endl;
        exit(EXIT_FAILURE);
    }
    
    cout << "Chat server started and listening on port " << port << endl;
}

void handle_new_client(ENetPeer* peer) {
	cout << "A new client connected from " << peer->address.host << ":" << peer->address.port << endl;
	 
	last_id++;
	
	Player* player = new Player;
	player->client_id = last_id;
	player->user_id = 0;
	players[peer] = player;
}

void remove_client(ENetPeer* peer) {
	cout << "removing client user_id: " << players[peer]->user_id << endl;
	delete players[peer];
	players.erase( peer );
}

void SendEncryptedAESKey(ENetPeer* peer) {
	try {
		CryptoPP::AutoSeededRandomPool rng;
	
		std::string encrypted;
		std::string hexKey;

		//exchange std::string public_key with CryptoPP::RSA::PublicKey, so there is no Load() every connection
		
		CryptoPP::RSA::PublicKey loadPublicKey;
		CryptoPP::ArraySource(server_chatAESkey.data(), server_chatAESkey.size() - 1, true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hexKey)));
		loadPublicKey.Load(CryptoPP::StringSource(players[peer]->public_key, true, new CryptoPP::HexDecoder()).Ref());
		
		CryptoPP::RSAES_OAEP_SHA_Encryptor e(loadPublicKey);
		CryptoPP::StringSource ss1(hexKey, true, new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::StringSink(encrypted)));
		
		Packet s;
		s.put("type", PacketType::chat_login_response);
		s.put("AES_key", encrypted);
		s.send(peer, Channel::control, ENET_PACKET_FLAG_RELIABLE);
	} catch(...) {}
}

void SendChatMessage(ENetPeer* peer, std::string to_user_name, std::string message, int message_type) {
	std::string from_user_name = players[peer]->user_name;
	
	Packet s;
	s.put("type", PacketType::chat_message);
	s.put("from_username", from_user_name);
	s.put("message", message);
	s.put("message_type", message_type);

	for(auto const& player : players) {
		if(message_type == 0) {
			if(player.first != peer) {
				s.send(player.first, Channel::msg, ENET_PACKET_FLAG_RELIABLE);
			}
		} else if(message_type == 1) {
			if(player.first != peer && player.second->user_name == to_user_name) {
				s.send(player.first, Channel::msg, ENET_PACKET_FLAG_RELIABLE);
				break;
			}
		}
	}
}

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	if(pkt == nullptr) { cout << "null pkt!!" << endl; return; }
	if(players.find(peer) == players.end()) return;
	
	char ipAddr[256];
	enet_address_get_host_ip(&peer->address, ipAddr, sizeof(ipAddr));
	std::string ipAddress(ipAddr);
            
	// cout << "rcv packet: " << pkt->data << endl;
	
	Packet p(pkt);
	switch(p.get_int("type")) {
		case PacketType::chat_login: {
			players[peer]->user_id = p.get_int("user_id");
			players[peer]->user_name = p.get_string("user_name");
			players[peer]->public_key = p.get_string("public_key");
			
			if(players[peer]->user_id > 0 && !players[peer]->user_name.empty() && players[peer]->public_key.length() == RSA_PUBLIC_KEY_SIZE) {
				cout << "client_id: " << players[peer]->client_id << " user_id: " << p.get_int("user_id") << " user_name: " << p.get_string("user_name") << endl;
				
				//send back encrypted aes
				SendEncryptedAESKey(peer);
			} else {
				cout << "User (" << players[peer]->user_id << ") " << p.get_string("user_name") << " could not be verified. Kicking!" << endl;
				enet_peer_disconnect(peer, 0);
				//+save log to file
			}
			
			break;
		}
		case PacketType::chat_message: {
			if(p.get_string("to_username").empty()) {
				//broadcast message to everyone except author
				SendChatMessage(peer, p.get_string("to_username"), p.get_string("message"), 0);
			} else {
				//send private message
				SendChatMessage(peer, p.get_string("to_username"), p.get_string("message"), 1);
			}
			
			break;
		}			
		default:
			cout << "received unknown packet! " << p.get_int("type") << endl;
			break;
	}
}

int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		cout << "An error occurred while initializing ENet." << endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	server_start(CHAT_SERVER_PORT);
	
	while(1) {
		server_wait_for_packet();
	}
	
	return 0;
}
