#ifndef SERVER_HPP
#define SERVER_HPP

#include <enet/enet.h>
#include <mysql++.h>
void server_start(short port);
void server_wait_for_packet();
void mysql_connect(const char *db, const char *server=0, const char *user=0, const char *password=0, unsigned int port=3306);
extern ENetHost* host;
extern mysqlpp::Connection con;

extern int createAccount(std::string email, std::string userName, std::string password);
extern int loginAccount(std::string email, std::string password);
extern mysqlpp::Row getExistingUser(unsigned int id);
extern std::vector<mysqlpp::Row> getAllAccountsVec();

#endif
