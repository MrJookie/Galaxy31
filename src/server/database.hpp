#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP
#include <string>
#include <vector>
#include <iostream>
#include <mysql++.h>
#include <thread>

using namespace std;

extern mysqlpp::Connection con;

int createAccount(std::string email, std::string userName, std::string password, std::string ipAddr); //returns user_id ( > 0 ) = ok, -1 = email/username exists, -2 = unspecified error
int loginAccount(std::string email, std::string password, std::string ipAddress, int challenge); //returns user_id ( > 0 ) = ok, 0 = login ok but account is banned (accounts.active = 0), -1 = wrong email/password
mysqlpp::Row getExistingUser(unsigned int account_id); //returns all user info in mysqlpp::Row

std::vector<mysqlpp::Row> getAllAccountsVec();
void mysql_connect(const char *db, const char *server=0, const char *user=0, const char *password=0, unsigned int port=3306);

#endif
