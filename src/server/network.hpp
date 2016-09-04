#ifndef SERVER_NETWORK_HPP
#define SERVER_NETWORK_HPP

#include <glm/glm.hpp>

#define KEY_SIZE 1024
#define PUBLIC_KEY_SIZE 310
#define MAX_ENCRYPTED_LEN 128
#define MAX_PLAIN_LEN MAX_ENCRYPTED_LEN-42

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
	test_packet,
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
		std::array<char, PUBLIC_KEY_SIZE+1> public_key;
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
		std::array<char, 41> user_email;
		std::array<char, 41> user_password;
	};
	
	struct authorize : public Packet {
		authorize() : Packet(PacketType::authorize) {}
		unsigned int user_id;
		int status_code; //0 = signup ok, 1 = signup email exists, 2 = signup user exists, 3 = signin ok, 4 = signin error, 5 = signin banned
		std::array<char, 11> user_name;
	};
	
	struct signup : public Packet {
		signup() : Packet(PacketType::signup) {}
		std::array<char, 41> user_email;
		std::array<char, 11> user_name;
		std::array<char, MAX_ENCRYPTED_LEN+1> user_password;
	};
	
	struct test_packet : public Packet {
		test_packet() : Packet(PacketType::test_packet) {}
		std::array<char, 1000> data;
	};
	
	/*
	 * //always fill from_user_id
	 * //if /w "Nick" message is set, then send message to Nick user (lookup by user_id on server), response to user, if user doesnt exist
	 * //else only message is filled, so user is empty, broadcast it to all peers, except this peer (author), overlapping?
	struct chat_message : public Packet {
		chat_message() : Packet(PacketType::chat_message) {}
		unsigned int from_user_id;
		std::array<char, 11> to_user_name;
		std::array<char, 256> message;
	};
	*/
}


#endif
