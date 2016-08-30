#ifndef RELOCATED_WORK_HPP
#define RELOCATED_WORK_HPP
#include <queue>
#include <functional>
#include <mutex>
class RelocatedWork {
	private:
		template<typename T>
		struct ref_k_tmpl {
			template<typename B, typename = typename std::enable_if<!std::is_same<ref_k_tmpl<T>, B>()>::type>
			ref_k_tmpl(B& a) : t(reinterpret_cast<T&>(a)) {}
			template<typename B>
			ref_k_tmpl(B* a) : t(reinterpret_cast<T&>(*a)) {}
			T& t;
			template<typename B>
			operator B& () { return reinterpret_cast<B&>(t); }
			template<typename B>
			operator B* () { return reinterpret_cast<B*>(&t); }
		};

		typedef ref_k_tmpl<int> ref_k;
		
		template<bool done, int n, typename... Args>
		struct tuple_skip_n_args;
		
		template<int n, typename Arg, typename... Args>
		struct tuple_skip_n_args<false, n, Arg, Args...> {
			using type = typename tuple_skip_n_args<n == 0 || n == 1, n-1, Args...>::type;
		};
		
		template<int n, typename... Args>
		struct tuple_skip_n_args<true, n, Args...> {
			using type = std::tuple<Args...>;
		};

		template< typename t, std::size_t n, typename = void >
		struct function_argument_type;

		template< typename r, typename ... a, std::size_t n >
		struct function_argument_type< r (*)( a ... ), n >
			{ 
			  // using type = typename std::tuple_element< n, std::tuple< a ... > >::type; 
			  using tuple = typename tuple_skip_n_args<n == 0, n, a...>::type;
			  using result = r;
			};

		template< typename r, typename c, typename ... a, std::size_t n >
		struct function_argument_type< r (c::*)( a ... ), n >
			: function_argument_type< r (*)( a ... ), n > {};

		template< typename r, typename c, typename ... a, std::size_t n >
		struct function_argument_type< r (c::*)( a ... ) const, n >
			: function_argument_type< r (*)( a ... ), n > {};

		template< typename ftor, std::size_t n >
		struct function_argument_type< ftor, n,
			typename std::conditional< false, decltype( & ftor::operator () ), void >::type >
			: function_argument_type< decltype( & ftor::operator () ), n > {};
		// --------------
		
		template<typename T>
		struct ThenObj{
			std::function<void()> func;
			T result;
			
			template<typename F>
			void then(F continue_function) {
				func = [=](){ 
					continue_function( result );
				};
			}
		};
	
		std::queue<std::function<void(ref_k)>> m_work;
		std::queue<std::function<void()>*> m_continue;
		std::mutex m_mutex;
		
	private:
		template <typename W, typename Tuple, std::size_t... N>
		auto& makeWork( W work_function, Tuple tuple, std::index_sequence<N...> ) {
			auto* ret = new ThenObj<typename function_argument_type<W,1>::result>();
			std::unique_lock<std::mutex> l(m_mutex);
			m_work.push([=](ref_k state){
				ret->result = work_function(state, std::get<N>(tuple)...);
				std::unique_lock<std::mutex> l(m_mutex);
				m_continue.push((std::function<void()>*)ret);
			});
			return *ret;
		}
		
	public:
	
		
			
		bool HasWork() { 
			std::unique_lock<std::mutex> l(m_mutex);
			return !m_work.empty();
		}
		
		bool HasResult() {
			std::unique_lock<std::mutex> l(m_mutex);
			return !m_continue.empty();
		}
		
		void Continue() {
			while(!m_continue.empty()) {
				std::function<void()>* continue_func;
				
				{
					std::unique_lock<std::mutex> l(m_mutex);
					if(m_continue.empty()) return;
					continue_func = m_continue.front();
					m_continue.pop();
				}
				
				if(*continue_func)
					(*continue_func)();
				delete continue_func;
			}
		}
		
		void Work(ref_k state) {
			while(!m_work.empty()) {
				std::function<void(ref_k)> work_func;
				
				{
					std::unique_lock<std::mutex> l(m_mutex);
					if(m_work.empty()) return;
					work_func = m_work.front();
					m_work.pop();
				}
				
				work_func(state);
			}
		}
		
		template <typename W, typename... Args>
		inline auto& MakeWork( W work_function, Args...  args ) {
			return makeWork<W, typename function_argument_type<W,1>::tuple> (std::forward<W>(work_function), {args...}, std::make_index_sequence<sizeof...(Args)>());
		}
		
		
		
};


#endif
