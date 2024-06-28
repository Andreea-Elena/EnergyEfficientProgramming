#include "../headers/str_utils.h"

void print_next_token(nt *NT) { printf("...(print_next_token)->[%s]\n", NT->str_token); }

// Tokenize the given string by token and returns the array of string resulting
int tokenizer(char *str, char *token, char **ret) {
  printf("(tokenizer)->started...\n");
  printf("(tokenizer)-> String \n[\n%s\n]\n\n", str);
  printf("(tokenizer)-> Token \n[\n%s\n]\n\n", token);

  int str_length = strlen(str);
  char *strCopy = (char *)malloc(str_length + 1);
  memcpy(strCopy, str, str_length + 1);
  strCopy[str_length] = '\0';

  nt *NT = get_next_token(strCopy, token);
  if (NT == NULL) {
    printf("(tokenizer)->No tokens found - NULL\n");
    return -1;
  }

  int i = 0;
  ret[i] = malloc(sizeof(char) * strlen(NT->str_token) + 1);
  memcpy(ret[i], NT->str_token, strlen(NT->str_token) + 1);
  ret[i][strlen(NT->str_token)] = '\0';

  printf("(tokenizer)->looping...\n");
  while (NT != NULL) {
    // printf("==[%d]==\n",i);
    print_next_token(NT);

    NT = get_next_token(NT->str_remaining, token);

    if (NT != NULL && NT->str_remaining == NULL) {
      print_next_token(NT);
      i++;
      if (i > MAX_HEADER_HEADERS)
        break;

      ret[i] = malloc(sizeof(char) * strlen(NT->str_token) + 1);
      memcpy(ret[i], NT->str_token, strlen(NT->str_token) + 1);
      ret[i][strlen(NT->str_token)] = '\0';
      break;
    } else if (NT != NULL) {
      if (strlen(NT->str_token) == 0) {
        printf("(tokenizer)->Empty token\n");
      } else {
        i++;
        if (i > MAX_HEADER_HEADERS)
          break;
        ret[i] = malloc(sizeof(char) * strlen(NT->str_token) + 1);
        memcpy(ret[i], NT->str_token, strlen(NT->str_token) + 1);
        ret[i][strlen(NT->str_token)] = '\0';
      }
    }
  }

  printf("(tokenizer)->ended...[%d]\n", i);
  free(NT);
  return i;
}

nt *get_next_token(char *str, char *token) {
  if (str == NULL)
    return NULL;

  int str_length = strlen(str);
  int token_length = strlen(token);
  nt *NT_RET = malloc(sizeof(nt));

  char *str_first = strstr(str, token);
  if (str_first == NULL) {
    if (strlen(str) > 0 && str != NULL && (strcmp(token, "\r\n") != 0 && strcmp(token, "\n") != 0)) {
      printf("(tokenizer)(get_next_token)->Tokenizer reached the end, returning remaining string\n");
      // Case the token is not \r\n. or new line in general, wereturn  the whole string until the end

      NT_RET->str_remaining = NULL;
      NT_RET->str_token = malloc(sizeof(char) * str_length + 1);
      memcpy(NT_RET->str_token, str, str_length + 1);
      NT_RET->str_token[str_length] = '\0';
      printf("%s\n", NT_RET->str_token);
      return NT_RET;
    }
    printf("(tokenizer)(get_next_token)->Could not find token, returning NULL\n");
    free(NT_RET);  // Free allocated memory before returning NULL
    return NULL;
  }
  int strflen = strlen(str_first);
  int flen = str_length - strflen;
  // printf("(tokenizer)(get_next_token)->Found next token [%d] with length [%d] - [%d] - [%d]\n", token_length, flen, strflen,str_length);

  char *strf = malloc(sizeof(char) * (flen + 1));
  memcpy(strf, str, flen + 1);
  strf[flen] = '\0';
  // printf("(tokenizer)(get_next_token)->Token\n[\n%s\n]\n", strf);

  int str_remaining_length = strflen - token_length;  // extract the token length, we do not want that included in the string
  NT_RET->str_remaining = malloc(sizeof(char) * str_remaining_length + 1);
  memcpy(NT_RET->str_remaining, str_first + token_length, str_remaining_length + 1);
  NT_RET->str_remaining[str_remaining_length] = '\0';
  NT_RET->str_token = strf;

  return NT_RET;
}
