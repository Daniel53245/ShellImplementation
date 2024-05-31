#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "tokeniser.h"
#include "utils.h"

const char * pipe_symble = "|";
const char* input_redirection_symble = ">";
const char * background_symble = "&";
const int built_in_command_count = 3;
const char* built_in_commands[] = {"cd","exit","path"};


/**
 * @brief      Given a string, chcek if it in the build tin command
 * @param[in]  command  char* the potitial string
 * @return     1 if it is a built in command, 0 otherwise
 * Does not alter the original string
*/
int is_built_in_command(char* command){
        for(int i = 0; i < built_in_command_count; i++){
                if(strcmp(command,built_in_commands[i]) == 0){
                        return 1;
                }
        }
        return 0;
}

/**
 * @brief      Creates a new token iwth given attribute
 * @param[in]  content  The content of the token
 * @param[in]  type     The type of the token
 * create the struct Tokne and allocat memory with malloc
*/
Token *new_token(char* content,int type){
        Token *token = malloc(sizeof(Token));
        if(token == NULL){
                return NULL;
        }

        token -> content = malloc(strlen(content));
        if (token->content == NULL){
                free(token);
                return NULL;
        }
        strcpy(token->content,content);
        token->type = type;
        return token;
}

void destory_token(Token *token){
        if(token != NULL){
                free(token -> content);
                free(token);
        }
        return;
}

/**
 * @param[in]  content  input stirng
 * @param[out]  rtn_size the number of token in the final partition
 * @return     list of string
 * the string si partition with strsep with delim " "
 * does not alter input text 
 * the return list is a list of string that is malloced
*/
char** str_partition(char* input,int* rtn_size){
    int size = 0;  
    char **tokens = NULL;  
    char *temp = strdup(input);  
    
    char *ptr = temp;
    while (*ptr) {
        if (*ptr == ' ') {
            ptr++; // skip spaces
            continue;
        }
        if (strchr("&|>", *ptr)) {
            tokens = realloc(tokens, sizeof(char*) * (size + 1));
            tokens[size] = malloc(2);
            tokens[size][0] = *ptr;
            tokens[size][1] = '\0';
            size++;
            ptr++;
        } else {
            char *start = ptr;
            while (*ptr && !strchr(" &|>", *ptr)) {
                ptr++;
            }
            tokens = realloc(tokens, sizeof(char*) * (size + 1));
            tokens[size] = strndup(start, ptr - start);
            size++;
        }
    }
    
    tokens = realloc(tokens, sizeof(char*) * (size + 1));
    tokens[size] = NULL;  // NULL-terminate the list of tokens
    *rtn_size = size;
    free(temp);  
    return tokens;
}


/**
 * @brief give a list of stirng return the list of token in Node*
 * 
 * @param str string to be tokensied
 * @return Node* the list of tokens stored in doublely liked list
 */
Node* tokenise_str(char* str){
        //sanitise input
        if(str == NULL){
                ERROR(EINVAL,"Tokeniser.c: tokennise_str failed NULL input\n");
                return NULL;
        }

        //division to substing by str_partition
        int sub_str_count = 0;
        //todo free sub_str_list at end
        char** sub_str_list = str_partition(str,&sub_str_count); 

        //creat token list using node in utils.h
        Node* token_list = NULL;
        for(int i = 0;i < sub_str_count;i++){
                //check the type of the sub_str
                Token* token = new_token(sub_str_list[i],Token_UNKNOWN);
                if(token == NULL){
                        ERROR(ENOMEM,"Tokeniser.c: tokennise_str failed to allocate memory\n");
                        return NULL;
                }
                //fprintf(stderr, "Tokeniser.c: tokennise_str ?%s?\n",sub_str_list[i]);

                //allocate the type of the token
                if(strcmp(sub_str_list[i],pipe_symble) == 0){
                        token -> type = Token_PIPE;
                }else if (strcmp(sub_str_list[i],input_redirection_symble) == 0){
                        token -> type = Token_REDIRECT;                
                }else if (strcmp(sub_str_list[i],background_symble) == 0){
                        token -> type = Token_BACKGROUND;
                }else if(is_built_in_command(sub_str_list[i])){
                        token -> type = Token_BUILTIN;
                }else{
                        token -> type = Token_NORMAL;
                }
                //copy the content into the token
                //append the token to the token_list
                if(token_list == NULL){
                        token_list = new_node(token,sizeof(Token));
                        if(token_list == NULL){
                                ERROR(ENOMEM,"Tokeniser.c: tokennise_str failed to allocate memory\n");
                                return NULL;
                        }
                }else{
                        int rtn = append_node(token_list,token,sizeof(Token));
                        if(rtn != 0){
                                fprintf(stderr, "Tokeniser.c: tokennise_str failed to append node\n");
                                return NULL;
                        }
                }
        }
        return token_list;
}



