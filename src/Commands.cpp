#include "commands/commands.hpp"

namespace Commands {
	
	COMMAND(std::string, concat, (std::vector<Arg> args)) {
		// cout << "func: " << a << ", " << b << endl;
		std::string str = "";
		for(auto& a : args) {
			if(a.type == Arg::t_int) 
				str = str + std::to_string(a.i);
			if(a.type == Arg::t_string) {
				str += a.s;
			} 
		}
		return str;
	}

	COMMAND(void, loop, (int n, Arg e)) {
		std::vector<Arg> arg;
		for(int i=0; i < n; i++)
			Command::Execute(e, arg);
	}
	
	void Init() {

	}
	
}
