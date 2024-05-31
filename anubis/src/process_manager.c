#include "process_manager.h"
#include "parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stddef.h>

//constatnt
const int MAX_PROCESS_COUNT = 1024;
int FULL_PATH_SIZE = 2048;

/**
 * @brief Create a path list object
 * @return path_list* -1 on failure| malloced address
 */
path_list* create_path_list(){
        path_list* new_list = malloc(sizeof(path_list));
        new_list->list_content = NULL;
        if(new_list == NULL){
                fprintf(stderr,"Create_path_list fail, no enough memroy");
                return -1;
        }
        new_list->list_size = 0;
        return new_list;
};

void print_path_list(path_list* list){
        if(list == NULL){
                return;
        }
        for(int i = 0; i < list->list_size;i++){
                if(list->list_content[i] != NULL){
                        fprintf(stderr,"path_list[%d]:%s\n",i,list->list_content[i]);
                }
        }
}
/**
 * @brief recycle memory coccupied by a path_list
 * 
 * @param list allocated list
 */
void destory_path_list(path_list* list){
        if(list == NULL){
                return;
        }
        for(int i = 0; i < list->list_size;i++){
                if(list->list_content[i] != NULL){
                        free(list->list_content[i]);
                }
        }
        free(list);
};

/**
 * @brief Add a new path string to path list
 * 
 * @param list allocated path list
 * @param new_path path to be added
 * @return path_list* -1 on fatal error
 */
path_list* append_path_to_list(path_list* list,char* new_path){
        if(list -> list_content != NULL){
            list->list_content = realloc(list->list_content,sizeof(char*)*(list->list_size +1));
        }else{
            list->list_content = malloc(sizeof(char*)*(list->list_size +1));
        }
        if(list->list_content){
                list->list_content[(list->list_size)] = strdup(new_path);
                list->list_size = list->list_size + 1;
                return list;
        }else{
                fprintf(stderr,"Append_path_to_list fail, no enough memroy");
                return -1;
        }
}





/**
 * @brief Create a process list object
 * @return process_list* 
 * Need to be free on death of main process
 */
process_list* create_process_list(){
        //on creation a null at first place
        process_list* list = malloc(sizeof(process_list));
        if(list == NULL){
                ERROR(ENOMEM,"process_manager.c: create_process_list failed to allocate memory\n");
                return NULL;
        }
        list->num_of_process = 0;
        list-> pid_list = malloc(sizeof(pid_t*)*MAX_PROCESS_COUNT);
        return list;
}

/**
 * @brief destory a process list object 
 * 
 * @param list pointer to process list object
 */
void destory_process_list(process_list* list){
        if(list == NULL){
                ERROR(EINVAL,"process_manager.c: destory_process_list failed NULL input\n");
                return;
        }
        for(int i = 0;i<list->num_of_process;i++){
                free(list->pid_list[i]);
        }
        free(list->pid_list);
        free(list);
}

/**
 * @brief add a pid_t into the process list 
 * 
 * @param list to be added
 * @param pid pid of new process
 * @return int -1 on failure 0 on success
 * Assuming list exits
 * should not be used in any other case 
 */
int append_process_to_list(process_list* list,pid_t pid){
        if(list == NULL){
                //ERROR(EINVAL,"process_manager.c: append_process_to_list failed NULL input\n");
                return -1;
        }
        if(list->num_of_process >= MAX_PROCESS_COUNT){
                ERROR(ENOMEM,"process_manager.c: append_process_to_list failed to allocate memory\n");
                return -1;
        }
        pid_t* new_pid = malloc(sizeof(pid_t));
        if(new_pid == NULL){
                ERROR(ENOMEM,"process_manager.c: append_process_to_list failed to allocate memory\n");
                return -1;
        }
        *new_pid = pid;
        list->pid_list[list->num_of_process] = new_pid;
        list->num_of_process++;
        return 0;
}

/**
 * @brief remove a pid from a pid list
 * 
 * @param list 
 * @param pid 
 * @return int -1 on filure
 * assuming the list exust and pid is in the list 
 * should not be used in other ocndition
 */
int remove_process_from_list(process_list* list,pid_t pid){
        if(list == NULL){
                ERROR(EINVAL,"process_manager.c: remove_process_from_list failed NULL input\n");
                return -1;
        }
        //consider condition num_of_process == 1 or 0
        if(list->num_of_process == 0){
                ERROR(ENOENT,"process_manager.c: remove_process_from_list failed to find the pid\n");
                return -1;
        }

        if(list->num_of_process==1 ){
                if(*(list->pid_list[0]) == pid){
                        free(list->pid_list[0]);
                        list->num_of_process--;
                        return 0;
                }else{
                        ERROR(ENOENT,"process_manager.c: remove_process_from_list failed to find the pid\n");
                        return -1;
                }
        }

        for(int i = 0;i<list->num_of_process;i++){
                if(*(list->pid_list[i]) == pid){
                        free(list->pid_list[i]);
                        list->pid_list[i] = list->pid_list[list->num_of_process-1];
                        list->num_of_process--;
                        return 0;
                }
        }
        ERROR(ENOENT,"process_manager.c: remove_process_from_list failed to find the pid\n");
        return -1;
}



/**
 * @brief      Given name to a poteintial executable, return the full path of the executable
 * do nothing to a relative path
 * @param[in]  programe_name  The programe name
 * @param       path_list     path_list* struct
 * @return     NULL on failure
 * Assuming caller free return value 
*/
char* resolve_abs_path(char* programe_name,path_list* path_list){
        //sanitising input

        if(programe_name == NULL){
                ERROR(EINVAL,"Tokeniser.c: reolve_abs_path failed NULL input\n");
                return NULL;
        }
        if (strlen(programe_name) == 0)
        {
                ERROR(EINVAL,"Tokeniser.c: reolve_abs_path failed empty stirng\n");
                return NULL;
        }
        if(path_list == NULL){
                ERROR(EINVAL,"Tokeniser.c: reolve_abs_path failed NULL input\n");
                return NULL;
        }

        int path_list_size = path_list->list_size;
        char** path_list_content = path_list->list_content;
        //resolving path of programme
        if (programe_name[0] == '.' || programe_name[0] == '~' || programe_name[0] == '/'){
                // Treat as abs path
                //let execvp to findfile
                return strdup(programe_name);
        }
        //use access to find the file
        char * full_path  = malloc(FULL_PATH_SIZE);
        for(int i = 0;i<path_list_size;i++){
                // free full_path later(
                int max_len = strlen(path_list_content[i]) + strlen(programe_name) + 2;
                if(max_len > FULL_PATH_SIZE){
                        FULL_PATH_SIZE = max_len;
                        full_path = realloc(full_path,FULL_PATH_SIZE);
                }
                full_path = strcpy(full_path,path_list_content[i]);
                full_path = strcat (full_path,"/");
                full_path = strcat (full_path,programe_name);
                //fprintf(stderr,"full path %s\n",full_path);
                //fprintf(stderr, "Check access \"%s\" is %d\n",full_path,access(full_path,X_OK));

                if(access(full_path,X_OK) == 0){
                        //fprintf(stderr, "Found the file %s\n",full_path);
                        return full_path;
                }else{
                        //file not found keep going
                }
        }
        ERROR(ENOENT,"Process Manager.c: reolve_abs_path failed to find the file |%s|\n",programe_name);
        free(full_path);
        return NULL;
}


// /**
//  * @brief Wrapper for c Execvp function
//  * @param cmd the command to be executed
//  * @param args the arguments of the command
//  * @return see execvp
//  * prints the error message if execvp fails
// */
// int Execvp(char* cmd,char* args[]){
//         int ret = execvp(cmd,args);
//         if (ret == -1)
//         {
//                 ERROR(errno,"execvp failed for %s",cmd);
//         }
//         return ret;
// }

//___________________________________________________

/**
 * @brief check taht the file have a .sh extension
 * 
 * @param file_name 
 * @return int 1 on sucess 
 */
int is_sh_file(char* file_name){
    int len = strlen(file_name);
    if(len < 3){
        return 0;
    }else{
        int ends = strcmp(&file_name[len - 3], ".sh") == 0;
        if(ends != 0 ){
            return 1;
        }else{
            return ends;
        }
    }
}

/**
 * @brief execute the bash
 * @param bash_file sh file to executed
 * @param path path list holding search path
 * @return int 
 */
int execute_bash(char* bash_file,path_list* path_list){
        if(path_list == NULL){
            //un able to find path
                return -1;
        }
        char** path = path_list->list_content;
        int path_count  = path_list->list_size;
        //open a bash file 
        //chck it for 
        char* abs_path = resolve_abs_path(bash_file,path_list);
        if(abs_path == NULL){
            //upable to find the file
                return -1;
        }

        //fork and executes it 
        pid_t pid = fork();
        if(pid == 0){
                execlp(abs_path,abs_path,NULL);
                exit(1);
        }
        waitpid(pid,NULL,0);
        free(abs_path);
        return 0;
}


/**
 * @brief execute command should only be in child process
 * TODO: add support for sh file
 * @param cmd  command expression to be executed
 * @param path_list search path for resolving executabel
 */
void execute_command(command_exp* cmd,path_list* path_list) {
    //check if the command_exp contains a sh file
    char* cmd_binary = cmd->args[0];
    if(is_sh_file(cmd_binary) == 1){
        //grind execute_bash file
        execute_bash(cmd_binary,path_list);
        return;
    }
    //fprintf(stderr,"Execute command call arg[0] %s\n",cmd->args[0]);
    char* full_path = resolve_abs_path(cmd->args[0],path_list);
    if(full_path == NULL){
            exit(1);
    }
    execvp(cmd->args[0], cmd->args);
}

/**
 * @brief setup pipe and call execute command for a chain of pipe
 * TODO: fuction to long consider reactor it
 * @param pipe_exp_in the pipe expression
 * @param pid_list the list of process
 * @param redir_out read end of previous channel
 * @param path_list struct path_list* the list of path
 */
void execute_pipe(pipe_exp* pipe_exp_in,process_list* pid_list,int redir_out,path_list* path_list) {
    pid_t child_pid = -1;
    //special case only one pip chain
    if(pipe_exp_in->after_comnand == NULL){
        child_pid = fork();
        if(child_pid == 0){
            if(redir_out != -1){
                    dup2(redir_out,STDOUT_FILENO);
            }
            execute_command(pipe_exp_in->pre_command,path_list);
            exit(0);
        }
        waitpid(child_pid,NULL,0);
        return;
    }
    
    command_exp* cmd = NULL;
    int my_pipe[2];// grand pipe ?
    int prevout = -1;
    pipe_exp* pipe_exp_tmp = pipe_exp_in;

    while(pipe_exp_tmp != NULL){
        if(prevout == -1){
            //setup pipe
            //1 we are in the at the first :prevout == -1 with a speciall case of no need for more pipe
            pipe(my_pipe);
            //write to 1 read form 0
            prevout = my_pipe[0];
            //fork
            child_pid = fork();
            if(child_pid == 0){
                close(my_pipe[0]);//read from prev not needed
                dup2(my_pipe[1],STDOUT_FILENO);//writing to pipe
                execute_command(pipe_exp_tmp->pre_command,path_list);
                exit(0);
            }
            waitpid(child_pid,NULL,0);
            close(my_pipe[1]);
            //(stderr,"first child retrn \n");
            //fprintf(stderr,"Prevout:%d\n",prevout);
        } else if(pipe_exp_tmp -> after_comnand != NULL){//prevout != NULL
            //2 we are in the middle :prevout != NULL make a new fd dup our out and in??? stucked
            pipe(my_pipe);
            //write to 1 read form 0
            child_pid = fork();
            if(child_pid == 0){
                //read from prev not needed
                dup2(prevout,STDIN_FILENO);
                dup2(my_pipe[1],STDOUT_FILENO);
                close(my_pipe[0]);
                execute_command(pipe_exp_tmp->pre_command,path_list);
                exit(0);
            }
            waitpid(child_pid,NULL,0);
            prevout = my_pipe[0];
            close(my_pipe[1]);
        }else{//problem with input 
            //3 we are at the last :pip_exp_tmp -> aftercommand = null, do not dup STDOUT
            //fork
            //fprintf(stderr,"Last command, prevout:%d\n",prevout);
            pipe(my_pipe);
            if(redir_out != -1){
                //this one is causing problem
                    //dup2(redir_out,STDOUT_FILENO);
            }else{
                    dup2(STDOUT_FILENO,my_pipe[1]);
                    close(my_pipe[1]);
            }
            child_pid = fork();
            if(child_pid == 0){
                //read from prev not needed
                dup2(prevout,STDIN_FILENO);
                dup2(my_pipe[1],STDOUT_FILENO);
                close(my_pipe[0]);
                execute_command(pipe_exp_tmp->pre_command,path_list);
                exit(0);
            }
            waitpid(child_pid,NULL,0);
            close(prevout);
            //fprintf(stderr,"Last child retrn \n");
        }
        pipe_exp_tmp = pipe_exp_tmp->after_comnand;
    }
    //keep going
    return;
}


/**
 * @brief execute redirection process and execute a rediection epxression
 * when no parallel is needed executed in the main process
 * else in a child process generated by executed parallel
 * @param redir the redir expression to be executed
 * @param pid_list list of pid in tracking
 * @param path_list path vaiable 
 * @param path_list_size size of path variable
 */
void execute_redirection(redir_exp* redir,process_list* pid_list,path_list* path_list) {
    if(redir->file_name == NULL ){
		execute_pipe(redir->pre_command,pid_list,-1,path_list);
        return;
	}

    //open a file for redirection
    if (redir->pre_command) {//sus
        //substitide stdout to the file
        FILE* file = fopen(redir->file_name, "w");         
        int file_fd = fileno(file);
        //problemtic
        pid_t child_pid = fork();
        if(child_pid == 0){
            dup2(file_fd, STDOUT_FILENO);
            execute_pipe(redir->pre_command,pid_list,file_fd,path_list);
            fclose(file);
            exit(0);
        }
        waitpid(child_pid,NULL,0);
        return;
    }else{
        //ths shows when only > exesits sould neve be the case
        fprintf(stderr,"Error: no command to execute\n");
        fprintf(stderr,"Redir to file %s\n",redir->file_name);
        exit(1);
        return;
    }
}


/**
 * @brief need reevaluation on usefullness of fuction
 * 
 * @param pid_list 
 * @return pid_t 
 */
pid_t fork_and_track(process_list* pid_list) {
    pid_t pid = fork();
    if (pid > 0) {
        // In the parent process
        append_process_to_list(pid_list, pid);
    }
    return pid;
}


//always in main process
/**
 * @brief execute a parallel expression(entry poitng of process_manager)
 * 
 * @param parallel iput expression
 * @param pid_list list to track all the child
 * @param path_list search path
 */
void execute_parallel(parallel_exp* parallel,process_list* pid_list,path_list* path_list) {
    //fprintf(stderr,"Execute parallel call arg[0] %s\n",parallel->pre_command->pre_command->pre_command->args[0]);
    if (parallel->next_expression != NULL) {
        if (fork_and_track(pid_list) == 0) {//in the child process
            execute_redirection(parallel->pre_command,pid_list,path_list);
            //stop everthing will be handle
            exit(0);
        } else {//in parent process do reccursion
            //fprintf(stderr,"Pareent process check next parallen\n");
            execute_parallel(parallel->next_expression,pid_list,path_list);
            //the stragety for witing is tricky 
            return;
        }
    } else {//no parallel is needed
        execute_redirection(parallel->pre_command,pid_list,path_list);
        return;
    }
}


void wait_for_all_processes(process_list* list) {
    int* rtn_status = malloc(sizeof(int));
    pid_t pid = waitpid(-1,rtn_status,0);
    for (int i = 0; i < list->num_of_process; i++) {
        if(*(list->pid_list[i]) == (pid_t)pid){
            remove_process_from_list(list, *list->pid_list[i]);
        }
    }
}



//___________________________________________________


/**
 * @brief Wrapper for c fork function
 * @return see fork
 * prints the error message if fork fails
*/
pid_t Fork(){
        pid_t pid = fork();
        if(pid == -1){
                ERROR(errno,"Fork failed");
        }
        return pid;
}

