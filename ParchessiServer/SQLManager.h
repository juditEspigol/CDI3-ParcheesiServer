#pragma once

#include "mysql_connection.h"

#define SQL_IP "127.0.0.1:3306"
#define SQL_USER "root"
#define SQL_PASSWORD "" //"enti"

#define SQL_DATABASE "TCP_Parchessi"

class SQLManager {
private:
	SQLManager() = default;
	~SQLManager() = default;
	SQLManager(const SQLManager&) = delete;
	SQLManager operator =(const SQLManager&) = delete;

	sql::Driver* driver;
	sql::Connection* connection;

	void CheckUsersTable();

public:
	inline static SQLManager& Instance() {
		static SQLManager instance;
		return instance;
	}

	void ConnectDatabase();
	bool InsertUser(std::string username, std::string password);
	int CheckLogin(std::string username, std::string password);
	void DisconnectDatabase();
};