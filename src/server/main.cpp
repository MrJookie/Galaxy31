#include <iostream>
#include "server.hpp"
#include "database.hpp"

#include <mysql++.h>

#include <chrono>
#include <thread>

#define HAVE_THREADS

using namespace std;

#if defined(HAVE_THREADS)
/*
// Define a concrete ConnectionPool derivative.  Takes connection
// parameters as inputs to its ctor, which it uses to create the
// connections we're called upon to make.  Note that we also declare
// a global pointer to an object of this type, which we create soon
// after startup; this should be a common usage pattern, as what use
// are multiple pools?
class SimpleConnectionPool : public mysqlpp::ConnectionPool
{
public:
	// The object's only constructor
	SimpleConnectionPool(const char *db, const char *server, const char *user, const char *password, unsigned int port) :
	conns_in_use_(0),
	db_(db),
	server_(server),
	user_(user),
	password_(password),
	port_(port)
	{
	}

	// The destructor.  We _must_ call ConnectionPool::clear() here,
	// because our superclass can't do it for us.
	~SimpleConnectionPool()
	{
		clear();
	}

	// Do a simple form of in-use connection limiting: wait to return
	// a connection until there are a reasonably low number in use
	// already.  Can't do this in create() because we're interested in
	// connections actually in use, not those created.  Also note that
	// we keep our own count; ConnectionPool::size() isn't the same!
	mysqlpp::Connection* grab()
	{
		while (conns_in_use_ > 8) {
			cout.put('R'); cout.flush(); // indicate waiting for release
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));	
		}

		++conns_in_use_;
		return mysqlpp::ConnectionPool::grab();
	}

	// Other half of in-use conn count limit
	void release(const mysqlpp::Connection* pc)
	{
		mysqlpp::ConnectionPool::release(pc);
		--conns_in_use_;
	}

protected:
	// Superclass overrides
	mysqlpp::Connection* create()
	{
		// Create connection using the parameters we were passed upon
		// creation.  This could be something much more complex, but for
		// the purposes of the example, this suffices.
		cout << "C" << endl; cout.flush(); // indicate connection creation
		return new mysqlpp::Connection(
				db_.empty() ? 0 : db_.c_str(),
				server_.empty() ? 0 : server_.c_str(),
				user_.empty() ? 0 : user_.c_str(),
				password_.empty() ? "" : password_.c_str());
				//add port?
	}

	void destroy(mysqlpp::Connection* cp)
	{
		// Our superclass can't know how we created the Connection, so
		// it delegates destruction to us, to be safe.
		cout << "D" << endl; cout.flush(); // indicate connection destruction
		delete cp;
	}

	unsigned int max_idle_time()
	{
		// Set our idle time at an example-friendly 3 seconds.  A real
		// pool would return some fraction of the server's connection
		// idle timeout instead.
		return 3;
	}

private:
	// Number of connections currently in use
	unsigned int conns_in_use_;

	// Our connection parameters
	std::string db_, server_, user_, password_;
	unsigned int port_;
};
*/
SimpleConnectionPool* poolptr = 0;

static void* worker_thread(void *running_flag)
{
	// Ask the underlying C API to allocate any per-thread resources it
	// needs, in case it hasn't happened already.  In this particular
	// program, it's almost guaranteed that the safe_grab() call below
	// will create a new connection the first time through, and thus
	// allocate these resources implicitly, but there's a nonzero chance
	// that this won't happen.  Anyway, this is an example program,
	// meant to show good style, so we take the high road and ensure the
	// resources are allocated before we do any queries.
	mysqlpp::Connection::thread_start();
	cout.put('S'); cout.flush(); // indicate thread started

	// Pull data from the sample table a bunch of times, releasing the
	// connection we use each time.
	for (size_t i = 0; i < 6; ++i) {
		// Go get a free connection from the pool, or create a new one
		// if there are no free conns yet.  Uses safe_grab() to get a
		// connection from the pool that will be automatically returned
		// to the pool when this loop iteration finishes.
		mysqlpp::ScopedConnection cp(*poolptr, true);
		if (!cp) {
			cerr << "Failed to get a connection from the pool!" << endl;
			break;
		}

		// Pull a copy of the sample stock table and print a dot for
		// each row in the result set.
		mysqlpp::Query query(cp->query("select * from accounts"));
		mysqlpp::StoreQueryResult res = query.store();
		for (size_t j = 0; j < res.num_rows(); ++j) {
			cout.put('.');
		}

		// Delay 1-4 seconds before doing it again.  Because this can
		// delay longer than the idle timeout, we'll occasionally force
		// the creation of a new connection on the next loop.
		
		//std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 4000 + 1000));	
	}

	// Tell main() that this thread is no longer running
	*reinterpret_cast<bool*>(running_flag) = false;
	cout.put('E'); cout.flush(); // indicate thread ended
	
	// Release the per-thread resources before we exit
	mysqlpp::Connection::thread_end();

	return 0;
}


void database_start(const char *db, const char *server=0, const char *user=0, const char *password=0, unsigned int port=3306) {
	#if defined(HAVE_THREADS)

		// Create the pool and grab a connection.  We do it partly to test
		// that the parameters are good before we start doing real work, and
		// partly because we need a Connection object to call thread_aware()
		// on to check that it's okay to start doing that real work.  This
		// latter check should never fail on Windows, but will fail on most
		// other systems unless you take positive steps to build with thread
		// awareness turned on.  See README-*.txt for your platform.
		poolptr = new SimpleConnectionPool(db, server, user, password, port);
		try {
			mysqlpp::ScopedConnection cp(*poolptr, true);
			if (!cp->thread_aware()) {
				cerr << "MySQL++ wasn't built with thread awareness!" << endl;
				return;
			}
		}
		catch (mysqlpp::Exception& e) {
			cerr << "Failed to set up initial pooled connection: " <<
					e.what() << endl;
			return;
		}

		// Setup complete.  Now let's spin some threads...
		cout << endl << "Pool created and working correctly.  Now to do some real work..." << endl;
		
		srand((unsigned int)time(0));
		bool threadBusy[] = { false, false };
		const size_t num_threads = sizeof(threadBusy) / sizeof(threadBusy[0]);

		size_t i;
		thread *t = new thread[num_threads];
		for( i = 0; i < num_threads; i++) {
			t[i] = thread(worker_thread, threadBusy + i);
		}

		for(int i=0; i < num_threads; i++) {
			t[i].detach();
		}
		
		/*
		// Test the 'running' flags every second until we find that they're
		// all turned off, indicating that all threads are stopped.
		cout.put('W'); cout.flush(); // indicate waiting for completion
		do {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			i = 0;
			while (i < num_threads && !running[i]) ++i;
		}
		while (i < num_threads);
		cout << endl << "All threads stopped!" << endl;

		// Shut it all down...
		delete poolptr;
		cout << endl;
		*/
	#else
		cout << "Threads are not enabled (defined)" << endl;
	#endif
}
#endif



std::string test(int a, int b) {
	std::cout << "mysql thread\n";
	if(a > b)
		return "a > b";
	else
		return "a < b";
}

void cont(std::string a) {
	std::cout << "calling thread\n";
	std::cout << "result: " << a << std::endl;
}



RelocatedWork w;

void mysql_thread() {
/*		
		std::thread::id this_id = std::this_thread::get_id();
		std::cout << "thread id " << this_id << std::endl;
*/		
	while(1) {
		w.Work();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

int main(int argc, char* argv[]) {
	if (enet_initialize () != 0) {
		std::cout << "An error occurred while initializing ENet." << std::endl;
		return -1;
	}
	atexit (enet_deinitialize);
	
	mysql_connect("test", "89.177.76.215", "root", "Galaxy31");
	
	/*
	//mysql_connect("test", "89.177.76.215", "root", "Galaxy31");
	
	//////////////////
	std::cout << "-----------------" << std::endl;
	
	
	int new_account_id = createAccount("email@email.com", "username", "plain_password", "127.0.0.1");
	if(new_account_id > 0) {
		std::cout << "Account: New account created!" << std::endl;
	} else if(new_account_id == 0) {
		std::cout << "Account: Could not create new account! Email or username already exists!" << std::endl;
	} else {
		std::cout << "Account: Unspecified error occured!" << std::endl;
	}
	
	int login_account_id = loginAccount("email@email.com", "plain_password", "127.0.0.1");
	if(login_account_id > 0) {
		std::cout << "Account: Login succeed." << std::endl;
		
		mysqlpp::Row loggedUser = getExistingUser(login_account_id);
		std::cout << loggedUser["id"] << " | " << loggedUser["email"] << std::endl;
	} else if(login_account_id == 0) {
		std::cout << "Account: Login failed! Account is banned." << std::endl;
	} else {
		std::cout << "Account: Login failed! Wrong email or password." << std::endl;
	}
	*/
	
	/*
	std::vector<mysqlpp::Row> accounts(getAllAccountsVec());
	for(const auto& account : accounts) {
		std::cout << account["email"] << std::endl;
	}
	*/
	
	std::cout << "-----------------" << std::endl;
	/////////////////
	
	database_start("test", "89.177.76.215", "root", "Galaxy31");
	
	server_start(1234, &w, poolptr);

	std::thread mysql_thrd(mysql_thread);
	mysql_thrd.detach();

	for(int i = 0; i < 10; i++) {
		w.MakeWork(
			[](mysqlpp::Row row) { std::cout << "got: " << row["username"] << std::endl; },
			[]() -> mysqlpp::Row {
				mysqlpp::Query query(con.query("SELECT * FROM accounts"));
				mysqlpp::StoreQueryResult res = query.store();
				
				for (size_t j = 0; j < res.num_rows(); ++j) {
					cout << res[j]["username"] << endl;
				}
								
				
				return res[0];
			}
		);
	}
	
	/*
	for(int i = 0; i < 10; i++) {
		w.MakeWork(
			[](mysqlpp::Row row) { std::cout << "got: " << row["username"] << std::endl; },
			[](SimpleConnectionPool* poolptr) -> mysqlpp::Row { 
				mysqlpp::Connection::thread_start();
				
				mysqlpp::ScopedConnection cp(*poolptr, true);
				if (!cp) {
					cerr << "Failed to get a connection from the pool!" << endl;
					throw std::string("Failed to get a connection from the pool!"); //comment out?
				}
				
				mysqlpp::Query query(cp->query("SELECT * FROM accounts"));
				mysqlpp::StoreQueryResult res = query.store();
				
				for (size_t j = 0; j < res.num_rows(); ++j) {
					cout << res[j]["username"] << endl;
				}
	
				std::cout << "Conns in use: " << poolptr->GetConnsInUse() << std::endl;
								
				mysqlpp::Connection::thread_end();
				
				return res[0];
			},
		poolptr);
	}
	*/
	
	while(1) {
		server_wait_for_packet();
		
		while(w.HasWork()) {
			w.Continue();
		}
	}
	
	return 0;
}
