#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP
#include <string>
#include <vector>
#include <iostream>
#include <mysql++.h>
#include <thread>

using namespace std;

mysqlpp::Connection mysql_connect(const char *mdb, const char *mserver=0, const char *muser=0, const char *mpassword=0, ushort mport=3306);

int createAccount(mysqlpp::Connection &con, std::string email, std::string userName, std::string password, std::string ipAddr); //returns user_id ( > 0 ) = ok, 0 = email/username exists, -1 = unspecified error
int loginAccount(mysqlpp::Connection &con, std::string email, std::string password, std::string ipAddress, int challenge); //returns user_id ( > 0 ) = ok, 0 = login ok but account is banned (accounts.active = 0), -1 = wrong email/password
mysqlpp::Row getExistingUser(mysqlpp::Connection &con, unsigned int account_id); //returns all user info in mysqlpp::Row

std::vector<mysqlpp::Row> getAllAccountsVec(mysqlpp::Connection &con);

#endif
