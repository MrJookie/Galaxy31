#include <iostream>
#include "server.hpp"

#include <mysql++.h>

using namespace std;

int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		std::cout << "An error occurred while initializing ENet." << std::endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	/*
	//////////////////
	std::cout << "-----------------" << std::endl;
	
	
	int new_account_id = createAccount("email@email.com", "username", "plain_password", "127.0.0.1");
	if(new_account_id > 0) {
		std::cout << "Account: New account created!" << std::endl;
	} else if(new_account_id == 0) {
		std::cout << "Account: Could not create new account! Email or username already exists!" << std::endl;
	} else {
		std::cout << "Account: Unspecified error occured!" << std::endl;
	}
	
	int login_account_id = loginAccount("email@email.com", "plain_password", "127.0.0.1");
	if(login_account_id > 0) {
		std::cout << "Account: Login succeed." << std::endl;
		
		mysqlpp::Row loggedUser = getExistingUser(login_account_id);
		std::cout << loggedUser["id"] << " | " << loggedUser["email"] << std::endl;
	} else if(login_account_id == 0) {
		std::cout << "Account: Login failed! Account is banned." << std::endl;
	} else {
		std::cout << "Account: Login failed! Wrong email or password." << std::endl;
	}
	*/
	
	/*
	std::vector<mysqlpp::Row> accounts(getAllAccountsVec());
	for(const auto& account : accounts) {
		std::cout << account["email"] << std::endl;
	}
	*/
	
	std::cout << "-----------------" << std::endl;
	/////////////////
	
	// server_start(1234, "galaxy31", "127.0.0.1", "Galaxy31", "Galaxy31", 3306);
	server_start(1234, "test", "89.177.76.215", "root", "Galaxy31", 3306);
	
	while(1) {
		server_wait_for_packet();
	}
	
	return 0;
}
