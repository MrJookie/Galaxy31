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
	new_ship,
	authenticate,
	authorize
};

namespace Packet {
	struct Packet {
		Packet(PacketType t) : type(t) {}
		PacketType type;
	};

	struct new_client : public Packet {
		new_client() : Packet(PacketType::new_client) {}
		int new_id;
		int challenge;
	};

	struct update_objects : public Packet {
		update_objects() : Packet(PacketType::update_objects) {}
		int num_objects;
	};
	
	struct new_ship : public Packet {
		new_ship() : Packet(PacketType::new_ship) {}
	};
	
	struct authenticate : public Packet {
		authenticate() : Packet(PacketType::authenticate) {}
		char user_email[41];
		char user_password[41];
	};
	
	struct authorize : public Packet {
		authorize() : Packet(PacketType::authorize) {}
		unsigned int user_id;
		char user_name[11];
	};
}


#endif
