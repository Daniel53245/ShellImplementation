#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include<stdlib.h>
#include <errno.h>
#include <string.h>
#include "utils.h"
#include "tokeniser.h"

typedef enum epresion_type{
  COMMAND_EXP,
  PIPE_EXP,
  REDIR_EXP,
  PARALLEL_EXP
} expression_type;

typedef struct command_exp{
  expression_type type;
  char** args;
} command_exp;

//this hosuld only used in identifying type of expression
typedef struct expression{
  expression_type type;
  void* exp;
} expression;

typedef struct pipe_exp{
  expression_type type;
  command_exp* pre_command;
  struct pipe_exp* after_comnand;
} pipe_exp;
typedef struct redir_exp{
  expression_type type;
  pipe_exp* pre_command;
  char* file_name;
} redir_exp;

typedef struct parallel_exp{
  expression_type type;
  redir_exp* pre_command;
  struct parallel_exp* next_expression;
} parallel_exp;

expression_type find_exp_type(void* exp);
void destory_command_exp(command_exp* exp);
void destory_pipe_exp(pipe_exp* exp);
void destory_redir_exp(redir_exp* exp);
void destory_parallel_exp(parallel_exp* exp);
pipe_exp* parse_pipe(Node* token_list,Node** end_node,int* error);
redir_exp* parse_redir(Node* token_list,Node** end_node,int* error);
parallel_exp* parse_parallel(Node* token_list,int* error);
command_exp* parse_command(Node* token_list,Node** end_node,int* error);
#endif