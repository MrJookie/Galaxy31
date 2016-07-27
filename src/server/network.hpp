#ifndef SERVER_NETWORK_HPP
#define SERVER_NETWORK_HPP

#include <glm/glm.hpp>

enum Channel {
	control = 0,
	msg,
	data,
	num_channels
};

enum PacketType {
	new_client,
	update_objects,
	new_ship
};

namespace Packet {
	struct Packet {
		Packet(PacketType t) : type(t) {}
		PacketType type;
	};

	struct new_client : public Packet {
		new_client() : Packet(PacketType::new_client) {}
		int new_id;
	};

	struct update_objects : public Packet {
		update_objects() : Packet(PacketType::update_objects) {}
		int num_objects;
	};
	
	struct new_ship : public Packet {
		new_ship() : Packet(PacketType::new_ship) {}
		
	};
}


#endif
