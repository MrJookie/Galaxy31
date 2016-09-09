#ifndef SERVER_HPP
#define SERVER_HPP

#include <enet/enet.h>

void server_start(ushort port, const char *mdb, const char *mserver, const char *muser, const char *mpassword, ushort mport);
void server_wait_for_packet();
void mysql_thread(const char *mdb, const char *mserver, const char *muser, const char *mpassword, ushort mport);

#endif
