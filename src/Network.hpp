#ifndef NETWORK_HPP
#define NETWORK_HPP
#include <string>

class Object;
namespace Network {
	bool connect(const char* ip, int port);
	void initialize();
	void cleanup();
	void handle_events(int n);
	void flush();
	
	
	void QueueObject(Object* o);
	void Process();
	void SendOurState();
	void SendAuthentication(std::string user_email, std::string user_password);
	void SendRegistration(std::string user_email, std::string user_name, std::string user_password);
};

namespace NetworkChat {
	bool connect(const char* ip, int port);
	void initialize();
	void cleanup();
	void handle_events(int n);
	void flush();
	
	
	void SendChatLogin(unsigned int user_id, std::string user_name);
	void SendChatMessage(std::string to_user_name, std::string message);
};

#endif
