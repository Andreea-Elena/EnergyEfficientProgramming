#include "../headers/run_lua.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <mysql/client_plugin.h>

#include "../headers/mysql_utils.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#define MAX_WRITE_SIZE \
  16384  // We limit the sending buffer to make sure we send all data in one go - also
         // depends on the client

void bail(lua_State *L, char *msg) { fprintf(stderr, "\nFATAL ERROR:\n  %s: %s\n\n", msg, lua_tostring(L, -1)); }

const char *execute_script(char *script, ht *parameters_values, int parameters_len, char *path, char *body, int content_length) {
  int bodyExists = 0;
  char *result = "FAILED";  // Allocate memory and copy "FAILED" into it
  char *combinedPath = malloc(strlen(script) + strlen(path) + 2);
  sprintf(combinedPath, "%s/%s", path, script);
  combinedPath[strlen(script) + strlen(path) + 1] = '\0';
  lua_State *L;

  L = luaL_newstate(); /* Create Lua state variable */
  luaL_openlibs(L);    /* Load Lua libraries */

  if (luaL_loadfile(L, combinedPath + 1)) /* Load but don't run the Lua script */
    bail(L, "luaL_loadfile() failed");    /* Error out if file can't be read */

  if (lua_pcall(L, 0, 0, 0))       /* PRIMING RUN. FORGET THIS AND YOU'RE TOAST */
    bail(L, "lua_pcall() failed"); /* Error out if Lua file has an error */

  lua_getglobal(L, "test"); /* Tell it to run script.lua->test() */
  if (content_length != -1) {
    if (body != NULL) {
      lua_pushstring(L, body);
      bodyExists = 1;
    }
  }
  if (parameters_len > 0) {
    hti it = ht_iterator(parameters_values);
    while (ht_next(&it)) {
      char *concatenated = concatenate_key_value(it.key, it.value);
      lua_pushstring(L, concatenated);
      free(concatenated);
    }
  }
  // Redirect Lua's print function to custom_print
  lua_register(L, "print", custom_print);
  lua_register(L, "query_insert_Lua", query_insert_C);
  lua_register(L, "query_select_Lua", query_select_C);
  lua_register(L, "query_update_Lua", query_update_C);
  lua_register(L, "query_delete_Lua", query_delete_C);
  lua_register(L, "format_response_Lua", format_response_C);
  // lua_pushnumber(L, 6);       /* Submit 6 as the argument to square() */
  if (lua_pcall(L, parameters_len + bodyExists, 1, 0)) { /* Call the function with parameters_len argument, returning 1 result */
    bail(L, "lua_pcall() failed");
  } else {
    if (lua_isstring(L, -1)) {
      result = strdup(lua_tostring(L, -1));
      printf("Returned result: %s\n", result);
    } else {
      bail(L, "Lua function did not return a string");
    }
  }
  // int mynumber = lua_tonumber(L, -1);
  // printf("%sReturned number=%d\n", BLUE, mynumber); // Get the result
  lua_close(L); /* Clean up, free the Lua state var */
  free(combinedPath);
  return result;
}

// Function to format the response
char *format_response_C(lua_State *L) {
  int *status_code = lua_tointeger(L, 1);       // Get the first parameter
  char *headers = lua_tostring(L, 2);           // Get the second parameter
  char *message = lua_tostring(L, 3);           // Get the third parameter
  static char response_buffer[MAX_WRITE_SIZE];  // Static buffer
  // Format the response
  sprintf(response_buffer, "HTTP/1.1 %d\r\n%s\r\n\r\n%s", status_code, headers, message);
  lua_pushstring(L, response_buffer);
  return 1;
}

char *capilua(lua_file_signature *lfs) {
  printf("\n\n$$$$$$$$$$$$$$ CAPILUA $$$$$$$$$$$$$$$$\n\n");
  printf("\nLFS verb :: [%s]\n", lfs->verb);
  printf("LFS path :: [%s]\n", lfs->path);
  printf("LFS file :: [%s]\n", lfs->file);
  printf("LFS parameters_len :: [%d]\n", lfs->parameters_len);
  printf("LFS content_length :: [%d]\n", lfs->content_length);
  if (lfs->content_length != -1) {
    printf("LFS body :: [%s]\n", lfs->body);
  }
  if (lfs->parameters_len > 0) {
    printf("LFS parameters_values ::\n");
    hti it = ht_iterator(lfs->paramters_values);
    while (ht_next(&it)) {
      printf("%s %s\n", it.key, (char *)it.value);
    }
  }
  const char *response = strdup("FAILED");  // Allocate memory and copy "FAILED" into it
  response = execute_script(lfs->file, lfs->paramters_values, lfs->parameters_len, lfs->path, lfs->body, lfs->content_length);
  printf("\n\n$$$$$$$$$$$$$$ EXIT CAPILUA $$$$$$$$$$$$$$$$\n\n");
  return response;
}

// Custom Lua print function that redirects to C function
int custom_print(lua_State *L) {
  const char *message = lua_tostring(L, 1);
  printf("Lua says: %s\n", message);
  return 0;
}

// Function to concatenate key and value with a comma
char *concatenate_key_value(const char *key, const char *value) {
  size_t total_length = strlen(key) + strlen(value) + 4;  // +4 for comma, null terminator and ""
  char *result = (char *)malloc(total_length);
  if (result == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    return;
  }
  snprintf(result, total_length, "%s=%s", key, value);
  return result;
}