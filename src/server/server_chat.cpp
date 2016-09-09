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

static std::queue<std::pair<ENetPacket*, ENetPeer*>> packets;

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

std::mutex host_mutex;

void server_wait_for_packet() {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    std::unique_lock<std::mutex> l(host_mutex);
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << endl;
            handle_new_client(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            packets.emplace(event.packet, event.peer);
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

std::chrono::high_resolution_clock::time_point last_status_update = std::chrono::high_resolution_clock::now();
void server_work() {
	{
		std::unique_lock<std::mutex> l(term);
		cout << "started thread " << (nthread++) << endl;
	}
	while(1) {
		std::pair<ENetPacket*, ENetPeer*> packet(0,0);
		if(packets.size() == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		} else {
			std::unique_lock<std::mutex> l(host_mutex);
			if(!packets.empty()) {
				packet = packets.front();
				packets.pop();				
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
		}
		
		if(packet.second && (packet.second->state & ENET_PEER_STATE_CONNECTED > 0)) {
		//if(packet.first) {
			parse_packet(packet.second, packet.first);
			enet_packet_destroy(packet.first);
		}
	}
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
    unsigned int num_cores = std::thread::hardware_concurrency();
    if(num_cores == 0) num_cores = 1;
    unsigned int num_threads = num_cores ;
    thread *t = new thread[num_threads];
    for(int i = 0; i < num_threads; i++) {
		t[i] = thread(server_work);
	}
	for(int i=0; i < num_threads; i++) {
		t[i].detach();
	}
    // delete[] t;
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

void SendChatMessage(std::string from_user_name, ENetPeer* peer, std::string message, int message_type) {
	ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_message), ENET_PACKET_FLAG_RELIABLE);
	Packet::chat_message *p = new (pkt->data) Packet::chat_message();
	
	if(from_user_name.length() < 11 && message.length() < 101) {
		strcpy(p->from_user_name.data(), from_user_name.c_str());
		strcpy(p->message.data(), message.c_str());
		p->message_type = message_type;
		
		if(peer == nullptr) {
			enet_host_broadcast(host, Channel::msg, pkt);
		} else {
			enet_peer_send(peer, Channel::msg, pkt);
		}
		enet_host_flush(host);
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
			
			std::cout << "user_id: " << packet->user_id << " hash: " << packet->hash.data() << " user_name: " << packet->user_name.data() << std::endl;
			
			break;
		}
		case PacketType::chat_message: {
			Packet::chat_message* packet = (Packet::chat_message*)pkt->data;
			//received message from author:
			//players[peer]->user_name ==== author
			//std::cout << players[peer]->user_name << " (" << players[peer]->user_id << ") to: " << packet->to_user_name.data() << " msg: " << packet->message.data() << std::endl;
			
			std::string to_user_name(packet->to_user_name.data());
			
			if(to_user_name.empty()) {
				//broadcast message to everyone except author
				
				for (auto const& player : players) {
					if(player.first != peer) {
						SendChatMessage(players[peer]->user_name, player.first, packet->message.data(), 0);
					}
				}
			} else {
				//send private message
				for (auto const& player : players) {
					if(player.second->user_name == to_user_name && players[peer]->user_name != to_user_name) {
						SendChatMessage(players[peer]->user_name, player.first, packet->message.data(), 1);
						break;
					}
				}
			}
			
			break;
		}
		default:
			//cout << "received unknown packet! " << (int)ppkt->type << endl;
			//add NetworkChat namespace in the Game
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
