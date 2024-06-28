#include "mysql_utils.h"

#include <assert.h>
#include <cJSON.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

void print_mysql_version() {
  unsigned long mversion = mysql_get_client_version();
  printf("MYSQL %lu\n", mversion);
};

MYSQL* init_mysql_connection() {
  MYSQL* conn;
  if ((conn = mysql_init(NULL)) == NULL) {
    printf("Could not init DB\n");
    return NULL;
  } else
    return conn;
};

void connect_db(MYSQL* conn, char* url, char* username, char* password, char* db, unsigned int port, const char* unix_socket,
                unsigned long client_flag) {
  if (mysql_real_connect(conn, url, username, password, db, port, unix_socket, client_flag) == NULL) {
    fprintf(stderr, "DB Connection Error\n");
  }
};

void finish_with_error(MYSQL* con, lua_State* L) {
  const char* error_message = mysql_error(con);
  size_t message_length = strlen(error_message);

  // Allocate memory for the error message, including space for the prefix and null terminator
  char* message = (char*)malloc(18 + message_length + 1);  // +1 for the null terminator
  if (message == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
  }

  // Format the error message
  sprintf(message, "MYSQL EXCEPTION: %s", error_message);

  // Print the error message
  fprintf(stderr, "%s\n", message);

  // Push the error message to Lua
  lua_pushstring(L, message);
  // Free the allocated memory for the message
  free(message);
}

void execute_query(lua_State* L, const char* query, const char* success_message) {
  MYSQL* con = mysql_init(NULL);
  if (con == NULL) {
    fprintf(stderr, "mysql_init() failed\n");
    finish_with_error(con, L);
  }

  if (mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0) == NULL) {
    finish_with_error(con, L);
  }

  if (mysql_query(con, query)) {
    finish_with_error(con, L);
  }

  if (mysql_errno(con) == 0) {
    printf("%s\n", success_message);
    lua_pushstring(L, success_message);
  }

  mysql_close(con);
}

int query_select_C(lua_State* L) {
  char* query = lua_tostring(L, 1);  // Get the first parameter

  MYSQL* con = mysql_init(NULL);
  if (con == NULL) {
    fprintf(stderr, "mysql_init() failed\n");
    finish_with_error(con, L);
  }

  if (mysql_real_connect(con, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0) == NULL) {
    finish_with_error(con, L);
  }

  if (mysql_query(con, query)) {
    finish_with_error(con, L);
  }

  MYSQL_RES* result = mysql_store_result(con);
  if (result == NULL) {
    finish_with_error(con, L);
  }

  int num_fields = mysql_num_fields(result);
  cJSON* jsonArray = cJSON_CreateArray();

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) {
    cJSON* jsonObject = cJSON_CreateObject();
    for (int i = 0; i < num_fields; i++) {
      cJSON_AddStringToObject(jsonObject, mysql_fetch_field_direct(result, i)->name, row[i] ? row[i] : "");
    }
    cJSON_AddItemToArray(jsonArray, jsonObject);
  }

  mysql_free_result(result);
  mysql_close(con);

  char* resultJson = cJSON_Print(jsonArray);
  printf("%s\n", resultJson);
  cJSON_Delete(jsonArray);
  lua_pushstring(L, resultJson);

  return 1;  // Number of return values
}

int query_insert_C(lua_State* L) {
  const char* query = lua_tostring(L, 1);  // Get the first parameter as SQL query
  const char* success_message = "Insertion successful!";
  execute_query(L, query, success_message);
  return 1;  // Number of return values
}

int query_delete_C(lua_State* L) {
  const char* query = lua_tostring(L, 1);  // Get the first parameter as SQL query
  const char* success_message = "Deletion successful!";
  execute_query(L, query, success_message);
  return 1;  // Number of return values
}

int query_update_C(lua_State* L) {
  const char* query = lua_tostring(L, 1);  // Get the first parameter as SQL query
  const char* success_message = "Update successful!";
  execute_query(L, query, success_message);
  return 1;  // Number of return values
}
void close_db(MYSQL* conn) { mysql_close(conn); }