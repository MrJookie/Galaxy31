#ifndef SERVER_HPP
#define SERVER_HPP

#include <enet/enet.h>

void server_start(short port);
void server_wait_for_packet();


#endif
