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
	
		std::queue<std::function<void(ref_k)>> m_work;
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
		
		template <typename F, typename W, typename... Args>
		void MakeWork( F continuation_function, 
					   W work_function,
					   Args... args ) 
		{
			m_work.push([=](ref_k state){
				auto result = work_function(state, args...);
				std::unique_lock<std::mutex> l(m_mutex);
				m_continue.push([=]() {
					continuation_function( result );
				});
			});
		}
};


#endif
