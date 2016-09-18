#include "Commands.hpp"
Command Command::singleton = Command();
void arg_convert(Arg& a) {
	if(a.type == Arg::t_string) {
		try {
			a.i = std::stoi(a.s);
			a.type = Arg::t_int;
		} catch(...) {
		}
	}
}
