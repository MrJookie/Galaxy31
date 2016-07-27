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
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);
static void send_states();

static std::queue<std::pair<ENetPacket*, ENetPeer*>> packets;

struct Player {
	uint32_t id;
	Object obj;
};

// local data (statics)
static ENetHost* host;
static uint32_t last_id = 0;
static std::map<ENetPeer*, Player> players;


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
}


std::mutex mtx;
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
		
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		if(now - last_status_update > std::chrono::milliseconds(10)) {
			send_states();
			last_status_update = now;
			// cout << "sending states\n";
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
                              Channel::num_channels      /* allow up to 2 channels to be used, 0 and 1 */,
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

void handle_new_client(ENetPeer* peer) {
	last_id++;
	Packet::new_client cl;
	cl.new_id = last_id;
	ENetPacket* pkt = enet_packet_create( &cl, sizeof(cl), ENET_PACKET_FLAG_RELIABLE );
	enet_peer_send(peer, Channel::control, pkt);
	
	Player player;
	player.id = last_id;
	players[peer] = player;
}

void remove_client(ENetPeer* peer) {
	cout << "removing client " << players[peer].id << endl;
	players.erase( peer );
}

void send_states() {
	int len = sizeof(Packet::update_objects) + sizeof(Object)*players.size();
	ENetPacket* pkt = enet_packet_create( nullptr, len, 0);
	int i=0;
	Packet::update_objects *upd = new (pkt->data) Packet::update_objects;
	upd->num_objects = players.size();
	Object* obj = (Object*)(pkt->data + sizeof(Packet::update_objects));
	for(auto p : players) {
		obj[i] = p.second.obj;
		obj[i].SetId(p.second.id);
		i++;
	}
	enet_host_broadcast(host, Channel::data, pkt);
}

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	if(pkt == nullptr) { cout << "null pkt!!" << endl; return; }
	// cout << "rcv packet: " << pkt->data << endl;
	Packet::Packet *ppkt = (Packet::Packet*)pkt->data;
	switch(ppkt->type) {
		case PacketType::update_objects: {
			Player& p = players[peer];
			Packet::update_objects* packet = (Packet::update_objects*)pkt->data;
			if(packet->num_objects != 1) return;
			p.obj = *(Object*)(pkt->data + sizeof(Packet::update_objects));
			// cout << "receiving states from " << p.id << "\n";
			break;
		}
		default:
			cout << "received unknown packet! " << (int)ppkt->type << endl;
			break;
	}
	
}
