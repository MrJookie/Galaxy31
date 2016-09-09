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

//crypto
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <cryptopp/files.h>

using std::cout;
using std::endl;
using std::thread;
const int timeout = 1;
std::mutex term;
int nthread = 0;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);

struct Player {
	unsigned int user_id;
	std::string user_name;
};

CryptoPP::RSA::PublicKey _publicKey;
CryptoPP::RSA::PrivateKey _privateKey;
std::string _publicKeyStr;
std::string _privateKeyStr;

// local data (statics)
static ENetHost* host;
static std::map<ENetPeer*, Player*> players;

void server_wait_for_packet() {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */

    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << endl;
            handle_new_client(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            //packets.emplace(event.packet, event.peer);
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
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

void generate_keypair() {
	CryptoPP::AutoSeededRandomPool rng;

	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, KEY_SIZE);

	CryptoPP::RSA::PublicKey publicKey(params);
	CryptoPP::RSA::PrivateKey privateKey(params);
	_publicKey = publicKey;
	_privateKey = privateKey;

	_publicKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(_publicKeyStr)).Ref());
	_privateKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(_privateKeyStr)).Ref());
                    
    //std::cout << "publickey: " << _publicKeyStr.length() << " ---- " << _publicKeyStr << std::endl;
    //std::cout << "privatekey: " << _privateKeyStr.length() << " ---- " << _privateKeyStr << std::endl;
}

void server_start(ushort port) {
	//generate_keypair();
	
    ENetAddress address;
    ENetHost * server;
    address.host = ENET_HOST_ANY;
    address.port = port;
    server = enet_host_create(&address,32,Channel::num_channels,0,0);
	host = server;
    if(server == NULL) {
        fprintf(stderr,
                "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Chat server started and listening on port " << port << std::endl;
    
}

void handle_new_client(ENetPeer* peer) {
	srand (time(NULL));
	
	Packet::new_client cl;
	cl.new_id = 0;
	cl.challenge = 0;
	if(_publicKeyStr.length() < sizeof(cl.public_key)) {
		memcpy(cl.public_key.data(), _publicKeyStr.c_str(), _publicKeyStr.length() + 1);
	}
	ENetPacket* pkt = enet_packet_create( &cl, sizeof(cl), ENET_PACKET_FLAG_RELIABLE );
	enet_peer_send(peer, Channel::control, pkt);
	
	Player* player = new Player;
	player->user_id = 0;
	players[peer] = player;
}

void remove_client(ENetPeer* peer) {
	cout << "removing client user_id: " << players[peer]->user_id << endl;
	delete players[peer];
	players.erase( peer );
}

void SendChatMessage(ENetPeer* peer, std::string to_user_name, std::string message, int message_type) {
	ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_message), ENET_PACKET_FLAG_RELIABLE);
	Packet::chat_message *p = new (pkt->data) Packet::chat_message();
	
	std::string from_user_name = players[peer]->user_name;
	
	if(from_user_name.length() < 11 && message.length() < 101) {
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

		enet_host_flush(host);
	}
	
	//enet_packet_destroy(pkt);
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
			
			std::cout << "user_id: " << packet->user_id << " hash: " << packet->hash.data() << " user_name: " << packet->user_name.data() << std::endl;
			
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
		std::cout << "An error occurred while initializing ENet." << std::endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	server_start(54301);
	
	while(1) {
		server_wait_for_packet();
	}
	
	return 0;
}
