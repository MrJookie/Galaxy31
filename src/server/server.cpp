#include <iostream>

#include <queue>
#include <utility>
#include <glm/glm.hpp>

// threading
#include <mutex>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>
#include <map>

#include "server.hpp"
#include "network.hpp"
#include "../Object.hpp"
#include "database.hpp"
#include "RelocatedWork.hpp"

//crypto
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <cryptopp/files.h>

using std::cout;
using std::endl;
using std::thread;
const int timeout = 1;
std::mutex term;
int nthread = 0;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);
static void send_states();

static std::queue<std::pair<ENetPacket*, ENetPeer*>> packets;

struct Player {
	//uint32_t id;
	unsigned int user_id;
	int challenge;
	std::vector<Object> obj;
};

CryptoPP::RSA::PublicKey _publicKey;
CryptoPP::RSA::PrivateKey _privateKey;
std::string _publicKeyStr;
std::string _privateKeyStr;

// local data (statics)
static ENetHost* host;
static uint32_t last_id = 0;
static std::map<ENetPeer*, Player*> players;

std::mutex host_mutex;

void server_wait_for_packet() {
    ENetEvent event;
    /* Wait up to 1000 milliseconds for an event. */
    std::unique_lock<std::mutex> l(host_mutex);
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            cout << "A new client connected from " << event.peer->address.host << ":" << event.peer->address.port << endl;
            handle_new_client(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            packets.emplace(event.packet, event.peer);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
			remove_client(event.peer);
            enet_peer_reset(event.peer);
            break;
        default:
			cout << "event: " << event.type << endl;
        }
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
}

RelocatedWork w;

std::chrono::high_resolution_clock::time_point last_status_update = std::chrono::high_resolution_clock::now();
void server_work() {
	{
		std::unique_lock<std::mutex> l(term);
		cout << "started thread " << (nthread++) << endl;
	}
	while(1) {
		if(w.HasResult()) {
			w.Continue();
		}
		
		std::pair<ENetPacket*, ENetPeer*> packet(0,0);
		if(packets.size() == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		} else {
			std::unique_lock<std::mutex> l(host_mutex);
			if(!packets.empty()) {
				packet = packets.front();
				packets.pop();				
			} else {
				std::this_thread::sleep_for(std::chrono::milliseconds(5));
				continue;
			}
		}
		
		if(packet.second && (packet.second->state & ENET_PEER_STATE_CONNECTED > 0)) {
		//if(packet.first) {
			parse_packet(packet.second, packet.first);
			enet_packet_destroy(packet.first);
		}
		
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		if(now - last_status_update > std::chrono::milliseconds(50)) {
			std::unique_lock<std::mutex> l(host_mutex);
			last_status_update = now;
			send_states();
			// cout << "sending states\n";
		}
	}
}

std::chrono::high_resolution_clock::time_point last_time_mysql_pinged = std::chrono::high_resolution_clock::now();
void mysql_thread(const char *mdb, const char *mserver, const char *muser, const char *mpassword, ushort mport) {
	mysqlpp::Connection con = mysql_connect(mdb, mserver, muser, mpassword, mport);
	
	while(1) {
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		if(now - last_time_mysql_pinged > std::chrono::milliseconds(1000)) {
			if(!con.ping()) {
				cout << "(MySQL has gone away?): reconnecting to mysql" << std::endl;
			}
			
			last_time_mysql_pinged = now;
		}
      
		w.Work(con);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void generate_keypair() {
	CryptoPP::AutoSeededRandomPool rng;

	CryptoPP::InvertibleRSAFunction params;
	params.GenerateRandomWithKeySize(rng, KEY_SIZE);

	CryptoPP::RSA::PublicKey publicKey(params);
	CryptoPP::RSA::PrivateKey privateKey(params);
	_publicKey = publicKey;
	_privateKey = privateKey;

	_publicKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(_publicKeyStr)).Ref());
	_privateKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(_privateKeyStr)).Ref());
                    
    //std::cout << "publickey: " << _publicKeyStr.length() << " ---- " << _publicKeyStr << std::endl;
    //std::cout << "privatekey: " << _privateKeyStr.length() << " ---- " << _privateKeyStr << std::endl;
}

void server_start(ushort port, const char *mdb, const char *mserver, const char *muser, const char *mpassword, ushort mport) {
	generate_keypair();
	
    ENetAddress address;
    ENetHost * server;
    address.host = ENET_HOST_ANY;
    address.port = port;
    server = enet_host_create(&address,32,Channel::num_channels,0,0);
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
    // delete[] t;
    
    std::thread mysql_thrd(mysql_thread, mdb, mserver, muser, mpassword, mport);
	mysql_thrd.detach();
}

void handle_new_client(ENetPeer* peer) {
	srand (time(NULL));
	int challenge = rand() % 9999999 + 1000000;
	
	last_id++;
	Packet::new_client cl;
	cl.new_id = last_id;
	cl.challenge = challenge;
	if(_publicKeyStr.length() < sizeof(cl.public_key)) {
		memcpy(cl.public_key.data(), _publicKeyStr.c_str(), _publicKeyStr.length() + 1);
	}
	ENetPacket* pkt = enet_packet_create( &cl, sizeof(cl), ENET_PACKET_FLAG_RELIABLE );
	enet_peer_send(peer, Channel::control, pkt);
	
	Player* player = new Player;
	//player->id = last_id;
	player->user_id = 0;
	player->challenge = challenge;
	players[peer] = player;
	
	//cout << "challenge: " << challenge << endl;
}

void remove_client(ENetPeer* peer) {
	cout << "removing client user_id: " << players[peer]->user_id << endl;
	delete players[peer];
	players.erase( peer );
}

void send_states() {
	int len = sizeof(Packet::update_objects);
	
	int num_objects = 0;
	for(auto& p : players) {
		num_objects += p.second->obj.size();
	}
	len += sizeof(Object) * num_objects;
	
	ENetPacket* pkt = enet_packet_create( nullptr, len, 0);
	int i=0;
	Packet::update_objects *upd = new (pkt->data) Packet::update_objects;
	upd->num_objects = num_objects;
	Object* obj = (Object*)(pkt->data + sizeof(Packet::update_objects));
	for(auto& p : players) {
		for(auto& o : p.second->obj) {
			obj[i] = o;
			obj[i].SetId(p.second->user_id);
			i++;
		}
		p.second->obj.clear();
	}
	
	enet_host_broadcast(host, Channel::data, pkt);
	enet_host_flush(host);
}

void send_authorize(ENetPeer* peer, int status_code = -1, unsigned int id = 0, std::string user_name = "") {
	int len = sizeof(Packet::authorize);
		
	ENetPacket* pkt = enet_packet_create(nullptr, len, ENET_PACKET_FLAG_RELIABLE);
	Packet::authorize *p = new (pkt->data) Packet::authorize();
	p->user_id = id;
	p->status_code = status_code;
	strcpy(p->user_name.data(), user_name.c_str());
	
	enet_peer_send(peer, Channel::control, pkt);
	enet_host_flush(host);
}

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	if(pkt == nullptr) { cout << "null pkt!!" << endl; return; }
	if(players.find(peer) == players.end()) return;
	
	char ipAddr[256];
	enet_address_get_host_ip(&peer->address, ipAddr, sizeof(ipAddr));
	std::string ipAddress(ipAddr);
            
	// cout << "rcv packet: " << pkt->data << endl;
	Packet::Packet *ppkt = (Packet::Packet*)pkt->data;
	switch(ppkt->type) {
		case PacketType::update_objects: {
			Player& p = *players[peer];
			Packet::update_objects* packet = (Packet::update_objects*)pkt->data;
			if(packet->num_objects != 1) return;
			
			std::unique_lock<std::mutex> l(host_mutex);
			p.obj.push_back( *(Object*)(pkt->data + sizeof(Packet::update_objects)) );
			// cout << "receiving states from " << p.id << "\n";
			break;
		}
		case PacketType::authenticate: {
			Packet::authenticate* packet = (Packet::authenticate*)pkt->data;
			
			w.MakeWork(
				loginAccount,
				packet->user_email.data(),
				packet->user_password.data(),
				ipAddress,
				players[peer]->challenge
			)
			.then(
				[=](int login_account_id) {
					if(login_account_id > 0) {
						w.MakeWork(getExistingUser, login_account_id)
						.then(
							[=](mysqlpp::Row loggedUser) {
								unsigned int user_id = loggedUser["id"];
								std::string user_name(loggedUser["username"]);
								
								players[peer]->user_id = user_id;

								send_authorize(peer, 0, user_id, user_name);
							}
						);
					//send account banned status if login_account_id = 0?
					} else {
						send_authorize(peer, 4);
					}
				}
			);

			break;
		}
		case PacketType::signup: {
			Packet::signup* packet = (Packet::signup*)pkt->data;
			
			//////////////////////////////////////////////// 
			// Decryption
			CryptoPP::AutoSeededRandomPool rng;
				
			std::string encryptedPass(packet->user_password.data(), MAX_ENCRYPTED_LEN);
			std::string decryptedPass;
			
			if(encryptedPass.length() == MAX_ENCRYPTED_LEN) {
				CryptoPP::RSAES_OAEP_SHA_Decryptor d(_privateKey);
				CryptoPP::StringSource ss2(encryptedPass, true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::StringSink(decryptedPass)));
			} else {
				break;
			}

			w.MakeWork(
				createAccount,
				packet->user_email.data(),
				packet->user_name.data(),
				decryptedPass,
				ipAddress
			)
			.then(
				[=](int login_account_id) {
					if(login_account_id > 0) {
						w.MakeWork(getExistingUser, login_account_id)
						.then(
							[=](mysqlpp::Row loggedUser) {
								unsigned int user_id = loggedUser["id"];
								std::string user_name(loggedUser["username"]);

								send_authorize(peer, 0, user_id, user_name);
							}
						);
					} else {
						send_authorize(peer, 1);
					}
				}
			);

			break;
		}
		case PacketType::test_packet: {
			Packet::test_packet* packet = (Packet::test_packet*)pkt->data;
				
			break;
		}
		default:
			cout << "received unknown packet! " << (int)ppkt->type << endl;
			break;
	}
}
