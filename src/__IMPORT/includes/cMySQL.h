#ifndef cMySQLH
#define cMySQLH

#include <windows.h>
#include <winsock2.h>
#define __WIN__
#include <mysql.h>

class cMySQL {
private:
	MYSQL* mysql;
	MYSQL_RES* msres;
	int rows;
	int cols;
	st_mysql_rows *row;
public:
	cMySQL();
	~cMySQL();

	bool connect(const char* server, const char* user, const char* password, const char* database, const char* codepage = "utf8");

	bool query(char* query);
	bool select(char* query);
	int fetchcols();
	char*** fetchrow();
	void freerow(char*** row);
	void freeresult();

	void escape(char* value);
	void unescape(char* value);
};

#endif
