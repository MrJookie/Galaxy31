#ifndef SERVER_NETWORK_HPP
#define SERVER_NETWORK_HPP

#include <glm/glm.hpp>

//crypto
#include <cryptopp/hex.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

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
	new_client = 0,
	update_objects,
	new_ship,
	authenticate,
	authorize,
	signup,
	chat_login,
	chat_login_response,
	chat_message
};

#endif
