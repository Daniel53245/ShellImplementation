#include <stdio.h>
#include<stdlib.h>
#include <errno.h>
#include <string.h>
#include "utils.h"
#include "tokeniser.h"
#include "parser.h"
//exit,cd,path

/**
 * @brief  return a type of an uknow expression
 * 
 * @param exp 
 * @return expression_type 
 */
expression_type find_exp_type(void* exp){
  if(exp == NULL){
    return -1;
  }
  expression_type type = ((expression*)exp)->type;
  return type;
}

void* my_malloc(size_t size){
  void* ptr = malloc(size);
  if(ptr == NULL){
    return NULL;
  }
  return ptr;
}

void destory_command_exp(command_exp* exp){
  if(exp == NULL){
    return;
  }
  if(exp->args != NULL){
    //free all the sub string in args till reach a null pointer
    int i = 0;
    while(exp->args[i] != NULL){
      free(exp->args[i]);
      i++;
    }
    free(exp->args);
  }
  free(exp);
}

void destory_pipe_exp(pipe_exp* exp){
  if(exp != NULL){
    destory_command_exp(exp->pre_command);
    if(exp->after_comnand != NULL){
      destory_pipe_exp(exp->after_comnand);
    }
    free(exp);
  }
}

void destory_redir_exp(redir_exp* exp){
  if(exp != NULL){
    destory_pipe_exp(exp->pre_command);
    if(exp->file_name != NULL){
      free(exp->file_name);
    }
    free(exp);
  }
}

void destory_parallel_exp(parallel_exp* exp){
  if(exp != NULL){
	if(exp->pre_command != NULL){
	    destory_redir_exp(exp->pre_command);	
	}

    if(exp->next_expression != NULL){
      destory_parallel_exp(exp->next_expression);
    }
    free(exp);
  }
}

parallel_exp* parse_parallel(Node* token_list,int* error){
  //print the node list on recursion
  if(*error != 0){//error in some process
    return NULL;
  }
  if(token_list == NULL){//base case for recursion
    return NULL;
  }
  //token list not null parse expression
  parallel_exp *exp_tmp = (parallel_exp*)my_malloc(sizeof(parallel_exp));
  //handle malloc error
  if(exp_tmp == NULL){
    *error = 1;
    return NULL;
  }
  //parse the 1st expression(the node list is never empty at this step)
  Node* new_head = NULL;
  exp_tmp->type = PARALLEL_EXP;
  redir_exp* redir_exp_tmp = parse_redir(token_list,&new_head,error);
  if(redir_exp_tmp == NULL || *error != 0){//should always have one being parsed
    if(*error == 0){
      *error = 1;
    }
    if(redir_exp_tmp != NULL){
      destory_redir_exp(redir_exp_tmp);
    }
    free(exp_tmp);
    //TODO recycle memory asap
    return NULL;
  }
  exp_tmp->pre_command = redir_exp_tmp;
  //check for more command to parse
  if(new_head == NULL){//no more command to parse we are done
    exp_tmp->next_expression = NULL;
    return exp_tmp;
  }
  //more token to parse
  Token *next = (Token*)new_head->content;
  //Redirexp = redirexp && redirexp / redirexp
  switch(next->type){
    case(Token_BACKGROUND)://possible leak
      new_head = new_head->next;
      //recursive call to self
      exp_tmp->next_expression = parse_parallel(new_head,error); 
      break;
    default://TODO:POTENTIAL LEAK INDICATED
      *error = 1;
      return NULL;
  }
  if(*error != 0){//recursion problem
    destory_parallel_exp(exp_tmp);
    return NULL;
  }
  return exp_tmp;
}

/**
 * @brief parse a redir expression from a list of tokens
 * 
 * @param token_list 
 * @param end_node 
 * @param error 
 * @return redir_exp* 
 * Safe on error no memory lead when NULL ?
 * TODO seems to be removing background token
 */
redir_exp* parse_redir(Node* token_list,Node **end_node,int* error){
    if(*error != 0){//error in some process
    return NULL;
  }
  //destory upon destruction
  redir_exp* exp = (redir_exp*)my_malloc(sizeof(redir_exp));
  //handle error in malloc
  if(exp == NULL){
    *error = 1;
    return NULL;
  }
  exp->type = REDIR_EXP;
  if(token_list == NULL){
    return NULL;
  }
  //pars sub expression should be destoried upon error
  Node* new_head = NULL;
  pipe_exp* pipe_exp_tmp = parse_pipe(token_list,&new_head,error);
  if(pipe_exp_tmp == NULL){
    *error = 1;
    //TODO recycle memory asap
    free(exp);
    return NULL;
  }
  exp->pre_command = pipe_exp_tmp;

  if(new_head== NULL){// no more tokens, we are done
    exp -> file_name = NULL;
    if(end_node != NULL){
      *end_node = new_head;
    }else{
      fprintf(stderr,"Parser.c/parse_redir:WTF:end node is null\n");
    }
    if(*error != 0){
      destory_redir_exp(exp);
      return NULL;
    }
    return exp;
  }  //how could this be null
  Token* new_head_token = (Token*)new_head->content;
   switch(new_head_token-> type){
    case(Token_REDIRECT):
      new_head = new_head->next;//skip the redirect token
      if(new_head == NULL){
        *error = 1;
        return NULL;
        break;
      }
      new_head_token = (Token*)new_head->content;
      exp->file_name = strdup(new_head_token->content);
      new_head = new_head->next;
      break;
    case(Token_BACKGROUND):
      exp-> file_name = NULL;
      break;
    default://this should never be the case ? 
      *error = 1;
      break;
  }

  if(*error != 0){
    destory_redir_exp(exp);
    return NULL;
  }
  if(end_node != NULL){
    *end_node = new_head;
  }
  if(exp == NULL){
    fprintf(stderr,"Parser.c/parse_redir:WTF:redir exp is null\n");
  }
  return exp;
}

/**
 * @brief parse a pipe expression from a list of tokens
 * Note the 2nd exp could be null when there is no pipe
 * @param token_list list of tokens
 * @param[out] end_node till where the token is consumed
 * @param[out] error flag for fatal error in parsng process
 * @return pipe_exp* malloced pipe expression
 * No leak on error(memory is recycled in NULL ?)
 * TODO: WTF not working
 */
pipe_exp* parse_pipe(Node* token_list,Node** end_node,int* error){
  if(*error != 0){//error in some process
    return NULL;
  }
  //recycle on error
  pipe_exp * exp = (pipe_exp*)my_malloc(sizeof(pipe_exp));
  if(token_list == NULL){
    return NULL;
  }
  exp->type = PIPE_EXP;
  //destory upon error
  //print_token_list(token_list);
  command_exp* cmd_exp1 = parse_command(token_list,&token_list,error);
  exp->pre_command = cmd_exp1;
  // if no pipe exist in the token list this step should be the end
  //1. parse_command consument all token
  if(token_list==NULL){
    exp->after_comnand = NULL;
    if(end_node != NULL){
      *end_node = token_list;
    }
    return exp;
  }
  //2 some token left
  Token* next_token = (Token*)token_list->content;
  switch(next_token-> type){
    case Token_PIPE://parse it(part of out role)
      token_list = token_list->next;//skip pipe
      exp -> after_comnand = parse_pipe(token_list,&token_list,error);
      break;
    case Token_NORMAL:
    //should never happend as parse_command would consumen all
    //inputLexit directly gets this 
      *error = 1;
      return NULL;
      break;
    default:
      //other tokens of higher level wedo not process
      exp->after_comnand = NULL;
      break;
  }
  //as above have call to parse_command we need to check error again
  if(*error != 0){
    //free necessary memory
    if(cmd_exp1 != NULL){
      destory_command_exp(cmd_exp1);
    }
    if(exp->after_comnand != NULL){
      destory_pipe_exp(exp->after_comnand);
    }
    free(exp);
    return NULL;
  }
  //update end node
  if(end_node != NULL){
    *end_node = token_list;
  }
  return exp;
}

/**
 * @brief parse a command expression form a list of tokens
 * @param token_list a list of tokens
 * @param[out] pointer to the node after return(comsumption)
 * @param[out] error flag for fatal error in parsng process
 * @return a command expression
 * @error return NULL upon error
 * parse a command_exp from a list of tokens(malloced), do not mutate input
 * assmuing the commnad_exp free by the caller 
 */
command_exp* parse_command(Node* token_list,Node** end_node,int* error){
  if(*error != 0){//error in some process
    return NULL;
  }
  //allocate necessary memory
  //free exp upon error
  if(token_list == NULL){
    // at case parse pipe exp ther will be empty token list
    *error = 1;
    return NULL;
  }
  int buffer_size = 1024;
  command_exp* exp = (command_exp*)my_malloc(sizeof(command_exp));
  exp->type = COMMAND_EXP;
  char ** args = my_malloc(sizeof(char*)*buffer_size);
  int argc = 0;
  
  if(exp == NULL || args == NULL){
    * error = 1;
    free(exp);
    free(args);
    return NULL;
  }

  //keep apeding till reach a non Token_NORMAL token
  Node* head = token_list;
  while(head != NULL){
    Token* token_tmp = (Token*)head->content;
    if(token_tmp == NULL){
      //warning
      break;
    }
    if(token_tmp->type == Token_BUILTIN){
      //builtin command should never get here
      *error = 1;
      return NULL;
    }

    if(token_tmp->type == Token_NORMAL){
      if(argc == buffer_size){//extend thebuffer when toolong
        buffer_size += buffer_size;
        char** args_tmp = realloc(args,sizeof(char*)*buffer_size);
        if(args_tmp == NULL){
          *error = 1;
          free(exp);
          free(args);
          return NULL;
        }
        args = args_tmp;
      }
      //usr str cpy to copy the string
      args[argc] = strdup(token_tmp->content);
      argc++;
      head = head->next;
    }else{//at a symbol of higher level
      break;
    }
  }
  if(argc == 0){//input: exit treiigers
    *error = 1;
    return NULL;
  }
  //add null at the end(execvp requirement)
  args[argc] = NULL;
  exp->args = args;
  if(end_node != NULL){
   *end_node = head;
  }
  return exp;
}