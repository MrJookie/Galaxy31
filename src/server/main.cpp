#include <iostream>
#include "server.hpp"
int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		std::cout << "An error occurred while initializing ENet." << std::endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	mysql_connect("test", "89.177.76.215", "root", "Galaxy31");
	
	//////////////////
	createAccount("email@email.com", "username", "plain_password"); //returns user_id ( > 0 ) = ok, -1 = email/username exists, -2 = unspecified error
	int userID = loginAccount("email@email.com", "plain_password"); //returns user_id ( > 0 ) = ok, 0 = wrong email/password
	if(userID > 0) {
		mysqlpp::Row loggedUser = getExistingUser(userID);
		std::cout << loggedUser["id"] << " | " << loggedUser["email"] << std::endl; //returns all user info in mysqlpp::Row
	}
	
	/*
	std::vector<mysqlpp::Row> accounts(getAllAccountsVec());
	for(const auto& account : accounts) {
		std::cout << account["email"] << std::endl;
	}
	*/
	/////////////////
	
	server_start(1234);
	while(1) {
		server_wait_for_packet();
	}
	
	return 0;
}
