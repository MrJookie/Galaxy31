#ifndef SERVER_DATABASE_HPP
#define SERVER_DATABASE_HPP
#include <string>
#include <vector>
#include <iostream>
#include <mysql++.h>
#include <thread>

using namespace std;

extern mysqlpp::Connection con;

int createAccount(std::string email, std::string userName, std::string password, std::string ipAddr); //returns user_id ( > 0 ) = ok, -1 = email/username exists, -2 = unspecified error
int loginAccount(std::string email, std::string password, std::string ipAddress, int challenge); //returns user_id ( > 0 ) = ok, 0 = login ok but account is banned (accounts.active = 0), -1 = wrong email/password
mysqlpp::Row getExistingUser(unsigned int account_id); //returns all user info in mysqlpp::Row

std::vector<mysqlpp::Row> getAllAccountsVec();
void mysql_connect(const char *db, const char *server=0, const char *user=0, const char *password=0, unsigned int port=3306);

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
	
	int GetConnsInUse() {
		return conns_in_use_;
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

#endif
