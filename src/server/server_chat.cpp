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

#include <enet/enet.h>
#include "network.hpp"

//crypto
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>
#include <cryptopp/rsa.h>
#include <cryptopp/osrng.h>
#include <cryptopp/integer.h>
#include <cryptopp/secblock.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

using std::cout;
using std::endl;

const int timeout = 1000;
int nthread = 0;

// local forwards
static void parse_packet(ENetPeer* peer, ENetPacket* pkt);
static void handle_new_client(ENetPeer* peer);
static void remove_client(ENetPeer* peer);

struct Player {
	uint32_t id;
	unsigned int user_id;
	std::string user_name;
	std::string public_key;
};

unsigned char serverAESkey[17];
unsigned char serverAESiv[16];

// local data (statics)
static ENetHost* host;
static uint32_t last_id = 0;
static std::map<ENetPeer*, Player*> players;

void generate_AES_key() {
	CryptoPP::AutoSeededRandomPool prng;

	CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
	prng.GenerateBlock( key, key.size() );

	unsigned char iv[ CryptoPP::AES::BLOCKSIZE ];
	prng.GenerateBlock( iv, sizeof(iv) );
	
	memcpy(serverAESkey, key, key.size() + 1);
	memcpy(serverAESiv, iv, sizeof(iv) + 1);

	std::string plain = "CBC Mode Test";
	std::string cipher, encoded, recovered;

	try
	{
		cout << "plain text: " << plain << endl;

		CryptoPP::CBC_Mode< CryptoPP::AES >::Encryption e;
		e.SetKeyWithIV( key, key.size(), iv );

		// The StreamTransformationFilter adds padding
		//  as required. ECB and CBC Mode must be padded
		//  to the block size of the cipher.
		CryptoPP::StringSource ss( plain, true, 
			new CryptoPP::StreamTransformationFilter( e,
				new CryptoPP::StringSink( cipher )
			) // StreamTransformationFilter      
		); // StringSource
	}
	catch( const CryptoPP::Exception& e )
	{
		std::cerr << e.what() << endl;
		exit(1);
	}


	// Pretty print cipher text
	CryptoPP::StringSource ss( cipher, true,
		new CryptoPP::HexEncoder(
			new CryptoPP::StringSink( encoded )
		) // HexEncoder
	); // StringSource
	cout << "cipher text: " << encoded << endl;

	try
	{
		CryptoPP::CBC_Mode< CryptoPP::AES >::Decryption d;
		d.SetKeyWithIV( serverAESkey, sizeof(serverAESkey) - 1, serverAESiv );

		// The StreamTransformationFilter removes
		//  padding as required.
		CryptoPP::StringSource ss( cipher, true, 
			new CryptoPP::StreamTransformationFilter( d,
				new CryptoPP::StringSink( recovered )
			) // StreamTransformationFilter
		); // StringSource

		cout << "recovered text: " << recovered << endl;
	}
	catch( const CryptoPP::Exception& e )
	{
		std::cerr << e.what() << endl;
		exit(1);
	}
}

void server_wait_for_packet() {
    ENetEvent event;
    
    while(enet_host_service(host, &event, timeout) > 0) {
        switch(event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            handle_new_client(event.peer);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            parse_packet(event.peer, event.packet);
			enet_packet_destroy(event.packet);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
			remove_client(event.peer);
            enet_peer_reset(event.peer);
            break;
        default:
			cout << "event: " << event.type << endl;
        }
    }
    
    //enet_host_flush(host);
}

void server_start(ushort port) {
	generate_AES_key();
	
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;
    host = enet_host_create(&address,32,Channel::num_channels,0,0);
    if(host == nullptr) {
        cout << "An error occurred while trying to create an ENet server host." << endl;
        exit(EXIT_FAILURE);
    }
    
    cout << "Chat server started and listening on port " << port << endl;
}

void handle_new_client(ENetPeer* peer) {
	cout << "A new client connected from " << peer->address.host << ":" << peer->address.port << endl;
	 
	last_id++;
	
	Player* player = new Player;
	player->id = last_id;
	player->user_id = 0;
	players[peer] = player;
}

void remove_client(ENetPeer* peer) {
	cout << "removing client user_id: " << players[peer]->user_id << endl;
	delete players[peer];
	players.erase( peer );
}

void SendChatMessage(ENetPeer* peer, std::string to_user_name, std::string message, int message_type) {
	ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_message), ENET_PACKET_FLAG_RELIABLE);
	Packet::chat_message *p = new (pkt->data) Packet::chat_message();
	
	std::string from_user_name = players[peer]->user_name;
	
	if(from_user_name.length() < 11 && message.length() < 101) {
		strcpy(p->from_user_name.data(), from_user_name.c_str());
		strcpy(p->message.data(), message.c_str());
		p->message_type = message_type;

		for(auto const& player : players) {
			if(message_type == 0) {
				if(player.first != peer) {
					enet_peer_send(player.first, Channel::msg, pkt);
				}
			} else if(message_type == 1) {
				if(player.first != peer && player.second->user_name == to_user_name) {
					enet_peer_send(player.first, Channel::msg, pkt);
					break;
				}
			}
		}
	} else {
		enet_packet_destroy(pkt);
	}
}

void SendEncryptedAESKey(ENetPeer* peer) {
	//std::string plain(reinterpret_cast<const char*>(serverAESkey), 16); 
	std::string encrypted;
	
	std::string hexKey;
        
	CryptoPP::ArraySource        (serverAESkey, sizeof(serverAESkey) - 1, true, 
			new CryptoPP::HexEncoder (
					new CryptoPP::StringSink (hexKey)
					)
			);

	CryptoPP::AutoSeededRandomPool rng;
	
	//exchange std::string public_key with CryptoPP::RSA::PublicKey 
	CryptoPP::RSA::PublicKey loadPublicKey;
	loadPublicKey.Load(CryptoPP::StringSource(players[peer]->public_key, true, new CryptoPP::HexDecoder()).Ref());
	
	CryptoPP::RSAES_OAEP_SHA_Encryptor e(loadPublicKey);
	CryptoPP::StringSource ss1(hexKey, true, new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::StringSink(encrypted)));
			
	ENetPacket* pkt = enet_packet_create(nullptr, sizeof(Packet::chat_login_response), ENET_PACKET_FLAG_RELIABLE);
	Packet::chat_login_response *p = new (pkt->data) Packet::chat_login_response();
	
	memcpy(p->AES_key.data(), encrypted.c_str(), encrypted.length() + 1);
	
	std::cout << "hexKey: " << hexKey << std::endl;
	
	enet_peer_send(peer, Channel::control, pkt);
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
		case PacketType::chat_login: {
			Packet::chat_login* packet = (Packet::chat_login*)pkt->data;
			players[peer]->user_id = packet->user_id;
			players[peer]->user_name = packet->user_name.data();
			players[peer]->public_key = packet->public_key.data();
			
			if(players[peer]->user_id > 0 && !players[peer]->user_name.empty() && players[peer]->public_key.length() == PUBLIC_KEY_SIZE) {
				cout << "id: " << players[peer]->id << " user_id: " << packet->user_id << " user_name: " << packet->user_name.data() << endl;
				
				//send back encrypted aes
				SendEncryptedAESKey(peer);
			} else {
				//save log to file
				cout << "User: " << packet->user_name.data() << " could not be verified. Kicking!" << endl;
				enet_peer_disconnect(peer, 0);
			}
			
			break;
		}
		case PacketType::chat_message: {
			Packet::chat_message* packet = (Packet::chat_message*)pkt->data;
			
			std::string cipher(packet->message.data());
			std::string recovered;
			
			CryptoPP::ECB_Mode< CryptoPP::AES >::Decryption d;
			d.SetKey( serverAESkey, sizeof(serverAESkey) - 1 );

			
			CryptoPP::StringSource ss(cipher, true, new CryptoPP::StreamTransformationFilter(d, new CryptoPP::StringSink(recovered)));

			cout << "recovered text: " << recovered << endl;
			
			std::string to_user_name(packet->to_user_name.data());
			if(to_user_name.empty()) {
				//broadcast message to everyone except author
				SendChatMessage(peer, to_user_name, packet->message.data(), 0);
			} else {
				//send private message
				SendChatMessage(peer, to_user_name, packet->message.data(), 1);
			}
			
			break;
		}
		default:
			cout << "received unknown packet! " << (int)ppkt->type << endl;
			break;
	}
}

int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		cout << "An error occurred while initializing ENet." << endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	server_start(54301);
	
	while(1) {
		server_wait_for_packet();
	}
	
	return 0;
}
