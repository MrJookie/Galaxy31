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
		
		// ---------- shitload of crap
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
			{ using type = typename std::tuple_element< n, std::tuple< a ... > >::type; 
			  using tuple = typename tuple_skip_n_args<n == 0, n, a...>::type;
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
		
		// -----------
		template <typename F, typename Tuple, bool Done, int Total, int... N>
		struct call_impl {
			static auto call(F f, Tuple && t) {
				return call_impl<F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
			}
		};

		template <typename F, typename Tuple, int Total, int... N>
		struct call_impl<F, Tuple, true, Total, N...> {
			static auto call(F f, Tuple && t) {
				return f(std::get<N>(std::forward<Tuple>(t))...);
			}
		};

		// user invokes this
		template <typename F, typename Tuple>
		auto call(F f, Tuple && t) {
			typedef typename std::decay<Tuple>::type ttype;
			return call_impl<F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
		}
		// -----------
	
		std::queue<std::function<void(ref_k)>> m_work;
		std::queue<std::function<void()>> m_continue;
		std::mutex m_mutex;
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
				std::function<void()> continue_func;
				
				{
					std::unique_lock<std::mutex> l(m_mutex);
					if(m_continue.empty()) return;
					continue_func = m_continue.front();
					m_continue.pop();
				}
				
				continue_func();
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
		
		template <typename F, typename W>
		void MakeWork( W work_function,
					   typename function_argument_type<W,1>::tuple tuple,
					   F continuation_function
					   ) 
		{
			std::unique_lock<std::mutex> l(m_mutex);
			m_work.push([=](ref_k state){
				auto result = call(work_function, std::tuple_cat(std::make_tuple(state), tuple)); //work_function(state, args...);
				std::unique_lock<std::mutex> l(m_mutex);
				m_continue.push([=]() {
					continuation_function( result );
				});
			});
		}
		
};


#endif
