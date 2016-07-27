#include "Network.hpp"
#include <enet/enet.h>
#include <iostream>
#include <cstring>
#include "GameState.hpp"
#include <chrono>

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
		
		ENetPeer *peer;
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
		int len = sizeof(Packet::update_objects) + sizeof(Object);
		ENetPacket* pkt = enet_packet_create(nullptr, len, 0);
		Packet::update_objects *p = new (pkt->data) Packet::update_objects;
		p->num_objects = 1;
		memcpy(pkt->data+sizeof(Packet::update_objects), obj, sizeof(Object));
		enet_peer_send(host, Channel::data, pkt);
	}
	
	void cleanup() {
		enet_host_destroy(client);
	}
	
	Ship::Chassis *chassis = nullptr;
	void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
		Packet::Packet *bp = (Packet::Packet *)pkt->data;
		switch(bp->type) {
			case PacketType::new_client: {
				Packet::new_client *nc = (Packet::new_client *)pkt->data;
				cout << "your client id is: " << nc->new_id << endl;
				GameState::player->SetId(nc->new_id);
				break;
			}
			case PacketType::update_objects: {
				Packet::update_objects *nc = (Packet::update_objects *)pkt->data;
				if(nc->num_objects * sizeof(Object) > pkt->dataLength) return;
				Object* objs = (Object*)(pkt->data+sizeof(Packet::update_objects));
				cout << "updating objects " << nc->num_objects << endl;
				for(int i=0; i < nc->num_objects; i++) {
					Object &o = objs[i];
					if(GameState::player->GetId() == o.GetId()) continue;
					if(GameState::ships.find(o.GetId()) == GameState::ships.end()) {
						cout << "added new ship" << endl;
						if(!chassis)
							chassis = new Ship::Chassis("main_ship", "ship_01_skin.png", "ship_01_skin.png");
						Ship *s = new Ship({0,0}, 0, *chassis);
						GameState::ships[o.GetId()] = s;
						((Object*)s)->Process((double)peer->roundTripTime * 0.5 * 0.001);
					}
					GameState::ships[o.GetId()]->CopyObjectState(o);
					const glm::vec2 pos = GameState::ships[o.GetId()]->GetPosition();
					cout << o.GetId() << ": " << pos.x << ", " << pos.y << endl;
				}
				cout << "rtt: " << peer->roundTripTime << endl;
				break;
			}
		}
	}
	
}
