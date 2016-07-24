#include "Network.hpp"
#include <enet/enet.h>
#include <iostream>
#include <cstring>


using std::cout;
using std::endl;
namespace Network {
	#include "server/network.hpp"
	
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
		
		client = enet_host_create (NULL, 1, 2, 0, 0);
		if (client == NULL)
		{
			cout << "An error occurred while trying to create an ENet client host." << endl;
			return false;
		}
		
		ENetPeer *peer;
		ENetEvent event;
		/* Initiate the connection, allocating the two channels 0 and 1. */
		peer = enet_host_connect(client, &address, 2, 0);
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
		} else {
			enet_peer_reset (peer);
			cout << "Connection to " << ip << ":" << port << " failed." << endl;
			return false;
		}
		return true;
	}
	
	
	
	void send_message(std::string message) {
		ENetPacket* packet = enet_packet_create((message).c_str(), message.size(), 
			ENET_PACKET_FLAG_RELIABLE); 
		enet_host_broadcast(client, Channel::msg, packet);
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
			case ENET_EVENT_TYPE_CONNECT:
				cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << endl;
				event.peer->data = (void*)"Client information";
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				cout << "A packet of length " << event.packet->dataLength << " containing " << 
					event.packet->data << " was received from " << event.peer->data << 
					" on channel " << event.channelID << endl;
				parse_packet(event.peer, event.packet);
				enet_packet_destroy(event.packet);

				break;

			case ENET_EVENT_TYPE_DISCONNECT:
				cout << event.peer->data << " disconnected " << endl;
				event.peer->data = NULL;
				break;
			default:
				cout << "event: " << event.type << endl;
			}
		}
	}
	
	void cleanup() {
		enet_host_destroy(client);
	}
	
	void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
		cout << "rcv packet: " << pkt->data << endl;
	}
	
}






