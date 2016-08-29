#include <iostream>
#include "server.hpp"
#include "database.hpp"

#include <mysql++.h>

#include <chrono>
#include <thread>

using namespace std;

RelocatedWork w;

void mysql_thread() {
	mysqlpp::Connection con;
	if(!con.connect("test", "89.177.76.215", "root", "Galaxy31")) {
		cout << con.error() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	while(1) {
		w.Work(con);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		std::cout << "An error occurred while initializing ENet." << std::endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	//mysql_connect("test", "89.177.76.215", "root", "Galaxy31");
	
	/*
	//mysql_connect("test", "89.177.76.215", "root", "Galaxy31");
	
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
		
	server_start(1234, &w);

	std::thread mysql_thrd(mysql_thread);
	mysql_thrd.detach();

	for(int i = 0; i < 10; i++) {
		w.MakeWork(
			[](mysqlpp::Row row) { std::cout << "got: " << row["username"] << std::endl; },
			[](mysqlpp::Connection con) -> mysqlpp::Row {
				mysqlpp::Query query(con.query("SELECT * FROM accounts"));
				mysqlpp::StoreQueryResult res = query.store();
				
				for (size_t j = 0; j < res.num_rows(); ++j) {
					cout << res[j]["username"] << endl;
				}
								
				
				return res[0];
			}
		);
	}
	
	/*
	for(int i = 0; i < 10; i++) {
		w.MakeWork(
			[](mysqlpp::Row row) { std::cout << "got: " << row["username"] << std::endl; },
			[](SimpleConnectionPool* poolptr) -> mysqlpp::Row { 
				mysqlpp::Connection::thread_start();
				
				mysqlpp::ScopedConnection cp(*poolptr, true);
				if (!cp) {
					cerr << "Failed to get a connection from the pool!" << endl;
					throw std::string("Failed to get a connection from the pool!"); //comment out?
				}
				
				mysqlpp::Query query(cp->query("SELECT * FROM accounts"));
				mysqlpp::StoreQueryResult res = query.store();
				
				for (size_t j = 0; j < res.num_rows(); ++j) {
					cout << res[j]["username"] << endl;
				}
	
				std::cout << "Conns in use: " << poolptr->GetConnsInUse() << std::endl;
								
				mysqlpp::Connection::thread_end();
				
				return res[0];
			},
		poolptr);
	}
	*/
	
	while(1) {
		server_wait_for_packet();
		
		while(w.HasWork()) {
			w.Continue();
		}
	}
	
	return 0;
}
