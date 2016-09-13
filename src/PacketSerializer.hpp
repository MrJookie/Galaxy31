#ifndef PACKET_SERIALIZER_HPP
#define PACKET_SERIALIZER_HPP

#include <enet/enet.h>
#include <cstring>
#include <string>
#include <map>
#include <utility>

#define INITIAL_ALLOCATION 1500
void packetFreeCallback(ENetPacket* pkt) {
	delete[] pkt->data;
}

class PacketSerializer {
	// num_keys, keys_offset, data..., keys
	private:
		using key_type = unsigned short;
		using size_type = unsigned short;
		size_type m_size;
		size_type m_allocated_size;
		size_type m_keys_offset;
		char* m_data;
		bool m_sent;
		bool m_readonly;
		const int bytes_per_key = sizeof(key_type)+sizeof(size_type)*2;
		
		// key, (offset, length)
		std::map<key_type, std::pair<size_type, size_type>> m_offsets;
		
		void parsePacket() {
			if(!m_offsets.empty())
				m_offsets.clear();
			
			if(m_size < 4) return;
			
			// read keys
			size_type num_keys;
			size_type* s = (size_type*)m_data;
			num_keys = s[0];
			m_keys_offset = s[1];
			
			if(num_keys < 0 || m_keys_offset < 0 || m_keys_offset >= m_size || m_keys_offset + num_keys*bytes_per_key >= m_size) return;
			
			size_type *keys = (size_type*)(m_data+m_keys_offset);
			for(int i=0; i < num_keys; i++) {
				if(keys[i*3+1] > m_size) {
					m_offsets.clear();
					return;
				}
				m_offsets[keys[i*3]] = std::make_pair((size_type)keys[i*3+1], (size_type)keys[i*3+2]);
			}
		}
		
		void alloc(int size) {
			if(m_size+size > m_allocated_size) {
				m_allocated_size = m_allocated_size*3/2;
				char* new_alloc = new char[m_allocated_size];
				memcpy(new_alloc, m_data, m_size);
				delete[] m_data;
				m_data = new_alloc;
			}
		}
		
		inline int keys_size() {
			return m_offsets.size()*bytes_per_key;
		}
		
		void appendKeysToPacket() {			
			m_keys_offset = m_size;
			
			// make sure keys can fit
			alloc(keys_size());
			
			size_type* s = (size_type*)m_data;
			s[0] = m_offsets.size();
			s[1] = m_keys_offset;
			
			size_type* write = (size_type*)&m_data[m_keys_offset];
			int w=0;
			for(auto& i : m_offsets) {
				write[w*3] = (size_type)i.first;
				write[w*3+1] = (size_type)i.second.first;
				write[w*3+2] = (size_type)i.second.second;
				w++;
			}
		}
		
	public:
	
		PacketSerializer(ENetPacket* pkt) {
			m_data = (char*)pkt->data;
			m_size = pkt->dataLength;
			m_readonly = true;
			parsePacket();
		}
		
		PacketSerializer(char* data, int length) {
			m_data = data;
			m_size = length;
			m_readonly = true;
			parsePacket();
		}
		
		PacketSerializer(const PacketSerializer& p, int additional_alloc = 512) {
			m_allocated_size = p.m_size + additional_alloc;
			m_data = new char[m_allocated_size];
			m_offsets = p.m_offsets;
			m_sent = false;
			m_readonly = false;
			m_keys_offset = 0;
			m_size = p.m_size;
			if(p.m_keys_offset != 0) {
				m_size -= keys_size();
			}
			memcpy(m_data, p.m_data, m_size);
		}
		
		PacketSerializer(int initial_allocation = INITIAL_ALLOCATION) {
			m_size = sizeof(size_type)*2;
			
			if(initial_allocation < 0) initial_allocation = INITIAL_ALLOCATION;
			m_data = new char[initial_allocation];
			m_allocated_size = initial_allocation;
			m_sent = false;
			m_readonly = false;
			m_keys_offset = 0;
		}
		
		~PacketSerializer() {
			if(!m_sent && !m_readonly && m_data) {
				delete [] m_data;
			}
		}
		
		// pkt["hehe"] = std::string("something")
		// char *buffer = allocate("hehe", 500);
		
		// auto h = get("hehe");
		// memcpy(h.first, whatever, h.second);
		
		char* data() {
			if(m_keys_offset == 0)
				appendKeysToPacket();
			return m_data; 
		}
		size_t size() { return m_size+keys_size(); }
		
		size_t allocated_size() { return m_allocated_size; }
		
		std::pair<char*,int> get(const std::string& key) {
			auto it = m_offsets.find(std::hash<std::string>{}(key));
			if(it != m_offsets.end())
				return std::make_pair((char*)&m_data[it->second.first], (int)it->second.second);
			else
				return std::make_pair((char*)0,(int)0);
		}
		
		int get_int(const std::string& key) {
			auto p = get(key);
			if(p.first && p.second == sizeof(int))
				return *((int*)p.first);
			return -1;
		}
		
		std::string get_string(const std::string& key) {
			auto p = get(key);
			if(p.first)
				return std::string(p.first, p.second);
			else
				return std::string("");
		}
		
		char* allocate(const std::string& key, size_type size) {
			if(m_sent || m_readonly) return 0;
			alloc(size);
			m_offsets[std::hash<std::string>{}(key)] = std::make_pair(m_size, size);
			int ofs = m_size;
			m_size += size;
			m_keys_offset = 0;
			return &m_data[ofs];
		}
		
		void put(const std::string& key, const std::string& value) {
			char* buffer = allocate(key, value.size());
			if(buffer)
				memcpy(buffer, value.data(), value.size());
		}
		
		void put(const std::string& key, int value) {
			char* buffer = allocate(key, sizeof(int));
			*((int*)(buffer)) = value;
		}
		
		void make_writeable() {
			// to implement
		}
		
		void send(ENetPeer* peer, int channel, int flags) {
			if(m_readonly) return;
			if(m_keys_offset == 0)
				appendKeysToPacket();
			ENetPacket* pkt = enet_packet_create(m_data, size(), flags | ENET_PACKET_FLAG_NO_ALLOCATE);
			pkt->freeCallback = packetFreeCallback;
			m_sent = true;
			enet_peer_send(peer, channel, pkt);
		}
		
		void broadcast(ENetHost* host, int channel, int flags) {
			if(m_readonly) return;
			if(m_keys_offset == 0)
				appendKeysToPacket();
			ENetPacket* pkt = enet_packet_create(m_data, size(), flags | ENET_PACKET_FLAG_NO_ALLOCATE);
			pkt->freeCallback = packetFreeCallback;
			m_sent = true;
			enet_host_broadcast(host, channel, pkt);
		}
		
		void release() {
			m_data = 0;
			m_size = 0;
		}
};

#endif
