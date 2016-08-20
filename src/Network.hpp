#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <string>

//gui
#include "controls/TextBox.hpp"

//crypto
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>

using namespace ng;

namespace Network {
	bool connect(const char* ip, int port);
	void initialize();
	void cleanup();
	void handle_events(int n);
	void send_message(std::string message);
	void flush();
	bool IsConnected();
	
	void SendOurState();
	void SendAuthentication(std::string user_email, std::string user_password);
};

#endif
