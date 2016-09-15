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
#include "../Packet.hpp"

using std::cout;
using std::endl;

std::mutex term;
std::mutex host_mutex;
std::mutex queue_mutex;
std::string serverPublicKeyStr;
std::string serverPrivateKeyStr;
const int timeout = 1;
int nthread = 0;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);
static void send_states();

static std::queue<std::pair<ENetPacket*, ENetPeer*>> packets;

struct Player {
	uint32_t id;
	unsigned int user_id;
	int challenge;
	std::vector<Object> obj;
};

// local data (statics)
static ENetHost* host;
static uint32_t last_id = 0;
static std::map<ENetPeer*, Player*> players;

void server_wait_for_packet() {
    ENetEvent event;

    std::unique_lock<std::mutex> l(host_mutex);
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
			case ENET_EVENT_TYPE_CONNECT:
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
    
    l.unlock();
    
    //enet_host_flush(host);
    
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

		{
			std::unique_lock<std::mutex> l(queue_mutex);
			if(!packets.empty()) {
				packet = packets.front();
				packets.pop();
			}
		}
		
		if(packet.second && (packet.second->state & ENET_PEER_STATE_CONNECTED > 0)) {
			parse_packet(packet.second, packet.first);
			enet_packet_destroy(packet.first);
		}
		
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		if(now - last_status_update > std::chrono::milliseconds(50)) {
			std::unique_lock<std::mutex> l(host_mutex);
			last_status_update = now;
			
			send_states();
		}
		
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

std::chrono::high_resolution_clock::time_point last_time_mysql_pinged = std::chrono::high_resolution_clock::now();
void mysql_work(const char *mdb, const char *mserver, const char *muser, const char *mpassword, ushort mport) {
	mysqlpp::Connection con = mysql_connect(mdb, mserver, muser, mpassword, mport);
	
	while(1) {
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		if(now - last_time_mysql_pinged > std::chrono::milliseconds(1000)) {
			if(!con.ping()) {
				cout << "(MySQL has gone away): reconnecting" << endl;
			}
			
			last_time_mysql_pinged = now;
		}
      
		w.Work(con);
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

void generate_RSA_keypair() {
	try {
		CryptoPP::AutoSeededRandomPool rng;

		CryptoPP::InvertibleRSAFunction params;
		params.GenerateRandomWithKeySize(rng, RSA_KEY_SIZE);

		CryptoPP::RSA::PublicKey publicKey(params);
		CryptoPP::RSA::PrivateKey privateKey(params);

		publicKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(serverPublicKeyStr)).Ref());
		privateKey.Save(CryptoPP::HexEncoder(new CryptoPP::StringSink(serverPrivateKeyStr)).Ref());
	} catch(...) {}
}

void server_start(ushort port, const char *mdb, const char *mserver, const char *muser, const char *mpassword, ushort mport) {
	generate_RSA_keypair();
	
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    host = enet_host_create(&address,32,Channel::num_channels,0,0);
    if(host == nullptr) {
        cout << "An error occurred while trying to create an ENet server host." << endl;
        exit(EXIT_FAILURE);
    }
    
    unsigned int num_cores = std::thread::hardware_concurrency();
    unsigned int num_threads = (num_cores == 0) ?  1 : num_cores;
    
    std::thread *t = new std::thread[num_threads];
    for(int i = 0; i < num_threads; i++) {
		t[i] = std::thread(server_work);
		
		if(t[i].joinable()) {
			t[i].detach();
		}
	}

    // delete[] t;
    
    std::thread mysql_thread(mysql_work, mdb, mserver, muser, mpassword, mport);
	mysql_thread.detach();
}

void handle_new_client(ENetPeer* peer) {
	cout << "A new client connected from " << peer->address.host << ":" << peer->address.port << endl;
	
	srand (time(NULL));
	int challenge = rand() % 9999999 + 1000000;
	
	last_id++;
	
	Packet s;
	s.put("type", PacketType::new_client);
	s.put("new_id", last_id);
	s.put("challenge", challenge);
	s.put("public_key", serverPublicKeyStr);
	s.send(peer, Channel::control, ENET_PACKET_FLAG_RELIABLE);
	
	Player* player = new Player;
	player->id = last_id;
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
	int num_objects = 0;
	for(auto& p : players) {
		num_objects += p.second->obj.size();
	}
	int i = 0;
	
	Packet s;
	s.put("type", PacketType::update_objects);
	s.put("num_objects", num_objects);

	Object* obj = new (s.allocate("objects", num_objects*sizeof(Object))) Object[num_objects];
	for(auto& p : players) {
		for(auto& o : p.second->obj) {
			obj[i] = o;
			obj[i].SetId(p.second->user_id);
			i++;
		}
		p.second->obj.clear();
	}
	
	s.broadcast(host, Channel::data, 0);
}

void send_authorize(ENetPeer* peer, int status_code = -1, unsigned int id = 0, std::string user_name = "") {
	Packet s;
	s.put("type", PacketType::authorize);
	s.put("user_id", id);
	s.put("status_code", status_code);
	s.put("user_name", user_name);
	s.send(peer, Channel::control, ENET_PACKET_FLAG_RELIABLE);
}

void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	if(pkt == nullptr) { cout << "null pkt!!" << endl; return; }
	if(players.find(peer) == players.end()) return;
	
	char ipAddr[256];
	enet_address_get_host_ip(&peer->address, ipAddr, sizeof(ipAddr));
	std::string ipAddress(ipAddr);
            
	// cout << "rcv packet: " << pkt->data << endl;
	
	Packet p(pkt);
	switch(p.get_int("type")) {
		case PacketType::update_objects: {
			Player& player = *players[peer];
			
			int num_objects = p.get_int("num_objects");
			if(num_objects != 1) return;
			
			std::unique_lock<std::mutex> l(host_mutex);
			player.obj.push_back( *((Object*)p.get_pair("objects").first) );
			// cout << "receiving states from " << player.id << "\n";
				
			break;
		}
		case PacketType::authenticate: {
			w.MakeWork(
				loginAccount,
				p.get_string("user_email"),
				p.get_string("user_password"),
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
			//decryption
			CryptoPP::AutoSeededRandomPool rng;
				
			std::string encrypted(p.get_string("user_password"), RSA_MAX_ENCRYPTED_LEN);
			std::string decrypted;
			
			if(encrypted.length() == RSA_MAX_ENCRYPTED_LEN) {
				try {
				CryptoPP::RSA::PrivateKey privateKey;
				privateKey.Load(CryptoPP::StringSource(serverPrivateKeyStr, true, new CryptoPP::HexDecoder()).Ref());
				
				CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
				CryptoPP::StringSource ss2(encrypted, true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::StringSink(decrypted)));
				} catch(...) {}
			} else {
				break;
			}

			w.MakeWork(
				createAccount,
				p.get_string("user_email"),
				p.get_string("user_name"),
				decrypted,
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
								
								players[peer]->user_id = user_id;

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
		default:
			cout << "received unknown packet! " << p.get_int("type") << endl;
			break;
	}
}
