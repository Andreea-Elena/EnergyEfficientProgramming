#ifndef _HTTP_HEADER_H
#define _HTTP_HEADER_H

#include <stdbool.h>

#include "ht.h"

#define MAX_HEADER_SIZE 8192
#define HEADER_BODY_DELIMITER "\r\n\r\n"
#define MAX_HEADER_VERB_LENGTH 16
#define MAX_HEADER_HTTP_VERSION_LENGTH 16
#define MAX_HEADER_URL_LENGTH 2000
#define MAX_HEADER_URL_BASE_TOKENS 8
#define MAX_HEADER_URL_BASE_LEN 512
#define MAX_HEADER_URL_PARAMETERS 32
#define MAX_HEADER_HEADERS 32
#define MAX_HEADER_HEADERS_TAG_LENGTH 64
#define MAX_HEADER_HEADERS_TAG_VALUE 1024
#define MIN_LUA_FILE_NAME_LENGTH 3
#define MAX_LUA_FILE_NAME_LENGTH 64
#define DEFAULT_LUA_FILE_NAME "default"
#define DEFAULT_LUA_VERB_PREFIX "__"
#define DEFAULT_LUA_VERB_DELIMITER "_"

#define HEADER_OK 0
#define HEADER_ERROR_SIZE_TOO_LARGE -100
#define HEADER_ERROR_MISSING_DELIMITER -101
#define HEADER_ERROR_MAX_HEADERS -102
#define HEADER_ERROR_TOKENIZER -103
#define HEADER_ERROR_LINE_TOKENIZER -104
#define HEADER_ERROR_VERB_LENGTH -105
#define HEADER_ERROR_VERB_NOT_ALLOWED -106
#define HEADER_ERROR_PROTOCOL_VERSION_LENGTH -107
#define HEADER_ERROR_VERSION_ERROR -108
#define HEADER_ERROR_WRONG_PROTOCOL -109
#define HEADER_ERROR_VERSION_NOT_ALLOWED -110
#define HEADER_ERROR_URL_MISSING -111
#define HEADER_ERROR_URL_TOO_LONG -112
#define HEADER_ERROR_URL_DEPTH_TO_DEEP -113
#define HEADER_ERROR_PARAMS_START_ERROR -114
#define HEADER_ERROR_PARAMS_TOO_MANY -115
#define HEADER_ERROR_PARAMS_BAD_PARAM -116
#define HEADER_ERROR_HEADRES_BAD_HEADER -117
#define HEADER_ERROR_HEADRES_BAD_CONTENT_LENGTH -118
#define HEADER_ERROR_HEADRES_BAD_LUA_ENDPOINT_LENGTH -119

#define HTTP_PROTOCOL "HTTP"
#define HTTP_MIN_VERSION 1.0
#define HTTP_MAX_VERSION 1.1

// ht* valid_verb_map;
// ht* valid_http_version_map;

// __[VERB]_[URL_PATH]_[URL_FILE].[extension]
// Parameters for the LUA script
struct HTTP_LUA_FILE_SIGNATURE {
  char *verb;  // VERB of HEADER
  char *path;  // URL BASE without LAST TAG
  char *file;  // LAST TAG of URL BASE
  char *body;
  int content_length;

  int parameters_len;    // PARAMETERS length
  char **parameters;     // PARAMETER TAGS word by word in order
  ht *paramters_values;  // PARAMETERS Tags and Values
};

typedef struct HTTP_LUA_FILE_SIGNATURE lua_file_signature;

struct HTTP_HEADER {
  char *header;  // The HEADER

  char verb[MAX_HEADER_VERB_LENGTH];  // Single word VERB of the HEADER
  char url[MAX_HEADER_URL_LENGTH];    // The whole URL found in the HEADER
  char *url_path;                     // The WHOLE PATH found in the URL
  char *url_base;                     // The URL in PATH  e.g URL BASE = "/test/one/two/three"
  char *url_params;                   // The PATAMTERS in PATH e.g URL BASE = "?asta=unu&una=true&me=arix"
  int url_base_tokens;                // Depth of the URL BASE, includes ROOT /
  char **url_base_ordered;            // URL PATH word by word in order e.g URL BASE = "/test/one/two/three" => [/],[test],[one],[two],[three]
  int url_params_tokens;              // Number of params
  char **url_params_ordered;          // URL PARAMETER TAGS word by word in order
  ht *url_params_values;              // URL PARAMETERS Tags and Values
  char *url_body;

  char *http_protocol_version;  // HTTP Protocol/Version
  char *http_protocol;          // HTTP Protocol
  char *http_version;           // HTTP Version Only

  char **header_headers;      // HEADER headers word by word in order
  ht *header_headers_values;  // HEADER headers Tags and Values

  int header_length;   // HEADER Length
  int content_length;  // HEADER content length value as int, if present, otherwise -1
  char *_epnt;         // Custom endpoint name - _epnt.lua to be executed
};

typedef struct HTTP_HEADER http_header;

// Initializes and fills the structure, returns ERROR CODES as defined, HEADER_OK (0) if all good
int init_header(char *header, http_header *hh);

// Checks if a specific header is present
bool is_header_present(http_header *hh, char *header_key);

// Returns the value of the requests header if exists, NULL if it doesn't
char *get_header(http_header *hh, char *header_key);

// Checks if a specific parameter is present
bool is_url_parameter_present(http_header *hh, char *parameter_key);

// Returns the value of the requests parameter if exists, NULL if it doesn't
char *get_parameter(http_header *hh, char *parameter_key);

// Returns the path and lua file script that should be called according to this header
lua_file_signature *get_lua_file_signature(http_header *hh, char *extension);

#endif