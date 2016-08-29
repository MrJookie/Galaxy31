#ifndef RELOCATED_WORK_HPP
#define RELOCATED_WORK_HPP
#include <queue>
#include <functional>
#include <mutex>
class RelocatedWork {
	private:
		std::queue<std::function<void()>> m_work;
		std::queue<std::function<void()>> m_continue;
		std::mutex m_mutex;
	public:
	
		bool HasWork() { 
			std::unique_lock<std::mutex> l(m_mutex);
			return !m_work.empty();
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
		void Work() {
			while(!m_work.empty()) {
				std::function<void()> work_func;
				
				{
					std::unique_lock<std::mutex> l(m_mutex);
					if(m_work.empty()) return;
					work_func = m_work.front();
					m_work.pop();
				}
				
				work_func();
			}
		}
		
		template<typename T>
		struct memfun_type
		{
			using type = void;
		};

		template<typename Ret, typename Class, typename... Args>
		struct memfun_type<Ret(Class::*)(Args...) const>
		{
			using type = std::function<Ret(Args...)>;
			using ret = Ret;
		};
		
		template <typename W, typename... Args>
		void MakeWork( std::function<void(typename memfun_type<decltype(&W::operator())>::ret)> continuation_function, 
					   W work_function,
					   Args... args ) 
		{
			m_work.push([=](){
		
				typename memfun_type<decltype(&W::operator())>::ret result = work_function(args...);
				
				std::unique_lock<std::mutex> l(m_mutex);
				m_continue.push([=]() {
					continuation_function( result );
				});
			});
		}
		
		template <typename Ret, typename... Args>
		void MakeWork( void(*continuation_function)(Ret), 
					   Ret (*work_function)(Args...),
					   Args... args ) 
		{
			m_work.push([=](){
				Ret result = work_function(args...);
				
				std::unique_lock<std::mutex> l(m_mutex);
				m_continue.push([=]() {
					continuation_function( result );
				});
			});
		}
		
	
};


#endif
