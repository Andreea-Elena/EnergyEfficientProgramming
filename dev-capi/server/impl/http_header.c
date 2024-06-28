#include "../headers/http_header.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../headers/str_utils.h"

static char *valid_http_verbs[5] = {"GET", "POST", "PUT", "PATCH", "DELETE"};
static char *valid_http_versions[2] = {"1.0", "1.1"};
ht *valid_verb_map;
ht *valid_http_version_map;
void init_header_verb_list() {
  valid_verb_map = ht_create(16);
  for (int i = 0; i < 5; i++) {
    ht_set(valid_verb_map, valid_http_verbs[i], "true");
  }
}

void init_header_version_list() {
  valid_http_version_map = ht_create(4);
  for (int i = 0; i < 2; i++) {
    ht_set(valid_http_version_map, valid_http_versions[i], "true");
  }
}

int check_valid_header_verb(char *verb) {
  if (ht_get(valid_verb_map, verb) != NULL)
    return 0;
  else
    return -1;
}

int check_valid_header_version(char *version) {
  if (ht_get(valid_http_version_map, version) != NULL)
    return 0;
  else
    return -1;
}

// Initializes and fills the structure, returns ERROR CODES as defined, HEADER_OK (0) if all good
int init_header(char *header, http_header *hh) {
  printf("(init_header)->started...\n");
  printf("(init_header)->START HEADER SAMPLE\n---\n%s\n---\n(init_header)->END HEADER SAMPLE\n", header);

  // Check total header size to not exceed max value set (MAX_HEADER_SIZE)
  printf("(init_header)->Step 1.0.0 :: Check HEADER SIZE\n");
  int header_length = strlen(header);
  if (header_length >= MAX_HEADER_SIZE) {
    printf("(init_header)->Step 1.0.1 :: HEADER_ERROR_SIZE_TOO_LARGE\n");
    return HEADER_ERROR_SIZE_TOO_LARGE;
  }
  printf("(init_header)->Step 1.1.0 :: HEADER Size is OK [%d]\n", header_length);
  hh->header = malloc(sizeof(char) * header_length + 1);
  memcpy(hh->header, header, header_length + 1);
  hh->header_length = header_length;

  // Copy the header to another string to work on it with strtok and other functions that 'destroy' the string
  printf("(init_header)->Step 1.2.0 :: Make copy of HEADER\n");
  char *copyHeader = (char *)malloc(header_length + 1);
  memcpy(copyHeader, header, header_length + 1);

  // Check if the HTTP delimiter Headers / Body exists [\r\n\r\n]
  printf("(init_header)->Step 1.3.0 :: Check if HEADER cotains HEADER/BODY delimiter\n");
  char *p = strstr(copyHeader, HEADER_BODY_DELIMITER);
  if (p == NULL) {
    printf("(init_header)->Step 1.3.2 :: HEADER_ERROR_MISSING_DELIMITER\n");
    free(copyHeader);
    return HEADER_ERROR_MISSING_DELIMITER;
  }
  printf("(init_header)->Step 1.3.3 :: HEADER delmiter present\n");

  // Tokenize the HEADER and get the LINES
  printf("(init_header)->Step 1.4.0 :: Header Tokenizer started...\n");
  char **header_lines = malloc(sizeof(char *) * MAX_HEADER_HEADERS);
  int tokens_lines = tokenizer(copyHeader, "\r\n", header_lines);
  if (tokens_lines > MAX_HEADER_HEADERS) {
    printf("(init_header)->Step 1.4.1 :: HEADER_ERROR_MAX_HEADERS\n");
    free(header_lines);
    free(copyHeader);
    return HEADER_ERROR_MAX_HEADERS;
  }
  if (tokens_lines < 0 || header_lines == NULL) {
    printf("(init_header)->Step 1.4.2 :: HEADER_ERROR_TOKENIZER\n");
    free(header_lines);
    free(copyHeader);
    return HEADER_ERROR_TOKENIZER;
  }
  for (int i = 0; i <= tokens_lines; i++) {
    printf("(init_header)->Step 1.4.0 :: TOKEN [%d][%s]\n", i, header_lines[i]);
  }
  printf("(init_header)->Step 1.4.0 :: Header Tokenizer ended...[%d]\n", tokens_lines);

  // Check first line of the HEADER and get the information
  printf("(init_header)->Step 1.5.0 :: Checking first line of the header\n");
  char *fl = header_lines[0];
  char **header_fl = malloc(sizeof(char *) * 3);
  int header_fl_tokens = tokenizer(fl, " ", header_fl);
  if (header_fl_tokens < 0 || header_fl == NULL || header_fl_tokens != 2) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    printf("(init_header)->Step 1.5.1 :: HEADER_ERROR_LINE_TOKENIZER\n");
    return HEADER_ERROR_LINE_TOKENIZER;
  }
  for (int i = 0; i <= header_fl_tokens; i++) {
    printf("(init_header)->Step 1.5.0 :: FL TOKEN [%d][%s]\n", i, header_fl[i]);
  }

  // Get&Check VERB of HEADER
  printf("(init_header)->Step 1.6.0 :: Checking VERB of HEADER\n");
  char *verb = header_fl[0];
  int verb_length = strlen(verb);
  if (verb_length > MAX_HEADER_VERB_LENGTH) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    printf("(init_header)->Step 1.6.1 :: HEADER_ERROR_VERB_LENGTH\n");
    return HEADER_ERROR_VERB_LENGTH;
  }
  init_header_verb_list();
  int valid_verb = check_valid_header_verb(verb);
  if (valid_verb != 0) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    printf("(init_header)->Step 1.6.2 :: HEADER_ERROR_VERB_NOT_ALLOWED [%s]\n", verb);
    return HEADER_ERROR_VERB_NOT_ALLOWED;
  } else {
    if (strcmp(verb, "PUT") == 0 || strcmp(verb, "POST") == 0) {
      hh->url_body = malloc(sizeof(char *) * strlen(p + 4));
      memcpy(hh->url_body, p + 4, strlen(p + 4));
      hh->url_body[strlen(p + 4)] = '\0';
      printf("(init_header)->Step 1.3.1 :: BODY -> [%s]\n", hh->url_body);
    } else {
      hh->url_body = NULL;
    }
  }
  memcpy(hh->verb, verb, verb_length + 1);
  printf("(init_header)->Step 1.6.0 :: Checking VERB of HEADER OK [%s]\n", verb);

  // Get&Check PROTOCOL/VERSION of HEADER
  printf("(init_header)->Step 1.7.0 :: VERSION TOKEN");
  char *version = header_fl[2];
  int http_p_v_len = strlen(version);
  if (http_p_v_len > MAX_HEADER_HTTP_VERSION_LENGTH) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    printf("(init_header)->Step 1.7.1 :: HEADER_ERROR_PROTOCOL_VERSION_LENGTH [%d]\n", http_p_v_len);
    return HEADER_ERROR_PROTOCOL_VERSION_LENGTH;
  }
  char **version_tk = malloc(sizeof(char *) * 2);
  int version_tokens = tokenizer(version, "/", version_tk);
  if (version_tokens < 0 || version_tokens > 1 || version_tk == NULL) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    printf("(init_header)->Step 1.7.2 :: HEADER_ERROR_VERSION_ERROR\n");
    return HEADER_ERROR_VERSION_ERROR;
  }
  printf("(init_header)->Step 1.7.0 :: PROTOCOL VERSION [%s][%d]\n", version, http_p_v_len);
  for (int i = 0; i <= version_tokens; i++) {
    printf("(init_header)->Step 1.7.0 :: VERSION TOKEN [%d][%s]\n", i, version_tk[i]);
  }
  char *protocol = version_tk[0];
  char *pversion = version_tk[1];
  printf("(init_header)->Step 1.7.0 :: VERSION TOKEN END VALUES [%s][%s]\n", protocol, pversion);
  if (strcmp(protocol, HTTP_PROTOCOL) != 0) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    printf("(init_header)->Step 1.7.3 :: HEADER_ERROR_WRONG_PROTOCOL\n");
    return HEADER_ERROR_WRONG_PROTOCOL;
  }
  init_header_version_list();
  int valid_version = check_valid_header_version(pversion);
  if (valid_version != 0) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    printf("(init_header)->Step 1.7.4 :: HEADER_ERROR_VERSION_NOT_ALLOWED [%s]\n", verb);
    return HEADER_ERROR_VERSION_NOT_ALLOWED;
  }
  hh->http_protocol = malloc(sizeof(char) * strlen(protocol) + 1);
  hh->http_version = malloc(sizeof(char) * strlen(pversion) + 1);
  hh->http_protocol_version = malloc(sizeof(char) * http_p_v_len + 1);
  memcpy(hh->http_protocol_version, version, http_p_v_len + 1);
  memcpy(hh->http_protocol, protocol, strlen(protocol) + 1);
  memcpy(hh->http_version, pversion, strlen(pversion) + 1);
  printf("(init_header)->Step 1.7.0 :: Checking PROTOCOL and VERSION of HEADER OK [%s][%s]\n", protocol, pversion);

  // Get&Check PARAMETERS of HEADER
  printf("(init_header)->Step 1.8.0 :: PARAMETERS");
  char *url_path = header_fl[1];
  if (url_path == NULL) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    printf("(init_header)->Step 1.8.1 :: HEADER_ERROR_URL_MISSING\n");
    return HEADER_ERROR_URL_MISSING;
  }
  int url_path_len = strlen(url_path);
  if (url_path_len > MAX_HEADER_URL_LENGTH) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    printf("(init_header)->Step 1.8.2 :: HEADER_ERROR_URL_TOO_LONG\n");
    return HEADER_ERROR_URL_TOO_LONG;
  }
  printf("(init_header)->Step 1.8.0 :: PARAMETERS \n---\n[%s]\n---\n", url_path);
  char *url_params = strstr(url_path, "?");
  int url_params_len = 0;
  if (url_params != NULL) {
    url_params_len = strlen(url_params);
  }
  int url_base_len = (url_params != NULL) ? (url_params - url_path) : strlen(url_path);
  char *url_base = malloc(sizeof(char) * (url_base_len) + 1);
  memcpy(url_base, url_path, url_base_len);
  url_base[url_base_len] = '\0';
  printf("(init_header)->Step 1.8.0 :: URL BASE [%s][%d]\n", url_base, url_base_len);
  printf("(init_header)->Step 1.8.0 :: URL PARAMS [%s][%d]\n", url_params, url_params_len);
  hh->url_path = malloc(sizeof(char) * url_path_len + 1);
  hh->url_base = malloc(sizeof(char) * url_base_len + 1);
  hh->url_params = malloc(sizeof(char) * url_params_len + 1);
  memcpy(hh->url_path, url_path, url_path_len + 1);
  memcpy(hh->url_base, url_base, url_base_len + 1);
  if (url_params_len != 0) {
    memcpy(hh->url_params, url_params, url_params_len + 1);
  }
  printf("(init_header)->Step 1.8.0 :: PARAMETERS [%s][%s]\n", url_base, url_params);

  // Get&Check URL BASE PATH of HEADER
  printf("(init_header)->Step 1.9.0 :: URL BASE");
  if (strcmp(url_base, "/") == 0) {
    hh->url_base_tokens = 1;
    hh->url_base_ordered = malloc(sizeof(char *) * 1);
    hh->url_base_ordered[0] = malloc(sizeof(char) * 2);
    memcpy(hh->url_base_ordered[0], url_base, 1);
    hh->url_base_ordered[1] = '\0';
  } else {
    char **url_base_tokens = malloc(sizeof(char *) * MAX_HEADER_URL_BASE_TOKENS);
    int url_base_tokens_len = tokenizer(url_base, "/", url_base_tokens);
    if (url_base_tokens_len > MAX_HEADER_URL_BASE_TOKENS) {
      free(header_lines);
      free(copyHeader);
      free(header_fl);
      free(version_tk);
      free(url_base);
      printf("(init_header)->Step 1.9.1 :: HEADER_ERROR_URL_DEPTH_TO_DEEP [%d]\n", MAX_HEADER_URL_BASE_TOKENS);
      return HEADER_ERROR_URL_DEPTH_TO_DEEP;
    }
    url_base_tokens[0] = malloc(sizeof(char) * 2);
    memcpy(url_base_tokens[0], "/\0", 2);

    for (int i = 0; i <= url_base_tokens_len; i++) {
      printf("(init_header)->Step 1.9.1 :: [%d][%s][%lu]\n", i, url_base_tokens[i], strlen(url_base_tokens[i]));
    }

    hh->url_base_ordered = malloc(sizeof(char *) * (url_base_tokens_len + 1));
    for (int i = 0; i <= url_base_tokens_len; i++) {
      hh->url_base_ordered[i] = malloc(sizeof(char) * strlen(url_base_tokens[i] + 1));
      memcpy(hh->url_base_ordered[i], url_base_tokens[i], strlen(url_base_tokens[i]) + 1);
    }
    hh->url_base_tokens = url_base_tokens_len;
    printf("(init_header)->Step 1.9.0 :: URL BASE [%d] tokens \n", url_base_tokens_len + 1);

    // Get&Check HEADERS of HEADER
    // header_lines - whole http header line by line
    // tokens_lines - number of lines in the whole http header
    printf("(init_header)->Step 1.11.0 :: HEADERS [%d]\n", tokens_lines);
    // Ignore first line of the header
    hh->content_length = -1;
    hh->_epnt = NULL;
    char **headers = malloc(sizeof(char *) * tokens_lines);
    for (int i = 0; i < tokens_lines; i++) {
      headers[i] = malloc(sizeof(char) * (strlen(header_lines[i + 1])) + 1);
      memcpy(headers[i], header_lines[i + 1], strlen(header_lines[i + 1]) + 1);
    }
    for (int i = 0; i < tokens_lines; i++) {
      printf("(init_header)->Step 1.11.0 :: HEADERS [%d][%s]\n", i, headers[i]);
    }

    hh->header_headers = malloc(sizeof(char *) * tokens_lines + 1);
    hh->header_headers_values = ht_create(tokens_lines * 2);

    for (int i = 0; i < tokens_lines; i++) {
      int k = 0;
      int found = -1;
      int hlen = strlen(headers[i]);
      for (k = 0; k < hlen; k++) {
        if (headers[i][k] == ':') {
          found = 0;
          break;
        }
      }
      if (found < 0) {
        free(header_lines);
        free(copyHeader);
        free(header_fl);
        free(version_tk);
        free(url_base);
        free(headers);
        printf("(init_header)->Step 1.11.1 :: HEADER_ERROR_HEADRES_BAD_HEADER [%d]\n", HEADER_ERROR_HEADRES_BAD_HEADER);
        return HEADER_ERROR_HEADRES_BAD_HEADER;
      }
      int value_len = hlen - k - 1;
      int tag_len = hlen - value_len;
      if (value_len < 1) {
        free(header_lines);
        free(copyHeader);
        free(header_fl);
        free(version_tk);
        free(url_base);
        free(headers);
        printf("(init_header)->Step 1.11.2 :: HEADER_ERROR_HEADRES_BAD_HEADER [%d]\n", HEADER_ERROR_HEADRES_BAD_HEADER);
        return HEADER_ERROR_HEADRES_BAD_HEADER;
      }
      char *tag = malloc(sizeof(char) * tag_len + 1);
      char *value = malloc(sizeof(char) * value_len + 1);
      memcpy(tag, headers[i], k);
      tag[k] = '\0';
      memcpy(value, headers[i] + k + 1, value_len + 1);

      char *value_trim;
      int value_trim_len;
      if (value[0] == ' ') {
        value_trim_len = value_len - 1;
        value_trim = malloc(sizeof(char) * value_trim_len + 1);
        memcpy(value_trim, value + 1, value_trim_len + 1);
      } else {
        value_trim_len = value_len;
        value_trim = malloc(sizeof(char) * value_trim_len + 1);
        memcpy(value_trim, value, value_trim_len + 1);
      }
      printf("(init_header)->Step 1.11.0 :: [%d][%d] - [%d][%d][%d] - [%s] - [%s][%s]\n", i, k, hlen, tag_len, value_trim_len, headers[i], tag,
             value_trim);

      hh->header_headers[i] = malloc(sizeof(char) * tag_len + 1);
      memcpy(hh->header_headers[i], tag, tag_len + 1);
      ht_set(hh->header_headers_values, tag, value);

      // Content-Length value
      if (strcasecmp(tag, "Content-Length") == 0) {
        printf("(init_header)->Step 1.11.0 :: HEADERS found Content-Length value [%s]\n", value_trim);
        if (atoi(value_trim) > 0) {
          hh->content_length = atoi(value_trim);
        } else {
          free(header_lines);
          free(copyHeader);
          free(header_fl);
          free(version_tk);
          free(url_base);
          free(headers);
          printf("(init_header)->Step 1.11.3 :: HEADER_ERROR_HEADRES_BAD_CONTENT_LENGTH [%d]\n", HEADER_ERROR_HEADRES_BAD_CONTENT_LENGTH);
          return HEADER_ERROR_HEADRES_BAD_CONTENT_LENGTH;
        }
      }

      // Call file name
      if (strcasecmp(tag, "_epnt") == 0) {
        printf("(init_header)->Step 1.11.0 :: HEADERS found _epnt value value [%s]\n", value_trim);
        if (value_trim_len < MIN_LUA_FILE_NAME_LENGTH || value_trim_len > MAX_LUA_FILE_NAME_LENGTH) {
          free(header_lines);
          free(copyHeader);
          free(header_fl);
          free(version_tk);
          free(url_base);
          free(headers);
          printf("(init_header)->Step 1.11.4 :: HEADER_ERROR_HEADRES_BAD_LUA_ENDPOINT_LENGTH [%d]\n", HEADER_ERROR_HEADRES_BAD_LUA_ENDPOINT_LENGTH);
          return HEADER_ERROR_HEADRES_BAD_LUA_ENDPOINT_LENGTH;
        }
        hh->_epnt = malloc(sizeof(char) * value_trim_len + 1);
        memcpy(hh->_epnt, value_trim, value_trim_len + 1);
      }
    }
    printf("(init_header)->Step 1.11.0 :: HEADERS [%d] OK\n", tokens_lines);
  }

  // Get&Check PARAMETERS of HEADER
  printf("(init_header)->Step 1.10.0 :: PARAMETERS [%s][%d]\n", url_params, url_params_len);
  // No params found
  if (url_params == NULL) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    free(url_base);
    return 0;
  }

  if (url_params[0] != '?') {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    free(url_base);
    printf("(init_header)->Step 1.10.1 :: HEADER_ERROR_PARAMS_START_ERROR [%d][%c]\n", HEADER_ERROR_PARAMS_START_ERROR, url_params[0]);
    return HEADER_ERROR_PARAMS_START_ERROR;
  }
  char *urlp_all = malloc(sizeof(char) * (url_params_len));
  memcpy(urlp_all, url_params + 1, url_params_len);
  char **urlp_tokens = malloc(sizeof(char *) * MAX_HEADER_URL_PARAMETERS);
  int urlp_tokens_len = tokenizer(urlp_all, "&", urlp_tokens);
  if (urlp_tokens_len > MAX_HEADER_URL_PARAMETERS) {
    free(header_lines);
    free(copyHeader);
    free(header_fl);
    free(version_tk);
    free(url_base);
    free(urlp_all);
    printf("(init_header)->Step 1.10.1 :: HEADER_ERROR_PARAMS_TOO_MANY [%d][%d][%d]\n", HEADER_ERROR_PARAMS_TOO_MANY, MAX_HEADER_URL_PARAMETERS,
           urlp_tokens_len);
    return HEADER_ERROR_PARAMS_TOO_MANY;
  }
  for (int i = 0; i <= urlp_tokens_len; i++) {
    printf("(init_header)->Step 1.10.0 :: [%d][%s][%lu]\n", i, urlp_tokens[i], strlen(urlp_tokens[i]));
  }

  hh->url_params_tokens = urlp_tokens_len + 1;
  char **url_params_tags = malloc(sizeof(char *) * urlp_tokens_len + 1);
  hh->url_params_ordered = malloc(sizeof(char *) * urlp_tokens_len + 1);
  hh->url_params_values = ht_create(urlp_tokens_len * 2);

  for (int i = 0; i <= urlp_tokens_len; i++) {
    int k = 0;
    int found = -1;
    int tlen = strlen(urlp_tokens[i]);
    for (k = 0; k < tlen; k++) {
      if (urlp_tokens[i][k] == '=') {
        found = 0;
        break;
      }
    }
    if (found < 0) {
      free(header_lines);
      free(copyHeader);
      free(header_fl);
      free(version_tk);
      free(url_base);
      free(urlp_all);
      printf("(init_header)->Step 1.10.2 :: HEADER_ERROR_PARAMS_BAD_PARAM [%d]\n", HEADER_ERROR_PARAMS_BAD_PARAM);
      return HEADER_ERROR_PARAMS_BAD_PARAM;
    }
    int value_len = tlen - k - 1;
    int tag_len = tlen - value_len;
    if (value_len < 1) {
      free(header_lines);
      free(copyHeader);
      free(header_fl);
      free(version_tk);
      free(url_base);
      free(urlp_all);
      printf("(init_header)->Step 1.10.3 :: HEADER_ERROR_PARAMS_BAD_PARAM [%d]\n", HEADER_ERROR_PARAMS_BAD_PARAM);
      return HEADER_ERROR_PARAMS_BAD_PARAM;
    }
    char *tag = calloc(1, sizeof(char) * tag_len);
    char *value = calloc(1, sizeof(char) * value_len);
    memcpy(tag, urlp_tokens[i], k);
    memcpy(value, urlp_tokens[i] + k + 1, value_len);
    printf("(init_header)->Step 1.10.0 :: [%d][%d] - [%d][%d][%d] - [%s] - [%s][%s]\n", i, k, tlen, tag_len, value_len, urlp_tokens[i], tag, value);

    hh->url_params_ordered[i] = malloc(sizeof(char) * tag_len + 1);
    memcpy(hh->url_params_ordered[i], tag, tag_len + 1);
    ht_set(hh->url_params_values, tag, value);
  }
  printf("(init_header)->Step 1.10.0 :: PARAMETERS [%d] OK\n", urlp_tokens_len + 1);

  return 0;
}

bool is_header_present(http_header *hh, char *header_key) {
  if (hh == NULL || hh->header_headers_values == NULL)
    return false;

  if (ht_get(hh->header_headers_values, header_key) != NULL) {
    return true;
  }
  return false;
}

char *get_header(http_header *hh, char *header_key) {
  if (hh == NULL || hh->header_headers_values == NULL)
    return false;

  return ht_get(hh->header_headers_values, header_key);
}

bool is_url_parameter_present(http_header *hh, char *parameter_key) {
  if (hh == NULL || hh->url_params_values == NULL)
    return false;

  if (ht_get(hh->url_params_values, parameter_key) != NULL) {
    return true;
  }
  return false;
}

char *get_parameter(http_header *hh, char *parameter_key) {
  if (hh == NULL || hh->url_params_values == NULL)
    return false;

  return ht_get(hh->url_params_values, parameter_key);
}

lua_file_signature *get_lua_file_signature(http_header *hh, char *extension) {
  if (hh == NULL)
    return NULL;
  if (extension == NULL || strlen(extension) == 0)
    return NULL;
  lua_file_signature *lfs = malloc(sizeof(lua_file_signature));

  if (hh->verb == NULL) {
    free(lfs);
    return NULL;
  }
  // lfs->verb = malloc(sizeof(char)*strlen(hh->verb));
  // memcpy(lfs->verb,hh->verb,strlen(hh->verb));
  // printf("(get_lua_file_signature)->Step 1.1 :: VERB [%s]\n", hh->verb);

  lfs->verb = malloc(sizeof(char) * (strlen(hh->verb) + 1));  // +1 for the null terminator
  memcpy(lfs->verb, hh->verb, strlen(hh->verb) + 1);          // +1 to include the null terminator
  lfs->verb[strlen(hh->verb)] = '\0';
  printf("(get_lua_file_signature)->Step 1.1 :: VERB [%s]\n", hh->verb);

  if (hh->url_body != NULL) {
    lfs->content_length = hh->content_length;
    lfs->body = malloc(sizeof(char) * (strlen(hh->url_body) + 1));
    memcpy(lfs->body, hh->url_body, strlen(hh->url_body) + 1);
    lfs->body[strlen(hh->url_body)] = '\0';
  } else {
    lfs->content_length = -1;
    lfs->body = NULL;
  }
  printf("(get_lua_file_signature)->Step 1.1.1 :: CONTENT-LENGTH [%d]\n", hh->content_length);
  printf("(get_lua_file_signature)->Step 1.1.2 :: BODY [%s]\n", hh->url_body);

  if (hh->url_base == NULL) {
    free(lfs);
    return NULL;
  }
  printf("(get_lua_file_signature)->Step 1.2 :: URL BASE [%s]\n", hh->url_base);
  int url_base_len = strlen(hh->url_base);
  if (url_base_len == 0 || url_base_len > MAX_HEADER_URL_BASE_LEN)
    return NULL;
  lfs->path = malloc(sizeof(char) * url_base_len + 1);
  memcpy(lfs->path, hh->url_base, url_base_len + 1);
  lfs->path[url_base_len] = '\0';

  int extension_len = strlen(extension);

  char *f_delimiter = ".";
  int f_delimiter_len = strlen(f_delimiter);

  char *df;
  int df_default_len = strlen(DEFAULT_LUA_FILE_NAME);
  if (hh->_epnt != NULL) {
    int df_epnt_len = strlen(hh->_epnt);
    df = malloc(sizeof(char) * df_epnt_len + 1);
    memcpy(df, hh->_epnt, df_epnt_len + 1);
  } else {
    df = malloc(sizeof(char) * df_default_len + 1);
    memcpy(df, DEFAULT_LUA_FILE_NAME, df_default_len + 1);
  }
  int df_len = strlen(df);
  int verb_len = strlen(hh->verb);
  int verb_prefix_len = strlen(DEFAULT_LUA_VERB_PREFIX);
  int verb_delimiter_len = strlen(DEFAULT_LUA_VERB_DELIMITER);

  printf("(get_lua_file_signature)->Step 1.3 :: LENGTH [%d][%d][%d][%d][%d][%d]\n", verb_prefix_len, verb_len, verb_delimiter_len, df_len,
         f_delimiter_len, extension_len);

  lfs->file = malloc(sizeof(char) * (verb_prefix_len + verb_len + verb_delimiter_len + df_len + f_delimiter_len + extension_len) + 1);
  memcpy(lfs->file, DEFAULT_LUA_VERB_PREFIX, verb_prefix_len);
  memcpy(lfs->file + verb_prefix_len, hh->verb, verb_len);
  memcpy(lfs->file + verb_prefix_len + verb_len, DEFAULT_LUA_VERB_DELIMITER, verb_delimiter_len);

  memcpy(lfs->file + verb_prefix_len + verb_len + verb_delimiter_len, df, df_len);
  memcpy(lfs->file + verb_prefix_len + verb_len + verb_delimiter_len + df_len, f_delimiter, f_delimiter_len);
  memcpy(lfs->file + verb_prefix_len + verb_len + verb_delimiter_len + df_len + f_delimiter_len, extension, extension_len);
  lfs->file[verb_prefix_len + verb_len + verb_delimiter_len + df_len + f_delimiter_len + extension_len] = '\0';
  printf("(get_lua_file_signature)->Step 1.4 :: FILE NAME [%s]\n", lfs->file);
  // PARAMETERS
  printf("(get_lua_file_signature)->Step 1.5 :: PARAMETERS LENGTH [%d]\n", hh->url_params_tokens);
  lfs->parameters_len = hh->url_params_tokens;

  if (hh->url_params_tokens > 0) {
    lfs->parameters = malloc(sizeof(char *) * hh->url_params_tokens + 1);
    lfs->paramters_values = ht_create(hh->url_params_tokens * 2);

    for (int i = 0; i < hh->url_params_tokens; i++) {
      char *tag = hh->url_params_ordered[i];
      char *value = ht_get(hh->url_params_values, hh->url_params_ordered[i]);
      if (tag != NULL && strlen(tag) > 0) {
        lfs->parameters[i] = malloc(sizeof(char) * strlen(tag) + 1);
        memcpy(lfs->parameters[i], tag, strlen(tag) + 1);

        if (value != NULL && strlen(value) > 0) {
          ht_set(lfs->paramters_values, tag, value);
        } else {
          printf("(get_lua_file_signature)->Step 1.5 :: ERROR GETTING THE PARAMETERS :: VALUE FOR TAG [%s] for [%s]\n", value, tag);
          return NULL;
        }
      } else {
        printf("(get_lua_file_signature)->Step 1.5 :: ERROR GETTING THE PARAMETERS :: TAG [%s]\n", tag);
        return NULL;
      }
    }
  }

  return lfs;
}