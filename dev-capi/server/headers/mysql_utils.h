#include <mysql/mysql.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#define DB_HOST LOCAL_HOST
#define DB_USER "admin"
#define DB_PASS "123456"
#define DB_NAME "capi"
#define DB_PORT 3306

void print_mysql_version();
MYSQL* init_mysql_connection();
void connect_db(MYSQL* conn, char* url, char* username, char* password, char* db, unsigned int port, const char* unix_socket,
                unsigned long client_flag);
void execute_query(lua_State* L, const char* query, const char* success_message);
int query_insert_C(lua_State* L);
int query_select_C(lua_State* L);
int query_update_C(lua_State* L);
int query_delete_C(lua_State* L);
void close_db(MYSQL* conn);
