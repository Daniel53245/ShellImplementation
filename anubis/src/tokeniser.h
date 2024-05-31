#ifndef MY_TOKENISER
#define MY_TOKENISER


#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "utils.h"
#include <stdio.h>

extern const char * pipe_symble;
extern const char* input_redirection_symble;
extern const char * background_symble;
extern const int built_in_command_count;
extern const char* built_in_commands[];

typedef enum token_type{
  Token_UNKNOWN,
  Token_PIPE,
  Token_REDIRECT,
  Token_BACKGROUND,
  Token_NORMAL,
  Token_BUILTIN
} token_type;

typedef struct Token{
  char* content;
  token_type type;
} Token;

void destory_token(Token *token);
Token *new_token(char* content,int type);
char** str_partition(char* content,int* rtn_size);
Node* tokenise_str(char* str);
int is_built_in_command(char* command);

#endif 
