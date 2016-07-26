#include "database.hpp"

mysqlpp::Connection con;

using std::cout;
using std::endl;

void mysql_connect(const char *db, const char *server, const char *user, const char *password, unsigned int port) {
	if(con.connect(db, server, user, password, port)) {
        cout << "Connected to MySQL server: " << server << ":" << port << endl;
    } else {
		cout << con.error() << std::endl;
		exit(EXIT_FAILURE);
	}
}

//add ip_addr INET_ATON(ip_of_user);
int createAccount(std::string email, std::string userName, std::string password, std::string ipAddr) {
	mysqlpp::Query query = con.query();
    query << "SELECT id FROM accounts WHERE email = '" << mysqlpp::escape << email << "' AND username = '" << mysqlpp::escape << userName << "' LIMIT 1";
    
    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		return 0;
	} else {
		query << "INSERT INTO accounts (id, email, username, password, active, ip_addr, datetime_registered) VALUES ("
			  << "'', "
			  << "'" << mysqlpp::escape << email << "', "
			  << "'" << mysqlpp::escape << userName << "', "
			  << "SHA1('" << mysqlpp::escape << password << "'), "
			  << "1, "
			  << "INET_ATON('" << mysqlpp::escape << ipAddr << "'), "
			  << "NOW() "
			  << ")";
		if(query.execute()) {
			return query.insert_id();
		}
	}
	
	return -1;
}

//update ip_addr INET_ATON(ip_of_user);
int loginAccount(std::string email, std::string password, std::string ipAddr) {
	mysqlpp::Query query = con.query();
    query << "SELECT id, active FROM accounts WHERE email = '" << mysqlpp::escape << email << "' AND password = SHA1('" << mysqlpp::escape << password << "') LIMIT 1";
    
    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		query << "UPDATE accounts SET ip_addr = INET_ATON('" << mysqlpp::escape << ipAddr << "'), datetime_last_login = NOW()";
		query.execute();
		
		if((int)res[0]["active"] > 0) {
			return res[0]["id"];
		} else {
			return 0;
		}
	}
	
	return -1;
}

mysqlpp::Row getExistingUser(unsigned int account_id) {
	mysqlpp::Query query = con.query();
    query << "SELECT * FROM accounts WHERE id = " << mysqlpp::escape << account_id << " LIMIT 1";
    
    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		return res[0];
	}
	
	return mysqlpp::Row();
}

std::vector<mysqlpp::Row> getAllAccountsVec() {
	std::vector<mysqlpp::Row> result;
	
	mysqlpp::Query query = con.query("SELECT * FROM accounts");
	query.storein(result);
	
	return result;
}
