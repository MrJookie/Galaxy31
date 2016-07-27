#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <string>

namespace Network {
	bool connect(const char* ip, int port);
	void initialize();
	void cleanup();
	void handle_events(int n);
	void send_message(std::string message);
	void flush();
	
	void SendOurState();
};

#endif
