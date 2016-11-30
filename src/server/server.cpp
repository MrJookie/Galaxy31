#include <iostream>

#include <enet/enet.h>
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

#include "network.hpp"
#include "../Object.hpp"
#include "database.hpp"
#include "RelocatedWork.hpp"
#include "../Packet.hpp"

#include <tclap/CmdLine.h>
#include "commands/commands.hpp"
#include "commands/tclap_loader.hpp"

#include <sstream>

#include <cassert>

#include "CursesConsole.hpp"


using Commands::Arg;

using std::cout;
using std::endl;

std::mutex term;
std::mutex host_mutex;
std::mutex queue_mutex;
std::string serverPublicKeyStr;
std::string serverPrivateKeyStr;
static const int c_timeout = 1;
int nthread = 0;
bool running = true;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);
static void send_states();
static void server_wait_for_packet();
static void server_start();
static void display_packets();

static std::queue<std::pair<ENetPacket*, ENetPeer*>> packets;

static std::vector<Object> static_objects;

static unsigned int unique_id = 0;

struct Player {
	uint32_t id;
	unsigned int user_id;
	int challenge;
	std::string ip_address;
	std::vector<Object> obj;
	//data
	std::string user_name;
	int resource_money;
};

// local data (statics)
static ENetHost* host;
static uint32_t last_id = 0;
static std::map<ENetPeer*, Player*> players;

#define IP_ANY "0.0.0.0"

// --- command line arguments ---
static TCLAP::CmdLine cmd("", ' ', "1.0");
static TCLAP::ValueArg<std::string> server_ip("", "server_ip", "", false, IP_ANY, "server port");
static TCLAP::ValueArg<std::string> server_port("", "server_port", "", false, std::to_string(SERVER_PORT), "server port");

static TCLAP::ValueArg<std::string> chat_ip("", "chat_ip", "", false, IP_ANY, "ip address");
static TCLAP::ValueArg<std::string> chat_port("", "chat_port", "", false, std::to_string(CHAT_SERVER_PORT), "chat port");

static TCLAP::ValueArg<std::string> sql_ip("", "sql_ip", "", false, SERVER_IP, "mysql ip");
static TCLAP::ValueArg<std::string> sql_db("", "sql_db", "", false, MYSQL_DB, "database");
static TCLAP::ValueArg<std::string> sql_user("", "sql_user", "", false, MYSQL_USER, "username");
static TCLAP::ValueArg<std::string> sql_pw("", "sql_pw", "", false, MYSQL_PASSWORD, "password");
static TCLAP::ValueArg<std::string> sql_port("", "sql_port", "", false, std::to_string(MYSQL_PORT), "port");

TCLAP::ValueArg<std::string> config("c", "cfg", "configuration file", false, "server.cfg", "config filename");
//

auto send_states_frequency = std::chrono::milliseconds(5);

CursesConsole console;
int delay_info = 0;
int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		cout << "An error occurred while initializing ENet." << std::endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	console.StartCurses();

	// code completion for console
	console.SetCodeCompleteHandler( [](std::string cmd, int cursor) {

		std::vector<std::string> vec = Command::Search(cmd, cmd.size()-1, 20);
		if(vec.size() > 1) {
			std::string info = "";
			for(auto& s : vec) {
				info += (cmd + s) + "     ";
			}
			console.SetInfoString(info);
			delay_info = 3;
		}
		return Command::Complete(cmd, cursor);
	});
	
	// command line 
	cmd.add( chat_ip );
	cmd.add( chat_port );
	
	cmd.add( server_ip );
	cmd.add( server_port );
	
	cmd.add( sql_ip );
	cmd.add( sql_db );
	cmd.add( sql_user );
	cmd.add( sql_pw );
	cmd.add( sql_port );
	
	cmd.add( config );
	try {
		cmd.parse( argc, argv );
	} catch (TCLAP::ArgException &e) {
		std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
		return -1;
	}
	
	cout << "-----------------" << std::endl;
	/////////////////
	
	Commands::LoadVariables(cmd);
	
	Command::LoadFromFile(config.getValue());
	
	server_start();
	
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	// box(wnd, 0, 0);
	
	Command::AddCommand("set_info_string", [&](std::string s) {
		console.SetInfoString(s);
		delay_info = 5;
	});
	
	// console input
	while(running) {
		std::string cmd;
		cmd = console.Input();
		cout << (std::string)Command::Execute(cmd) << endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	
	console.StopCurses();
	
	
	if(!config.getValue().empty()) {
		Command::SaveVarariablesToFile(config.getValue(), true);
	}
	
	return 0;
}

void server_wait_for_packet() {
    ENetEvent event;
	while(1) {
		// curses::wrefresh(wnd);
		console.Refresh();
		std::unique_lock<std::mutex> l(host_mutex);
		while(enet_host_service(host, &event, c_timeout) > 0) {
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
}

RelocatedWork w;

auto last_status_update = std::chrono::high_resolution_clock::now();
auto last_info_update = std::chrono::high_resolution_clock::now();
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
		
		auto now = std::chrono::high_resolution_clock::now();
		
		if(now - last_status_update > send_states_frequency) {
			std::unique_lock<std::mutex> l(host_mutex);
			last_status_update = now;
			send_states();
		}
		
		if(now - last_info_update > std::chrono::seconds(1)) {
			last_info_update = now;
			display_packets();
		}
		
		std::this_thread::sleep_for(std::chrono::microseconds(100));
	} 
}

std::chrono::high_resolution_clock::time_point last_time_mysql_pinged = std::chrono::high_resolution_clock::now();
void mysql_work() {
	// cout << Command::GetString("sql_db").c_str() << endl;
	// cout << Command::GetString("sql_ip").c_str() << endl;
	mysqlpp::Connection con = mysql_connect(Command::GetString("sql_db").c_str(), Command::GetString("sql_ip").c_str(), 
											Command::GetString("sql_user").c_str(), Command::GetString("sql_pw").c_str(), Command::Get("sql_port"));
	int ping_freq = Command::Get("mysql_ping_frequency");
	auto mysql_ping_freq = std::chrono::milliseconds(ping_freq <= 1 ? 3000 : ping_freq);
	while(1) {
		std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
		if(now - last_time_mysql_pinged > mysql_ping_freq) {
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
	
void server_start() {
	generate_RSA_keypair();
	// cout << Command::GetString("server_ip") << " : " << Command::Get("server_port") << endl;
    ENetAddress address;
    enet_address_set_host (&address, Command::GetString("server_ip").c_str());
    address.port = Command::Get("server_port").to_int();
    host = enet_host_create(&address, 32, Channel::num_channels,0,0);
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
    
    std::thread mysql_thread(mysql_work);
	mysql_thread.detach();
	
	std::thread *wait_packets_thread = new std::thread(server_wait_for_packet);
	wait_packets_thread->detach();
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
	
	char ipAddr[16];
	enet_address_get_host_ip(&peer->address, ipAddr, sizeof(ipAddr));
	std::string ipAddress(ipAddr);
	
	Player* player = new Player;
	player->id = last_id;
	player->user_id = 0;
	player->challenge = challenge;
	player->ip_address = ipAddress;
	player->user_name = "";
	players[peer] = player;
	//cout << "challenge: " << challenge << endl;
}

void remove_client(ENetPeer* peer) {
	cout << "removing client id: " << players[peer]->id << endl;
	
	Packet s;
	s.put("type", PacketType::player_removed);
	s.put("user_id", players[peer]->id);
	s.broadcast(host, ENET_PACKET_FLAG_RELIABLE);
	
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
	
	Object* obj = (Object*)s.allocate("objects", num_objects*sizeof(Object));
	for(auto& p : players) {
		for(auto& o : p.second->obj) {
			obj[i] = o;
			obj[i].SetId(p.second->id);
			std::strcpy(obj[i].name.data(), p.second->user_name.data());
			assert(i < num_objects);
			i++;
		}
		p.second->obj.clear();
	}
	
	// static objects
	int num_static_objects = static_objects.size();
	s.put("num_static_objects", num_static_objects);
	if(num_static_objects > 0) {
		Object* st_obj = (Object*)s.allocate("static_objects", num_static_objects*sizeof(Object));
		memcpy(st_obj, &static_objects[0], num_static_objects*sizeof(Object));
		static_objects.clear();
	}
	
	s.broadcast(host, Channel::data, 0);
}

void send_authorize(ENetPeer* peer, status_code status = status_code::unknown, unsigned int id = 0, std::string user_name = "", int resource_money = 0) {
	Packet s;
	s.put("type", PacketType::authorize);
	s.put("user_id", id);
	s.put("status_code", status);
	s.put("user_name", user_name);
	s.put("resource_money", resource_money);
	if(status == status_code::login_ok) {
		s.put("chat_ip", Command::GetString("chat_ip"));
		s.put("chat_port", (int)Command::Get("chat_port"));
	}
	s.send(peer, Channel::control, ENET_PACKET_FLAG_RELIABLE);
}

int num_packets = 0;
int data_size = 0;
void display_packets() {
	if(delay_info > 0) { delay_info--; return;}
	// GameState::debug_fields["incoming packets/s"] = std::to_string(num_packets);
	// GameState::debug_fields["incoming B/s"] = std::to_string(data_size);
	console.SetInfoString("incoming packets/s: " + std::to_string(num_packets) + "     " +
						  "incoming B/s: " + std::to_string(data_size) );
	num_packets = 0;
	data_size = 0;
}
	
void parse_packet(ENetPeer* peer, ENetPacket* pkt) {
	if(pkt == nullptr) { cout << "null pkt!!" << endl; return; }
	num_packets++;
	
	data_size += pkt->dataLength;
	if(players.find(peer) == players.end()) return;
	
	Player& player = *players[peer];
	// cout << "rcv packet: " << pkt->data << endl;
	
	
	Packet p(pkt);
	switch(p.get_int("type")) {
		case PacketType::update_objects: {
			
			int num_objects = p.get_int("num_objects");
			if(num_objects != 1) return;
			
			int num_static_objects = p.get_int("num_static_objects");
			if(num_static_objects > 0) {
				Object* o = (Object*)p.get_pair("static_objects").first;
				
				for(int i=0; i < num_static_objects; i++) {
					static_objects.push_back(o[i]);
				}
			}
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
				player.ip_address,
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
								int resource_money(loggedUser["resource_money"]);
								
								players[peer]->user_id = user_id;
								players[peer]->user_name = user_name;
								players[peer]->resource_money = resource_money;

								send_authorize(peer, status_code::login_ok, user_id, user_name, resource_money);
							}
						);
					//send account banned status if login_account_id = 0?
					} else {
						send_authorize(peer, status_code::error_logging_in);
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
				player.ip_address
			)
			.then(
				[=](int login_account_id) {
					if(login_account_id > 0) {
						w.MakeWork(getExistingUser, login_account_id)
						.then(
							[=](mysqlpp::Row loggedUser) {
								unsigned int user_id = loggedUser["id"];
								std::string user_name(loggedUser["username"]);
								int resource_money(loggedUser["resource_money"]);
								
								players[peer]->user_id = user_id;
								players[peer]->user_name = user_name;
								players[peer]->resource_money = resource_money;
								
								send_authorize(peer, status_code::login_ok, user_id, user_name, resource_money);
							}
						);
					} else {
						send_authorize(peer, status_code::email_exist);
					}
				}
			);

			break;
		}
		case PacketType::goodbye: {
			std::unique_lock<std::mutex> l(host_mutex);
			remove_client(peer);
			enet_peer_reset(peer);
			
			Packet s;
			s.put("type", PacketType::player_removed);
			s.put("user_id", player.id);
			s.broadcast(host, ENET_PACKET_FLAG_RELIABLE);
			break;
		}
		default:
			cout << "received unknown packet! " << p.get_int("type") << endl;
			break;
	}
}



// commands

COMMAND(void, quit, ()) {
	running = false;
}

COMMAND(void, send_states_frequency, (int ms)) {
	send_states_frequency = std::chrono::milliseconds( 1000 / ms );
}
