#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <iostream>
using namespace std;

#include <map>
#include <vector>
#include <string>
#include <exception>
#include <functional>
#include <tuple>
#include <type_traits>

class CommandException {
	private:
		std::string reason;
	public:
	CommandException(std::string s) {
		reason = s;
	}
	std::string& what() { return reason; }
};


// ----- function type information
template< typename t, std::size_t n, typename = void >
struct function_type_information;

template< typename r, typename ... a, std::size_t n >
struct function_type_information< r (*)( a ... ), n > {
	using type = typename std::tuple_element< n, std::tuple< a ... > >::type; 
	using tuple = std::tuple<a...>;
	using result = r;
};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_type_information< r (c::*)( a ... ), n >
	: function_type_information< r (*)( a ... ), n > {};

template< typename r, typename c, typename ... a, std::size_t n >
struct function_type_information< r (c::*)( a ... ) const, n >
	: function_type_information< r (*)( a ... ), n > {};

template< typename ftor, std::size_t n >
struct function_type_information< ftor, n,
	typename std::conditional< false, decltype( & ftor::operator () ), void >::type >
	: function_type_information< decltype( & ftor::operator () ), n > {};
// --------------------------------------

struct Arg {
	enum { t_void, t_int, t_float, t_double, t_char, t_charp, t_string,  // basic types
		   t_command, t_deferred, t_variable }
		type;
	union {
		int i;
		float f;
		double d;
		char c;
		char* cp;
		std::string s;
	};
	Arg() : s() { type = t_void; }
	Arg& operator=(const Arg& a) {
		if(a.type == t_int)
			i = a.i;
		else if(a.type == t_float)
			f = a.f;
		else if(a.type == t_double)
			d = a.d;
		else if(a.type == t_char)
			c = a.c;
		else if(a.type == t_string)
			s = a.s;
		type = a.type;
		
		return *this;
	}
	Arg(const Arg& a) : s() {
		*this = a;
	}
	operator int() {
		if(type == t_int)
			return i;
		else if(type == t_string) {
			return std::stoi(s);
		}
	}
	~Arg(){}
};
		
// --------- function call: Tuple as arguments
namespace detail
{
    template <typename ret, typename F, typename Tuple, bool Done, int Total, int... N>
    struct call_impl {
        static ret call(F& f, Tuple && t) {
			return call_impl<ret, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
        }
    };

    template <typename ret, typename F, typename Tuple, int Total, int... N>
    struct call_impl<ret, F, Tuple, true, Total, N...> {
        static ret call(F& f, Tuple && t) {
			return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };
}

// user invokes this
template <typename ret, typename F, typename Tuple>
ret call(F& f, Tuple && t) {
    using ttype = typename std::decay<Tuple>::type;
	return detail::call_impl<ret, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
}
// -----------------------

void arg_convert(Arg& a);
		
class Command {
	private:
		using adapter_function_type = std::function<Arg(std::vector<Arg>& args)>;
		
		// command name -> index of vector which contains commands
		std::map<std::string, int> m_commands_map;
		std::map<std::string, int> m_variables_map;
		
		std::vector<adapter_function_type> m_commands;
		std::vector<Arg> m_variables;
		
		
	
		template<bool done, int n, int N>
		struct gen_adapter;
		
		// not done
		template<bool done, int n, int N>
		struct gen_adapter {
		
			static void handle_element(int& i, std::vector<Arg>& args) {
				if(args[n].type == Arg::t_int) {
					i = args[n].i;
				} else if(args[n].type == Arg::t_string) {
					i = std::stoi(args[n].s);
				}
				else
					throw CommandException("cannot convert to integer");
			}
			static void handle_element(char& c, std::vector<Arg>& args) {
				if(args[n].type == Arg::t_char)
					c = args[n].c;
			}
			static void handle_element(float& f, std::vector<Arg>& args) {
				if(args[n].type == Arg::t_float) {
					f = args[n].f;
				} else if(args[n].type == Arg::t_string) {
					f = std::stof(args[n].s);
				}
			}
			static void handle_element(double& d, std::vector<Arg>& args) {
				d = args[n].d;
			}
			static void handle_element(Arg& a, std::vector<Arg>& args) {
				a = args[n];
			}
			static void handle_element(std::string& s, std::vector<Arg>& args) {
				if(args[n].type == Arg::t_string) {
					s = args[n].s;
					if(n+1 == N && args.size() != N) {
						for(int i=n+1; i < args.size(); i++) {
							if(args[i].type != Arg::t_string) break;
							s += " " + args[i].s;
						}
					}
				}
			}
			static void handle_element(std::vector<Arg>& e, std::vector<Arg>& args) {
				e.resize(args.size()-n);
				for(int i=n; i < args.size(); i++) {
					e[i-n] = args[i];
				}
			}
			
			
			template<typename F, typename Tuple>
			static Arg adapter_function(F func, std::vector<Arg>& args, Tuple &tuple) {
				using type = typename std::tuple_element<n,Tuple>::type;
				auto& elem = std::get<n>(tuple);
				
				handle_element(elem, args);
				
				return gen_adapter<n+1==N, n+1, N>::adapter_function(func, args, tuple);
			}
		};
		
		// done
		template<int n, int N>
		struct gen_adapter<true,n,N> {
			
			static void handle_result(Arg& arg, int r) {
				arg.type = Arg::t_int;
				arg.i = r;
			}
			static void handle_result(Arg& arg, char r) {
				arg.type = Arg::t_char;
				arg.c = r;
			}
			static void handle_result(Arg& arg, float r) {
				arg.type = Arg::t_float;
				arg.f = r;
			}
			static void handle_result(Arg& arg, double r) {
				arg.type = Arg::t_double;
				arg.d = r;
			}
			static void handle_result(Arg& arg, const Arg& a) {
				arg = a;
			}
			
			template<typename F, typename Tuple>
			static Arg adapter_function(F func, std::vector<Arg>& args, Tuple &tuple) {
				
				using result = typename function_type_information<F,0>::result;
				Arg a;
				handle_result(a, call<result>(func, tuple));
				arg_convert(a);
				return a;
			}
		};
		
		template<typename Tuple, typename F>
		adapter_function_type make_adapter(F& func) {
			return [=](std::vector<Arg>& vec) -> Arg {
				Tuple tuple;
				return gen_adapter<std::tuple_size<Tuple>::value==0, 0, std::tuple_size<Tuple>::value>::adapter_function(func, vec, tuple);
			};
		}
		
		
		static Command singleton;
		
	public:
		Command() {
			AddCommand("get", [&](std::string variable) -> Arg {
				return Get(variable);
			});
			AddCommand("set", [&](std::string variable, Arg v) -> int {
				Set(variable, v);
			});
		}
	
		
		
		template<typename F>
		bool add_command(const std::string name, F func) {
			m_commands.push_back(make_adapter<typename function_type_information<F,0>::tuple> (func));
			m_commands_map[name] = m_commands.size()-1;
		}
		
		
		Arg get(std::string variable) {
			auto it = m_variables_map.find(variable);
			if(it != m_variables_map.end()) {
				return m_variables[it->second];
			}
			Arg a;
			a.type = Arg::t_void;
			return a;
		}
		
		void set(std::string variable, Arg value) {
			auto it = m_variables_map.find(variable);
			if(it != m_variables_map.end()) {
				m_variables[it->second] = value;
			} else {
				m_variables.push_back(value);
				m_variables_map[variable] = m_variables.size()-1;
			}
		}
		
		// [ command ( ) [ ] args... ]
		// command_index args ...
		// std::vector<Arg> 
		
		
		Arg execute(std::string command) {
			// parse which command and which parameters are given
			std::string cmd; // find out
			int last_space = 0;
			int space = command.find(' ');
			cmd = command.substr(last_space,space);
			last_space = space+1;
			std::vector<Arg> args; // find out
			Arg a;
			while(space = command.find(' ', last_space)) {
				a.type = Arg::t_string;
				if(space == -1)
					a.s = command.substr(last_space);
				else
					a.s = command.substr(last_space, space-last_space);
				// cout << "adding: " << last_space << ", " << space << " : " << a.s << endl;
				args.push_back(a);
				if(space == std::string::npos)
					break;
				last_space = space+1;
			}
			auto it = m_commands_map.find(cmd);
			Arg ret;
			if(it != m_commands_map.end()) {
				ret = (m_commands[it->second])(args);
			} else {
				throw CommandException("unknown command");
			}
			return ret;
		}
		
		// ----------------------------------------------
		// singleton functions
		template<typename F>
		static bool AddCommand(const std::string& name, F func) {
			singleton.add_command(name, func);
		}
		static Arg Get(const std::string& variable) {
			return singleton.get(variable);
		}
		static void Set(const std::string& variable, Arg value) {
			singleton.set(variable, value);
		}
		static Arg Execute(const std::string& command) {
			singleton.execute(command);
		}
		// -----------------------------------------------
};

#define COMMAND(ret, name, prototype) \
	ret _cmd_##name prototype; \
	bool _cmd_##name##_cmd_ = Command::AddCommand( #name, (_cmd_##name) ); \
	ret _cmd_##name prototype


#endif
