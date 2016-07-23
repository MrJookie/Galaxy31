#include "server.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <stdio.h>
#include "network.hpp"
ENetHost* host;
using std::cout;
using std::endl;
using std::thread;
const int timeout = 5000;
std::mutex term;
int nthread = 0;

void parse_packet(ENetPeer* peer, ENetPacket* pkt);

void server_wait_for_packet() {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << endl;
            /* Store any relevant client information here. */
            event.peer->data = (void*)"Client information";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            // cout << "A packet of length " << event.packet->dataLength << " containing " << 
				// event.packet->data << " was received from " << event.peer->data << 
				// " on channel " << (int)event.channelID << endl;
			
            /* Clean up the packet now that we're done using it. */
            parse_packet(event.peer, event.packet);
            enet_packet_destroy(event.packet);

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

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	cout << "rcv packet: " << pkt->data << endl;
}

void server_work() {
	std::unique_lock<std::mutex> l(term);
	cout << "started thread " << (nthread++) << endl;
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
    delete[] t;
}
