#include "server.hpp"
#include <iostream>
#include <thread>
#include <mutex>
#include <stdio.h>
#include "network.hpp"
ENetHost* host;
mysqlpp::Connection con;
using std::cout;
using std::endl;
using std::thread;
const int timeout = 5000;
std::mutex term;
int nthread = 0;

void parse_packet(ENetPeer* peer, ENetPacket* pkt);

void server_wait_for_packet() {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << endl;
            /* Store any relevant client information here. */
            event.peer->data = (void*)"Client information";
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            // cout << "A packet of length " << event.packet->dataLength << " containing " << 
				// event.packet->data << " was received from " << event.peer->data << 
				// " on channel " << (int)event.channelID << endl;
			
            /* Clean up the packet now that we're done using it. */
            parse_packet(event.peer, event.packet);
            enet_packet_destroy(event.packet);

            break;

        case ENET_EVENT_TYPE_DISCONNECT:
            cout << event.peer->data << " disconnected " << endl;
            /* Reset the peer's client information. */
            event.peer->data = NULL;
            break;
        default:
			cout << "event: " << event.type << endl;
        }
    }
}

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	cout << "rcv packet: " << pkt->data << endl;
}

void server_work() {
	std::unique_lock<std::mutex> l(term);
	cout << "started thread " << (nthread++) << endl;
}


void server_start(short port) {
    ENetAddress address;
    ENetHost * server;
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (&address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = port;
    server = enet_host_create(&address /* the address to bind the server host to */,
                              32      /* allow up to 32 clients and/or outgoing connections */,
                              2      /* allow up to 2 channels to be used, 0 and 1 */,
                              0      /* assume any amount of incoming bandwidth */,
                              0      /* assume any amount of outgoing bandwidth */);
	host = server;
    if(server == NULL) {
        fprintf(stderr,
                "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }
    unsigned int num_cores = std::thread::hardware_concurrency();
    if(num_cores == 0) num_cores = 1;
    unsigned int num_threads = num_cores ;
    thread *t = new thread[num_threads];
    for(int i = 0; i < num_threads; i++) {
		t[i] = thread(server_work);
	}
	for(int i=0; i < num_threads; i++) {
		t[i].detach();
	}
    delete[] t;
}

void mysql_connect(const char *db, const char *server, const char *user, const char *password, unsigned int port) {
	if(con.connect(db, server, user, password, port)) {
        cout << "Connected to MySQL server: " << server << ":" << port << endl;
    } else {
		cout << con.error() << std::endl;
		exit(EXIT_FAILURE);
	}
}

int createAccount(std::string email, std::string userName, std::string password) {
	mysqlpp::Query query = con.query();
    query << "SELECT id FROM accounts WHERE email = '" << mysqlpp::escape << email << "' AND username = '" << mysqlpp::escape << userName << "' LIMIT 1";
    
    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		cout << "Could not create new account! Email or username already exists!" << endl;
		return -1;
	} else {
		query << "INSERT INTO accounts (id, email, username, password, active, datetime_registered) VALUES ("
			  << "'', "
			  << "'" << mysqlpp::escape << email << "', "
			  << "'" << mysqlpp::escape << userName << "', "
			  << "SHA1('" << mysqlpp::escape << password << "'), "
			  << "1, "
			  << "NOW() "
			  << ")";
		if(query.execute()) {
			return query.insert_id();
		}
	}
	
	return -2;
}

mysqlpp::Row getExistingUser(unsigned int id) {
	mysqlpp::Query query = con.query();
    query << "SELECT * FROM accounts WHERE id = " << mysqlpp::escape << id << " LIMIT 1";
    
    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		return res[0];
	}
	
	return mysqlpp::Row();
}

int loginAccount(std::string email, std::string password) {
	mysqlpp::Query query = con.query();
    query << "SELECT id FROM accounts WHERE email = '" << mysqlpp::escape << email << "' AND password = SHA1('" << mysqlpp::escape << password << "') LIMIT 1";
    
    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		return res[0]["id"];
	}
	
	return 0;
}

std::vector<mysqlpp::Row> getAllAccountsVec() {
	std::vector<mysqlpp::Row> result;
	
	mysqlpp::Query query = con.query("SELECT * FROM accounts");
	query.storein(result);
	
	return result;
}
