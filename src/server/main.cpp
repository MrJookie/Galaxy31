#include <iostream>
#include "server.hpp"
int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		std::cout << "An error occurred while initializing ENet." << std::endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	mysql_connect("test", "89.177.76.215", "root", "Galaxy31");
	
	std::vector<mysqlpp::Row> accounts(getAllAccountsVec());
	for(const auto& account : accounts) {
		std::cout << account["email"] << std::endl;
	}
	
	server_start(1234);
	while(1) {
		server_wait_for_packet();
	}
	
	return 0;
}
