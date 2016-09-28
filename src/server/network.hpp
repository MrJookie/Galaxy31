#ifndef SERVER_NETWORK_HPP
#define SERVER_NETWORK_HPP

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

#define SERVER_IP "89.177.76.215"
#define SERVER_PORT 17694

#define CHAT_SERVER_IP "89.177.76.215"
#define CHAT_SERVER_PORT 17792

enum Channel {
	control = 0,
	msg,
	data,
	num_channels
};

enum status_code {
	unknown = -1,
	login_ok = 0,
	email_exist = 1,
	username_taken = 2,
	error_logging_in = 4
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
	chat_message,
	goodbye,
	player_removed
};

#endif
