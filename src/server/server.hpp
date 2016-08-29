#ifndef SERVER_HPP
#define SERVER_HPP

#include <enet/enet.h>

#include "database.hpp"
#include "RelocatedWork.hpp"

void server_start(short port, RelocatedWork* w, SimpleConnectionPool* poolptr);
void server_wait_for_packet();


#endif
