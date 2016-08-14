#include "cMySQL.h"

#include <stdio.h>

static struct {
	int loaded;
	void *handle;

	my_bool __stdcall (*mysql_thread_init)(void);
	void __stdcall (*mysql_thread_end)(void);
	MYSQL * __stdcall (*mysql_init)(MYSQL *mysql);
	void __stdcall (*mysql_close)(MYSQL *sock);
	MYSQL * __stdcall (*mysql_real_connect)(MYSQL *mysql,
		const char *host, const char *user, const char *passwd,
		const char *db, unsigned int port, const char *unix_socket,
		unsigned long clientflag);
	int __stdcall (*mysql_query)(MYSQL *mysql, const char *q);
	MYSQL_RES * __stdcall (*mysql_store_result)(MYSQL *mysql);
	void __stdcall (*mysql_free_result)(MYSQL_RES *result);

/*
my_ulonglong __stdcall mysql_num_rows(MYSQL_RES *res);
unsigned int __stdcall mysql_num_fields(MYSQL_RES *res);
my_bool __stdcall mysql_eof(MYSQL_RES *res);
MYSQL_FIELD *__stdcall mysql_fetch_field_direct(MYSQL_RES *res,
					      unsigned int fieldnr);
MYSQL_FIELD * __stdcall mysql_fetch_fields(MYSQL_RES *res);
MYSQL_ROW_OFFSET __stdcall mysql_row_tell(MYSQL_RES *res);
MYSQL_FIELD_OFFSET __stdcall mysql_field_tell(MYSQL_RES *res);

unsigned int __stdcall mysql_field_count(MYSQL *mysql);
my_ulonglong __stdcall mysql_affected_rows(MYSQL *mysql);
my_ulonglong __stdcall mysql_insert_id(MYSQL *mysql);
unsigned int __stdcall mysql_errno(MYSQL *mysql);
const char * __stdcall mysql_error(MYSQL *mysql);
const char *__stdcall mysql_sqlstate(MYSQL *mysql);
unsigned int __stdcall mysql_warning_count(MYSQL *mysql);
const char * __stdcall mysql_info(MYSQL *mysql);
unsigned long __stdcall mysql_thread_id(MYSQL *mysql);
const char * __stdcall mysql_character_set_name(MYSQL *mysql);
int          __stdcall mysql_set_character_set(MYSQL *mysql, const char *csname);

my_bool		__stdcall mysql_ssl_set(MYSQL *mysql, const char *key,
				      const char *cert, const char *ca,
				      const char *capath, const char *cipher);
const char *    __stdcall mysql_get_ssl_cipher(MYSQL *mysql);
my_bool		__stdcall mysql_change_user(MYSQL *mysql, const char *user,
					  const char *passwd, const char *db);
MYSQL *		__stdcall mysql_real_connect(MYSQL *mysql, const char *host,
					   const char *user,
					   const char *passwd,
					   const char *db,
					   unsigned int port,
					   const char *unix_socket,
					   unsigned long clientflag);
int		__stdcall mysql_select_db(MYSQL *mysql, const char *db);
int		__stdcall mysql_send_query(MYSQL *mysql, const char *q,
					 unsigned long length);
int		__stdcall mysql_real_query(MYSQL *mysql, const char *q,
					unsigned long length);
MYSQL_RES *     __stdcall mysql_store_result(MYSQL *mysql);
MYSQL_RES *     __stdcall mysql_use_result(MYSQL *mysql);
my_bool		__stdcall mysql_master_query(MYSQL *mysql, const char *q,
					   unsigned long length);
my_bool		__stdcall mysql_master_send_query(MYSQL *mysql, const char *q,
						unsigned long length);
my_bool		__stdcall mysql_slave_query(MYSQL *mysql, const char *q,
					  unsigned long length);
my_bool		__stdcall mysql_slave_send_query(MYSQL *mysql, const char *q,
						   unsigned long length);
void        __stdcall mysql_get_character_set_info(MYSQL *mysql,
						   MY_CHARSET_INFO *charset);
void		__stdcall mysql_free_result(MYSQL_RES *result);
void		__stdcall mysql_data_seek(MYSQL_RES *result,
					my_ulonglong offset);
MYSQL_ROW_OFFSET __stdcall mysql_row_seek(MYSQL_RES *result,
						MYSQL_ROW_OFFSET offset);
MYSQL_FIELD_OFFSET __stdcall mysql_field_seek(MYSQL_RES *result,
					   MYSQL_FIELD_OFFSET offset);
MYSQL_ROW	__stdcall mysql_fetch_row(MYSQL_RES *result);
unsigned long * __stdcall mysql_fetch_lengths(MYSQL_RES *result);
MYSQL_FIELD *	__stdcall mysql_fetch_field(MYSQL_RES *result);
MYSQL_RES *     __stdcall mysql_list_fields(MYSQL *mysql, const char *table,
					  const char *wild);
unsigned long	__stdcall mysql_escape_string(char *to,const char *from,
						unsigned long from_length);
unsigned long	__stdcall mysql_hex_string(char *to,const char *from,
										 unsigned long from_length);
unsigned long __stdcall mysql_real_escape_string(MYSQL *mysql,
						   char *to,const char *from,
						   unsigned long length);
int __stdcall mysql_next_result(MYSQL *mysql);
*/

} libMySQL;

int initMySQL() {
	if ( libMySQL.loaded == 0 ) {
		libMySQL.handle = (void *)LoadLibrary("libmysql.dll");
		if ( libMySQL.handle == NULL ) {
			return -1;
		}
		bool funcFailed = false;
		do {
			libMySQL.mysql_thread_init =
				(my_bool __stdcall (*) (void))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_thread_init");
			if ( libMySQL.mysql_thread_init == NULL ) {
				funcFailed = true; break;
			}
			libMySQL.mysql_thread_end =
				(void __stdcall (*) (void))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_thread_end");
			if ( libMySQL.mysql_thread_end == NULL ) {
				funcFailed = true; break;
			}
			libMySQL.mysql_init =
				(MYSQL * __stdcall (*) (MYSQL*))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_init");
			if ( libMySQL.mysql_init == NULL ) {
				funcFailed = true; break;
			}
			libMySQL.mysql_close =
				(void __stdcall (*) (MYSQL*))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_close");
			if ( libMySQL.mysql_close == NULL ) {
				funcFailed = true; break;
			}
			libMySQL.mysql_real_connect =
				(MYSQL * __stdcall (*) (MYSQL*, const char*, const char*, const char*, const char*, unsigned int, const char*, unsigned long))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_real_connect");
			if ( libMySQL.mysql_real_connect == NULL ) {
				funcFailed = true; break;
			}
			libMySQL.mysql_query =
				(int __stdcall (*) (MYSQL*, const char*))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_query");
			if ( libMySQL.mysql_query == NULL ) {
				funcFailed = true; break;
			}
			libMySQL.mysql_store_result =
				(MYSQL_RES * __stdcall (*) (MYSQL*))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_store_result");
			if ( libMySQL.mysql_store_result == NULL ) {
				funcFailed = true; break;
			}
			libMySQL.mysql_free_result =
				(void __stdcall (*) (MYSQL_RES*))
				GetProcAddress((HMODULE)libMySQL.handle, "mysql_free_result");
			if ( libMySQL.mysql_free_result == NULL ) {
				funcFailed = true; break;
			}

		} while (false);
		if (funcFailed) {
			FreeLibrary((HMODULE)libMySQL.handle);
			return -1;
		}
	}
	++libMySQL.loaded;
	return 0;
}

void quitMySQL() {
	if ( libMySQL.loaded == 0 ) {
		return;
	}
	if ( libMySQL.loaded == 1 ) {
		FreeLibrary((HMODULE)libMySQL.handle);
	}
	--libMySQL.loaded;
}

cMySQL::cMySQL() {
	initMySQL();
	mysql = NULL;
	libMySQL.mysql_thread_init();
	mysql = libMySQL.mysql_init(mysql);
	msres = NULL;
}

void cMySQL::freeresult() {
	if (msres) {
		libMySQL.mysql_free_result(msres);
		rows = 0; row = NULL; msres = NULL;
	}
}

cMySQL::~cMySQL() {
	if (mysql) {
		freeresult();
		libMySQL.mysql_close(mysql);
		libMySQL.mysql_thread_end();
	}
	quitMySQL();
}

bool cMySQL::connect(const char* server, const char* user, const char* password, const char* database, const char* codepage) {
	if (mysql) {
		mysql = libMySQL.mysql_real_connect(mysql,server,user,password,database, 0, NULL, 0);
		if (mysql) {
			char* str = (char*)malloc(128);
			if (str) {
				sprintf(str,"/*!40101 SET NAMES '%s' */",codepage);
				query(str);
				sprintf(str,"set character_set_client='%s';",codepage);
				query(str);
				sprintf(str,"set character_set_results='%s';",codepage);
				query(str);
				sprintf(str,"set collation_connection='%s_bin';",codepage);
				query(str);
				free(str);
			}
		}
	}
}

bool cMySQL::query(char* query) {
	if (mysql) {
		freeresult();
		return (libMySQL.mysql_query(mysql,query) == 0);
	}
	return false;
}

bool cMySQL::select(char* query) {
	if (mysql) {
		freeresult();
		int res = libMySQL.mysql_query(mysql,query);
		if (res != 0)
			return false;
		msres = libMySQL.mysql_store_result(mysql);
		if (msres == NULL)
			return false;
		rows = msres->row_count;
		cols = msres->field_count;
		row = msres->data_cursor;
		if (rows > 0)
			return true;
		freeresult();
	}
	return false;
}

int cMySQL::fetchcols() {
	if (msres) {
		return cols;
	}
	return 0;
}

char*** cMySQL::fetchrow() {
	if (msres) {
		if (rows > 0) {
			char*** res = (char***)malloc(sizeof(char**)*2);
			if (res) {
				memset(res,0,sizeof(char**)*2);
				for (int i = 0; i < 2; i++) {
					res[i] = (char**)malloc(sizeof(char*)*(cols+1));
					if (res[i]) {
						memset(res[i],0,sizeof(char*)*(cols+1));
					} else {
						freerow(res);
						return NULL;
					}
				}
			} else {
				return NULL;
			}
			for (int i = 0; i < cols; i++) {
				char* colname = (char*)(msres->fields[i].name);
				char* val = (char*)(row->data[i]);
				int len = strlen(colname);
				char* buf = (char*)malloc(len+1);
				if (buf) {
					memcpy(buf,colname,len+1);
					res[0][i] = buf;
				}
				len = strlen(val);
				buf = (char*)malloc(len+1);
				if (buf) {
					memcpy(buf,val,len+1);
					res[1][i] = buf;
				}
			}
			row = row->next; rows--;
			return res;
		}
	}
	return NULL;
}

void cMySQL::freerow(char*** row) {
	if (row) {
		for (int i = 0; i < 2; i++) {
			if (row[i]) {
				int j = 0;
				while (row[i][j]) {
					free(row[i][j]);
					j++;
				}
				free(row[i]);
			}
		}
		free(row);
	}
}

//const char MySQL_CP_UTF_8[] = "utf8";
