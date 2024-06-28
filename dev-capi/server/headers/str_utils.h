#ifndef _STR_UTILS_H
#define _STR_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "http_header.h"

struct NEXT_TOKEN {
  char *str_remaining;
  char *str_token;
};

typedef struct NEXT_TOKEN nt;

// Tokenize the given string by token and returns the array of string resulting
int tokenizer(char *str, char *token, char **ret);

nt *get_next_token(char *str, char *token);

#endif
