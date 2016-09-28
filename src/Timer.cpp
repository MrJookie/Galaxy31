#include <list>
#include "EventSystem/Event.hpp"
#include <iostream>

namespace Timer {
	
Event::id_type timer_id;
Event::id_type timer_listen_id;

typedef double time_type;
struct container {
	Event::id_type listener_id;
	time_type fixed_time;
	time_type time;
};

std::list<container> time_list;
time_type time_passed;

// forwards
void process_timer(time_type time_step);
void addToList( container &c );
// void HandleListeners( bool add, Event::id_type listener_id, int time );


void process_timer(time_type time_step) {

	Event::id_type send_to_listener_id = 0;
	time_type time_p;

	time_passed += time_step;
	if( time_passed >= time_list.front().time ) {
		container c = time_list.front();
		time_list.pop_front();
		send_to_listener_id = c.listener_id;
		time_p = time_passed - c.time;
		addToList( c );
	}

	// std::cout << "on check event ... sending event to: " << send_to_listener_id << std::endl;
	if( send_to_listener_id != 0 )
		Event::Emit( send_to_listener_id, time_p );
}


void addToList( container &c ) {
	c.time = c.fixed_time;
	c.time += time_passed;
	if( time_passed > c.time ) {
		// reset time_passed
		for( auto &e : time_list ) {
			e.time -= time_passed;
		}
		time_passed = 0;
	}

	if( time_list.size() > 0 ) {
		// find place to insert
		bool found = false;
		for( auto it = time_list.begin(); it != time_list.end(); it++ ) {
			if( it->time > c.time ) {
				time_list.insert( it, c );
				found = true;
				break;
			}
		}
		if( !found )
			time_list.push_back( c );
	} else {
		time_list.push_front( c );
	}
}


void HandleListeners( bool add, Event::id_type listener_id, time_type time ) {
	if(add) {
		// std::cout << "registered new listener: " << listener_id << " , with param " << time << std::endl;
		container c;
		c.listener_id = listener_id;
		c.time = time;
		c.fixed_time = time;
		if(time_list.empty())
			timer_listen_id = Event::Listen("tick", process_timer);
		addToList( c );
	} else {
		for( auto it = time_list.begin(); it != time_list.end(); it++ ) {
			// std::cout << it->listener_id << ", " << listener_id << std::endl;
			if( it->listener_id == listener_id ) {
				time_list.erase( it );
				if(time_list.empty()) {
					Event::StopListening( timer_listen_id );
					// std::cout << "stopping listening\n";
				}
				break;
			}
		}
		// std::cout << "on remove listener ..." << std::endl;
	}
}




void Init() {
	time_passed = 0;
	timer_id = Event::Register("timer", HandleListeners);
}

}
