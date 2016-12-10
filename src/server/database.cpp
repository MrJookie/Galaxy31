#include "database.hpp"
//crypto
#include <cryptopp/sha.h>
#include <cryptopp/filters.h>
#include <cryptopp/hex.h>


mysqlpp::Connection mysql_connect(const char *mdb, const char *mserver, const char *muser, const char *mpassword, unsigned short mport) {
	mysqlpp::Connection con;
	con.set_option(new mysqlpp::ReconnectOption(true));
	//con.set_option(new mysqlpp::ConnectTimeoutOption(5));
	
	//con.set_option(new mysqlpp::MultiStatementsOption(true));
	
	if(con.connect(mdb, mserver, muser, mpassword, mport)) {
        cout << "Connected to MySQL server: " << mserver << ":" << mport << endl;
    } else {
		cout << con.error() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	/*
	if(!con.thread_aware()) {
        cout << "Mysql++ not compiled with threading support" << endl;
    } else {
        cout << "Mysql++ compiled with threading support" << endl;
    }
    */
    
    return con;
}

//add ip_addr INET_ATON(ip_of_user);
int createAccount(mysqlpp::Connection& con, std::string email, std::string userName, std::string password, std::string ipAddr) {
	mysqlpp::Query query = con.query();
    query << "SELECT id FROM accounts WHERE email = '" << mysqlpp::escape << email << "' OR username = '" << mysqlpp::escape << userName << "' LIMIT 1";
    
    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		return 0;
	} else {
		query << "INSERT INTO accounts (id, email, username, password, active, ip_addr, datetime_registered) VALUES ("
			  << "'', "
			  << "'" << mysqlpp::escape << email << "', "
			  << "'" << mysqlpp::escape << userName << "', "
			  << "'" << mysqlpp::escape << password << "', "
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
int loginAccount(mysqlpp::Connection& con, std::string email, std::string password, std::string ipAddress, int challenge) {
	mysqlpp::Query query = con.query();
    query << "SELECT id, active, password FROM accounts WHERE email = '" << mysqlpp::escape << email << "' LIMIT 1";

    mysqlpp::StoreQueryResult res = query.store();
    if(res.num_rows() > 0) {
		CryptoPP::SHA1 sha1;
		std::string db_pass(res[0]["password"]);
		std::string source = db_pass + std::to_string(challenge);
		std::string hash = "";
		CryptoPP::StringSource(source, true, new CryptoPP::HashFilter(sha1, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hash), false)));
	  
		if(hash == password) {
			query << "UPDATE accounts SET ip_addr = INET_ATON('" << mysqlpp::escape << ipAddress << "'), datetime_last_login = NOW() WHERE id = " << mysqlpp::escape << res[0]["id"];
			query.execute();
			
			if((int)res[0]["active"] > 0) {
				return res[0]["id"];
			} else {
				return 0;
			}
		}
	}
	
	return -1;
}

mysqlpp::Row getExistingUser(mysqlpp::Connection& con, unsigned int account_id) {
	try {
		mysqlpp::Query query = con.query();
		query << "SELECT * FROM accounts WHERE id = " << mysqlpp::escape << account_id << " LIMIT 1";
		
		mysqlpp::StoreQueryResult res = query.store();
		if(res.num_rows() > 0) {
			return res[0];
		}
		
		return mysqlpp::Row();
	} catch(...) {}
}

int flushPlayerData(mysqlpp::Connection& con, std::string statement) {
	mysqlpp::Query query = con.query();
	
	query << statement;
 
	if(query.execute()) {
		return 1;
	}
	
	return 0;
}

/*
std::vector<mysqlpp::Row> getAllAccountsVec(mysqlpp::Connection& con) {
	std::vector<mysqlpp::Row> result;
	
	mysqlpp::Query query = con.query("SELECT * FROM accounts");
	query.storein(result);
	
	return result;
}
*/
