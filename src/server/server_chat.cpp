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

using std::cout;
using std::endl;

const int timeout = 1000;
int nthread = 0;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);

struct Player {
	uint32_t id;
	unsigned int user_id;
	std::string user_name;
	std::string public_key;
};

std::array<unsigned char, 17> server_chatAESkey;

// local data (statics)
static ENetHost* host;
static uint32_t last_id = 0;
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

void server_start(ushort port) {
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
	player->id = last_id;
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
				
		ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_login_response), ENET_PACKET_FLAG_RELIABLE);
		Packet::chat_login_response *p = new (pkt->data) Packet::chat_login_response();
		
		memcpy(p->AES_key.data(), encrypted.c_str(), encrypted.length() + 1);
		
		enet_peer_send(peer, Channel::control, pkt);
	} catch(...) {}
}

void SendChatMessage(ENetPeer* peer, std::string to_user_name, std::string message, int message_type) {
	ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_message), ENET_PACKET_FLAG_RELIABLE);
	Packet::chat_message *p = new (pkt->data) Packet::chat_message();
	
	std::string from_user_name = players[peer]->user_name;
	
	if(from_user_name.length() < 11 && message.length() < p->message.size()) {
		strcpy(p->from_user_name.data(), from_user_name.c_str());
		strcpy(p->message.data(), message.c_str());
		p->message_type = message_type;

		for(auto const& player : players) {
			if(message_type == 0) {
				if(player.first != peer) {
					enet_peer_send(player.first, Channel::msg, pkt);
				}
			} else if(message_type == 1) {
				if(player.first != peer && player.second->user_name == to_user_name) {
					enet_peer_send(player.first, Channel::msg, pkt);
					break;
				}
			}
		}
	} else {
		enet_packet_destroy(pkt);
	}
}

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	if(pkt == nullptr) { cout << "null pkt!!" << endl; return; }
	if(players.find(peer) == players.end()) return;
	
	char ipAddr[256];
	enet_address_get_host_ip(&peer->address, ipAddr, sizeof(ipAddr));
	std::string ipAddress(ipAddr);
            
	// cout << "rcv packet: " << pkt->data << endl;
	Packet::Packet *ppkt = (Packet::Packet*)pkt->data;
	switch(ppkt->type) {
		case PacketType::chat_login: {
			Packet::chat_login* packet = (Packet::chat_login*)pkt->data;
			players[peer]->user_id = packet->user_id;
			players[peer]->user_name = packet->user_name.data();
			players[peer]->public_key = packet->public_key.data();
			
			if(players[peer]->user_id > 0 && !players[peer]->user_name.empty() && players[peer]->public_key.length() == RSA_PUBLIC_KEY_SIZE) {
				cout << "id: " << players[peer]->id << " user_id: " << packet->user_id << " user_name: " << packet->user_name.data() << endl;
				
				//send back encrypted aes
				SendEncryptedAESKey(peer);
			} else {
				cout << "User (" << players[peer]->user_id << ") " << packet->user_name.data() << " could not be verified. Kicking!" << endl;
				enet_peer_disconnect(peer, 0);
				//+save log to file
			}
			
			break;
		}
		case PacketType::chat_message: {
			Packet::chat_message* packet = (Packet::chat_message*)pkt->data;
			
			std::string to_user_name(packet->to_user_name.data());
			if(to_user_name.empty()) {
				//broadcast message to everyone except author
				SendChatMessage(peer, to_user_name, packet->message.data(), 0);
			} else {
				//send private message
				SendChatMessage(peer, to_user_name, packet->message.data(), 1);
			}
			
			break;
		}
		default:
			cout << "received unknown packet! " << (int)ppkt->type << endl;
			break;
	}
}

int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		cout << "An error occurred while initializing ENet." << endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	server_start(54301);
	
	while(1) {
		server_wait_for_packet();
	}
	
	return 0;
}
