#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h> 
#include <fcntl.h>
#include <assert.h>
#include <stdarg.h>
#include <errno.h>
#include "tokeniser.h"
#include "parser.h"
#include "process_manager.h"
#include "utils.h"

char **path;
int path_count;
process_list *processes_list;

/** @brief  execute a builtin command 
 * @param args list of string containing the command and its arguments
 * @param argc number of input to command
 * @param list path_list** path variable
 * @return int 1 on success, 0 on failure no return on exit erro handled inside
 */
int execute_built_in(char** args_builtin,int argc,path_list** list){

    if(strcmp(args_builtin[0],"exit") == 0){
        if(argc > 1 ){
            ERROR(EINVAL,"Too many arguments for exit");
            return 0;
        }
        exit(0);
    } else if(strcmp(args_builtin[0],"path")==0){
        //destory path completely 
        destory_path_list(*list);
        *list = create_path_list();
        for(int i = 1;i < argc;i++){
            char* path_value = args_builtin[i];
            append_path_to_list(*list,path_value);
            if(*list == -1){
                fprintf(stderr,"Anubis.c/execute_built_in:Error appending path to list\n");
                exit(1);
            }
        }
            return 1;
    }else if(strcmp(args_builtin[0],"cd")==0){
        //processing cd command
        if(argc > 2){
            ERROR(EINVAL,"Too many arguments for cd");
            return 0;
        }
        if(argc == 1){
            ERROR(EINVAL,"No enough argument for cd");
            return 0;
        }
        char*  relative_dir = NULL;
        switch(args_builtin[1][0]){//processing realtive and absolute path
            case '/':
                relative_dir = (args_builtin[1]);
                break;
            case '.':
                relative_dir = (args_builtin[1]);
                break;
            case '~':
                relative_dir = (args_builtin[1]);
                break;
            default:
                relative_dir = malloc(sizeof(char)*3 + sizeof(char)*strlen(args_builtin[1]));
                relative_dir[0] = '.';
                relative_dir[1] = '/';
                relative_dir[2] = NULL;
                strcat(relative_dir,args_builtin[1]);
                break;
        }
        int chdir_rtn = chdir(relative_dir);
        if(chdir_rtn != 0){
            ERROR(EINVAL,"Error changing directory");
            return 0;
        }
        return 1;
        //change directory
    }else{
        ERROR(EINVAL,"Unknown built in command");
        return 0;
    }
}

/**
 * @brief main fucntion
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char *argv[])
{
    //sanitise stdin
    if(argc > 2){
        ERROR(EINVAL,"Too many arguments");
        return 1;
    }

    //setup fiel structure for anubis
        //path related structre
    path_list* path_list = create_path_list();
    if(path_list == -1){
        return 1;
    }
    path_list = append_path_to_list(path_list,"/bin");
    
        //setup STDIN
    FILE* input_file = NULL;
    //args[1] is a file containing input command
      if(argc > 1){
        if(access(argv[1],F_OK|R_OK) == 0){
        //read input form a file 
        input_file = fopen(argv[1],"r");
        }else{
            ERROR(errno,"Error opening input file");
            return 1;
        }
    }else{
        input_file = stdin;
    }

        //struiture related to readlie
    char * input_buffer = NULL;
    long unsigned int default_buffer_size = 1024;

    //main loop for comsuing
    while(1){
    //read input
    size_t input_size = getline(&input_buffer,&default_buffer_size,input_file);

    
    if(input_size == -1){
        exit(0);
    }
    if(strlen(input_buffer) == 1){
        exit(0);
    }
    char* input_buffer_copy = NULL;
    if(input_buffer[input_size-1] == '\n'){
        input_buffer[input_size-1] = '\0'; 
    }

    //detecting builtin command
    input_buffer_copy = strdup(input_buffer);
    int num_sub_str = 0;
    char** sub_str_list = str_partition(input_buffer_copy,&num_sub_str);
    if(num_sub_str == 0){
        continue;
    }
    //execute built-in command and free related resources
    if(is_built_in_command(sub_str_list[0])){
        execute_built_in(sub_str_list,num_sub_str,&path_list);
        free(sub_str_list);
        free(input_buffer_copy);
        continue;
    }else if(sub_str_list[0][0] == '&'){
        free(sub_str_list);
        free(input_buffer_copy);
        continue;
    }else {
        free(sub_str_list);
        free(input_buffer_copy);
    }

    //command is an input, tokennise,parse and execute
    int error_parsing = 0;
    Node* tokens = tokenise_str(input_buffer);
    if(tokens == NULL){//WTF just skipped
        ERROR(EINVAL,"Error tokenising input, NULL return");
        continue;
    }
    
    //print_token_list(tokens);
    //parsing input
    //case 11 make problem here
    parallel_exp* parsed_exp = parse_parallel(tokens,&error_parsing);
    if(error_parsing !=0){
        ERROR(EINVAL,"Error parsing input");
        continue;
    }

    //executing input        
    //all most function in execute need reactor 
    execute_parallel(parsed_exp,processes_list,path_list);


    //recycle memory
    destory_parallel_exp(parsed_exp);
    continue;

}
return 0;
}