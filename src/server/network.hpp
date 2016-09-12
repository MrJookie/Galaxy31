#ifndef SERVER_NETWORK_HPP
#define SERVER_NETWORK_HPP

#include <glm/glm.hpp>

#define RSA_KEY_SIZE 1024
#define RSA_PUBLIC_KEY_SIZE 320
#define RSA_MAX_ENCRYPTED_LEN 128
#define RSA_MAX_PLAIN_LEN RSA_MAX_ENCRYPTED_LEN-42
#define AES_KEY_SIZE 16 //bytes (CryptoPP::AES::DEFAULT_KEYLENGTH)
#define AES_MAX_MESSAGE_LEN 128

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
	chat_login,
	chat_login_response,
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
		std::array<char, RSA_PUBLIC_KEY_SIZE+1> public_key;
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
		std::array<char, RSA_MAX_ENCRYPTED_LEN+1> user_password;
	};

	struct chat_login : public Packet {
		chat_login() : Packet(PacketType::chat_login) {}
		unsigned int user_id;
		std::array<char, 11> user_name;
		std::array<char, RSA_PUBLIC_KEY_SIZE+1> public_key;
	};
	
	struct chat_login_response : public Packet {
		chat_login_response() : Packet(PacketType::chat_login_response) {}
		std::array<char, RSA_MAX_ENCRYPTED_LEN+1> AES_key;
	};
	
	struct chat_message : public Packet {
		chat_message() : Packet(PacketType::chat_message) {}
		int message_type;
		std::array<char, 11> from_user_name;
		std::array<char, 11> to_user_name;
		std::array<char, AES_MAX_MESSAGE_LEN+1> message;
		//std::array<unsigned char, 129> AESiv;
	};
	
}


#endif
