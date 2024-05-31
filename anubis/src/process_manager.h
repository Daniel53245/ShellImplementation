#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "process_manager.h"
#include "parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>

extern const int MAX_PROCESS_COUNT;

typedef struct path_list{
        int list_size;
        char ** list_content;
} path_list;

path_list* create_path_list();
void destory_path_list(path_list* list);
path_list* append_path_to_list(path_list* list,char* new_path);

typedef struct process_list{
        int num_of_process;
        pid_t** pid_list;
} process_list;
 
process_list* create_process_list();
void destory_process_list(process_list* list);
int append_process_to_list(process_list* list,pid_t pid);
int remove_process_from_list(process_list* list,pid_t pid);
void print_path_list(path_list* list);
void execute_pipe(pipe_exp* pipe_exp_in,process_list* pid_list,int redir_out,path_list* path_list);
void execute_parallel(parallel_exp* parallel,process_list* pid_list,path_list* path_list);
int execute_bash(char* bash_file,path_list* path_list);
char* resolve_abs_path(char* programe_name,path_list* path_list);

#endif