#include <iostream>

#include <queue>
#include <utility>
#include <sstream>
#include <glm/glm.hpp>

// threading
#include <mutex>
#include <thread>
#include <chrono>

#include "server.hpp"
#include "network.hpp"
#include "../Object.hpp"



using std::cout;
using std::endl;
using std::thread;
const int timeout = 5000;
std::mutex term;
int nthread = 0;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void new_client(ENetPeer* peer);
static std::queue<std::pair<ENetPacket*, ENetPeer*>> packets;

struct Player {
	uint32_t id;
	Object obj;
	ENetPeer* peer;
};

// local data (statics)
static ENetHost* host;
static uint32_t last_id = 0;
static std::vector<Player> players;


void server_wait_for_packet() {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << endl;
            new_client(event.peer);
            
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            // cout << "A packet of length " << event.packet->dataLength << " containing " << 
				// event.packet->data << " was received from " << event.peer->data << 
				// " on channel " << (int)event.channelID << endl;
			
            /* Clean up the packet now that we're done using it. */
            // parse_packet(event.peer, event.packet);
            packets.emplace(event.packet, event.peer);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            cout << event.peer->data << " disconnected " << endl;
            /* Reset the peer's client information. */
            event.peer->data = NULL;
            break;
        default:
			cout << "event: " << event.type << endl;
        }
    }
}


std::mutex mtx;
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
			std::unique_lock<std::mutex> l(mtx);
			if(!packets.empty()) {
				packet = packets.front();
				packets.pop();				
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
		}
		
		if(packet.first) {
			parse_packet(packet.second, packet.first);
			enet_packet_destroy(packet.first);			
		}
		
	}
}


void server_start(short port) {
    ENetAddress address;
    ENetHost * server;
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (&address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = port;
    server = enet_host_create(&address /* the address to bind the server host to */,
                              32      /* allow up to 32 clients and/or outgoing connections */,
                              2      /* allow up to 2 channels to be used, 0 and 1 */,
                              0      /* assume any amount of incoming bandwidth */,
                              0      /* assume any amount of outgoing bandwidth */);
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

void new_client(ENetPeer* peer) {
	last_id++;
	std::stringstream bytes;
	bytes << (uint32_t)PacketType::new_id
		  << last_id;
	std::string bs = bytes.str();
	int len = bs.size();
	ENetPacket* pkt = enet_packet_create( bs.c_str(), bs.size()+1, ENET_PACKET_FLAG_RELIABLE ); 
	cout << "len: " << len << endl;
	// bytes.get((char*)pkt->data, len);
	
	pkt->data[len] = 0;
	cout << pkt->data << endl;
	enet_peer_send(peer, Channel::control, pkt);
	
	Player player;
	player.id = last_id;
	player.peer = peer;
	players.push_back(player);
}

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	if(pkt == nullptr) return;
	// cout << "rcv packet: " << pkt->data << endl;
	PacketType type;
	switch(type) {
		case PacketType::update:
			
			break;
		default:
			break;
	}
	
}
