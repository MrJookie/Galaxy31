#ifndef SERVER_NETWORK_HPP
#define SERVER_NETWORK_HPP

enum Channel {
	control,
	msg
};

enum PacketType {
	new_id,
	update
};


#endif
