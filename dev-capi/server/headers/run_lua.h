#include "ht.h"
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
#include "str_utils.h"

// find and execute the lua script and returns the result
const char* execute_script(char* script, ht* parameters_values, int parameters_len, char* path, char* body, int content_length);
// prints the parameters all parameters and returns the result from execute_script function
char* capilua(lua_file_signature* lfs);
// prints in the terminal an error
void bail(lua_State* L, char* msg);
int custom_print(lua_State* L);
char* concatenate_key_value(const char* key, const char* value);
char* format_response_C(lua_State* L);
