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
	authorize,
	signup,
	chat_message
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
		unsigned char public_key[310];
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
		int status_code; //0 = signup ok, 1 = signup email exists, 2 = signup user exists, 3 = signin ok, 4 = signin error, 5 = signin banned
		char user_name[11];
	};
	
	struct signup : public Packet {
		signup() : Packet(PacketType::signup) {}
		char user_email[41];
		char user_name[11];
		char user_password[41];
	};
	
	/*
	 * //always fill from_user_id
	 * //if /w "Nick" message is set, then send message to Nick user (lookup by user_id on server), response to user, if user doesnt exist
	 * //else only message is filled, so user is empty, broadcast it to all peers, except this peer (author), overlapping?
	struct chat_message : public Packet {
		chat_message() : Packet(PacketType::chat_message) {}
		unsigned int from_user_id;
		char to_user_name[11];
		char message[256];
	};
	*/
}


#endif
