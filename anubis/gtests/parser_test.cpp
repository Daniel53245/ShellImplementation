#include <gtest/gtest.h>
extern "C"{
        #include "../src/utils.h"
        #include "../src/parser.h"
        #include "../src/tokeniser.h"
        #include <string.h>
}

TEST(Parser,parse_command_simple){
        //setup
        int parse_err = 0;
        char* test_command = "ls -l";
        Node* token_list = tokenise_str(test_command);
        Node* new_head = NULL;
        command_exp* outcome = parse_command(token_list,&new_head,&parse_err);

        //test
        fprintf(stderr,"outcome command :%zu\n",outcome->args);
        fprintf(stderr,"outcome arg1 :%s\n",outcome->args[0]);
        fprintf(stderr,"outcome arg2 :%s\n",outcome->args[1]);
        EXPECT_EQ(0,strcmp(outcome->args[0],"ls"));
        EXPECT_EQ(0,strcmp(outcome->args[1],"-l"));
        EXPECT_TRUE(NULL == outcome->args[2]);
        //recycle
        destory_command_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_command_token_head_NULL){
                //setup
        int parse_err = 0;
        char* test_command = "ls -l";
        Node* token_list = tokenise_str(test_command);
        Node* token_list_head = token_list;
        command_exp* outcome = parse_command(token_list,&token_list,&parse_err);

        //test
        fprintf(stderr,"outcome command :%zu\n",outcome->args);
        fprintf(stderr,"outcome arg1 :%s\n",outcome->args[0]);
        fprintf(stderr,"outcome arg2 :%s\n",outcome->args[1]);
        EXPECT_EQ(0,strcmp(outcome->args[0],"ls"));
        EXPECT_EQ(0,strcmp(outcome->args[1],"-l"));
        EXPECT_TRUE(NULL == token_list);
        //recycle
        destory_command_exp(outcome);
        destory_token_list(token_list_head);
}

TEST(Parser,parse_command_token_head_not_NULL){
                //setup
        int parse_err = 0;
        char* test_command = "ls -l |";
        Node* token_list = tokenise_str(test_command);
        Node* token_list_head = token_list;
        //error this line do not finish
        command_exp* outcome = parse_command(token_list,&token_list,&parse_err);
        fprintf(stderr,"outcome command :%zu\n",token_list);
        //test
        fprintf(stderr,"outcome command :%zu\n",outcome->args);
        fprintf(stderr,"outcome arg1 :%s\n",outcome->args[0]);
        fprintf(stderr,"outcome arg2 :%s\n",outcome->args[1]);
        EXPECT_EQ(0,strcmp(outcome->args[0],"ls"));
        EXPECT_EQ(0,strcmp(outcome->args[1],"-l"));
        Token*  token_left = (Token*)token_list->content;
        EXPECT_EQ(0,strcmp(token_left->content,"|"));
        EXPECT_EQ(token_left->type,Token_PIPE);
        //recycle
        destory_command_exp(outcome);
        destory_token_list(token_list_head);
}

TEST(Parser,parse_pipe_pipe){
        int parse_err = 0;
        char* test_command = "ls -l | wc";
        Node* token_list = tokenise_str(test_command);
        pipe_exp* outcome = parse_pipe(token_list,NULL,&parse_err);
        fprintf(stderr,"parse_pipe return\n");
        command_exp *cmd1 = outcome->pre_command;
        pipe_exp *pipe2 = outcome->after_comnand;
        EXPECT_TRUE(pipe2->after_comnand == NULL);
        EXPECT_TRUE(pipe2->pre_command != NULL);
        command_exp* cmd2 = pipe2->pre_command;
        EXPECT_EQ(0,strcmp(cmd1->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd1->args[1],"-l"));
        EXPECT_EQ(0,strcmp(cmd2->args[0],"wc"));

        //recycle
        destory_pipe_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,prse_pipe_command){
        int parse_err = 0;
        char* test_command = "ls -l";
        Node* token_list = tokenise_str(test_command);
        Node* new_head = NULL;
        pipe_exp* outcome = parse_pipe(token_list,&new_head,&parse_err);
        command_exp *cmd1 = outcome->pre_command;
        EXPECT_TRUE(outcome->after_comnand == NULL);
        //recycle
        destory_pipe_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,prse_pipe_returnhead){
        int parse_err = 0;
        char* test_command = "ls -l > wc";
        Node* token_list = tokenise_str(test_command);
        Node* new_list = NULL;
        pipe_exp* outcome = parse_pipe(token_list,&new_list,&parse_err);
        EXPECT_FALSE(new_list == NULL);
        EXPECT_TRUE(outcome->after_comnand == NULL);
        //check first character in new head
        Token* token = (Token*)new_list->content;
        EXPECT_EQ(0,strcmp(token->content,">"));
        EXPECT_EQ(token->type,Token_REDIRECT);
        //recycle
        destory_pipe_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_pipe_complicated){
        int parse_err = 0;
        char* test_command = "ls -l | wc > test.txt";
        Node* token_list = tokenise_str(test_command);
        Node* new_list = NULL;
        pipe_exp* outcome = parse_pipe(token_list,&new_list,&parse_err);
        command_exp *cmd1 = outcome->pre_command;
        pipe_exp *cmd2 = outcome->after_comnand;
        EXPECT_EQ(0,strcmp(cmd1->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd1->args[1],"-l"));
        EXPECT_TRUE(cmd2->after_comnand == NULL);
        command_exp* cmd3 = cmd2->pre_command;
        EXPECT_EQ(0,strcmp(cmd3->args[0],"wc"));
        EXPECT_FALSE(new_list == NULL);
        //check first character in new head
        Token* token = (Token*)new_list->content;
        EXPECT_EQ(0,strcmp(token->content,">"));
        EXPECT_EQ(token->type,Token_REDIRECT);
        //recycle
        destory_pipe_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_redir_no_reidr){
        char* test_str = "ls -l";
        Node* token_list = tokenise_str(test_str);
        int parse_err = 0;
        Node* new_list = NULL;
        redir_exp* outcome = parse_redir(token_list,&new_list,&parse_err);
        fprintf(stderr,"successful return of parse_redir\n");
        pipe_exp* pipe = outcome->pre_command;
        EXPECT_TRUE(pipe->after_comnand == NULL);
        command_exp* cmd = pipe->pre_command;
        EXPECT_EQ(0,strcmp(cmd->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd->args[1],"-l"));
        //recycle
        destory_redir_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_redir_with_redir){
        char* test_str = "ls -a -l > test.txt";
        Node* token_list = tokenise_str(test_str);
        int parse_err = 0;
        Node* new_list = NULL;
        redir_exp* outcome = parse_redir(token_list,&new_list,&parse_err);
        pipe_exp* pipe = outcome->pre_command;
        EXPECT_TRUE(pipe->after_comnand == NULL);
        command_exp* cmd = pipe->pre_command;
        EXPECT_EQ(0,strcmp(cmd->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd->args[1],"-a"));
        EXPECT_EQ(0,strcmp(cmd->args[2],"-l"));
        //check file name
        char* file_name = outcome->file_name;
        EXPECT_EQ(0,strcmp(file_name,"test.txt"));

        //recycle
        destory_redir_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_redir_handl_parallel){
        char* test_str = "ls -a -l > test.txt &";
        Node* token_list = tokenise_str(test_str);
        int parse_err = 0;
        Node* new_list = NULL;
        redir_exp* outcome = parse_redir(token_list,&new_list,&parse_err);
        pipe_exp* pipe = outcome->pre_command;
        EXPECT_TRUE(pipe->after_comnand == NULL);
        command_exp* cmd = pipe->pre_command;
        EXPECT_EQ(0,strcmp(cmd->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd->args[1],"-a"));
        EXPECT_EQ(0,strcmp(cmd->args[2],"-l"));
        //check file name
        char* file_name = outcome->file_name;
        EXPECT_EQ(0,strcmp(file_name,"test.txt"));

        //recycle
        destory_redir_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_parallel_no_paralle){
        char* test_str = "ls -l";
        Node* token_list = tokenise_str(test_str);
        int parse_err = 0;
        Node* new_list = NULL;
        parallel_exp* outcome = parse_parallel(token_list,&parse_err);
        fprintf(stderr,"successful return of parse_parallel\n");
        redir_exp* redir = outcome->pre_command;
        EXPECT_TRUE(redir->file_name == NULL);
        pipe_exp* pipe = redir->pre_command;
        EXPECT_TRUE(pipe->after_comnand == NULL);
        command_exp* cmd = pipe->pre_command;
        EXPECT_EQ(0,strcmp(cmd->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd->args[1],"-l"));
        //recycle
        destory_parallel_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_parapllel_pipe){
        char* test_str = "ls -l | wc";
        Node* token_list = tokenise_str(test_str);
        int parse_err = 0;
        Node* new_list = NULL;
        parallel_exp* outcome = parse_parallel(token_list,&parse_err);
        fprintf(stderr,"successful return of parse_parallel\n");
        redir_exp* redir = outcome->pre_command;
        EXPECT_TRUE(redir->file_name == NULL);
        pipe_exp* pipe = redir->pre_command;
        EXPECT_TRUE(pipe->after_comnand != NULL);
        command_exp* cmd = pipe->pre_command;
        EXPECT_EQ(0,strcmp(cmd->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd->args[1],"-l"));
        pipe_exp* cmd2 = pipe->after_comnand;
        EXPECT_TRUE(cmd2->after_comnand == NULL);
        EXPECT_TRUE(cmd2->pre_command != NULL);
        command_exp* cmd3 = cmd2->pre_command;
        EXPECT_EQ(0,strcmp(cmd3->args[0],"wc"));
        //recycle
        destory_parallel_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parse_parallel_precedance){
        char* test_str = "ls -l | whoami > test.txt";
        Node* token_list = tokenise_str(test_str);
        int parse_err = 0;
        Node* new_list = NULL;
        parallel_exp* outcome = parse_parallel(token_list,&parse_err);
        fprintf(stderr,"successful return of parse_parallel\n");
        redir_exp* redir = outcome->pre_command;
        EXPECT_TRUE(redir->file_name != NULL);
        EXPECT_EQ(0,strcmp(redir->file_name,"test.txt"));
        pipe_exp* pipe = redir->pre_command;
        EXPECT_TRUE(pipe->after_comnand != NULL);
        command_exp* cmd = pipe->pre_command;
        //null terminated char[]
        EXPECT_EQ(0,strcmp(cmd->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd->args[1],"-l"));
        EXPECT_TRUE(cmd->args[2] == NULL);

        pipe_exp* pipe2 = pipe->after_comnand;
        EXPECT_TRUE(pipe2->after_comnand == NULL);
        EXPECT_TRUE(pipe2->pre_command != NULL);
        command_exp* cmd2 = pipe2->pre_command;
        EXPECT_EQ(0,strcmp(cmd2->args[0],"whoami"));
        EXPECT_TRUE(cmd2->args[1] == NULL);
        //recycle
        destory_parallel_exp(outcome);
        destory_token_list(token_list);
}

TEST(Parser,parser_parallel_simple){
        char* tset_str = "whoami & wc";
        Node* token_list = tokenise_str(tset_str);
        int parse_err = 0;
        Node* new_list = NULL;

        parallel_exp* outcome = parse_parallel(token_list,&parse_err);
        EXPECT_TRUE(outcome != NULL);
        fprintf(stderr,"successful return of parse_parallel\n");
        EXPECT_TRUE(outcome->pre_command != NULL);
        redir_exp* redir = outcome->pre_command;
        EXPECT_TRUE(redir->file_name == NULL);
        pipe_exp* pipe = redir->pre_command;
        EXPECT_TRUE(pipe->after_comnand == NULL);
        command_exp* cmd = pipe->pre_command;
        EXPECT_EQ(0,strcmp(cmd->args[0],"whoami"));
        EXPECT_TRUE(cmd->args[1] == NULL);

        parallel_exp* parallel2 = outcome->next_expression;
        EXPECT_TRUE(parallel2->pre_command != NULL);
        redir_exp* redir2 = parallel2->pre_command;
        EXPECT_TRUE(redir2->file_name == NULL);
        pipe_exp* pipe2 = redir2->pre_command;
        EXPECT_TRUE(pipe2->after_comnand == NULL);
        command_exp* cmd2 = pipe2->pre_command;
        EXPECT_EQ(0,strcmp(cmd2->args[0],"wc"));
        EXPECT_TRUE(cmd2->args[1] == NULL);

        //recycle
        destory_parallel_exp(outcome);
        destory_token_list(token_list);
}       

TEST(Parser,parse_parallel_complicated){
        char* test_str = "ls -l | whoami > test.txt & ls -al > w.txt & whoami & hello";
        Node* token_list = tokenise_str(test_str);
        int parse_err = 0;
        Node* new_list = NULL;
        parallel_exp* outcome = parse_parallel(token_list,&parse_err);
        EXPECT_TRUE(outcome!= NULL);
        if(outcome != NULL){
        fprintf(stderr,"successful return of parse_parallel\n");
        }
        //chcek length of out come
        int length = 0;
        parallel_exp* temp = outcome;
        while(temp != NULL){
                length++;
                temp = temp->next_expression;
        }
        fprintf(stderr,"length of paralle :%d\n",length);
        //ls -l | whoami > test.txt
        EXPECT_TRUE(outcome->pre_command != NULL);
        redir_exp* redir1 = outcome->pre_command;
        //no redrection file name shold be null precommand not null
        EXPECT_TRUE(redir1->file_name != NULL);
        EXPECT_EQ(0,strcmp(redir1->file_name,"test.txt"));
        EXPECT_TRUE(redir1->pre_command != NULL);
        pipe_exp* pipe1 = redir1->pre_command;
        //next command should be null pre_command not null
        EXPECT_TRUE(pipe1->after_comnand != NULL);
        EXPECT_TRUE(pipe1->pre_command != NULL);
        command_exp* cmd1 = pipe1->pre_command;
        //checkl values 
        EXPECT_EQ(0,strcmp(cmd1->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd1->args[1],"-l"));
        EXPECT_TRUE(cmd1->args[2] == NULL);
        //whoami
        pipe_exp* pipe_after = pipe1->after_comnand;
        EXPECT_TRUE(pipe_after->after_comnand == NULL);
        EXPECT_TRUE(pipe_after->pre_command != NULL);
        command_exp* cmd2 = pipe_after->pre_command;
        EXPECT_EQ(0,strcmp(cmd2->args[0],"whoami"));
        EXPECT_TRUE(cmd2->args[1] == NULL);
        //______________________________________________
        //ls -al > w.txt
        EXPECT_TRUE(outcome->next_expression != NULL);
        parallel_exp* parallel2 = outcome->next_expression;
                //file name not null precommand not null;
        EXPECT_TRUE(parallel2->pre_command != NULL);
        redir_exp* redir2 = parallel2->pre_command;
        EXPECT_TRUE(redir2->file_name != NULL);
        EXPECT_TRUE(redir2->pre_command != NULL);
        EXPECT_EQ(0,strcmp(redir2->file_name,"w.txt"));

        pipe_exp* pipe2 = redir2->pre_command;
        EXPECT_TRUE(pipe2->after_comnand == NULL);
        EXPECT_TRUE(pipe2->pre_command != NULL);
        command_exp* cmd3 = pipe2->pre_command;
        EXPECT_EQ(0,strcmp(cmd3->args[0],"ls"));
        EXPECT_EQ(0,strcmp(cmd3->args[1],"-al"));
        EXPECT_TRUE(cmd3->args[2] == NULL);
        //______________________________________________
        //whoami
        fprintf(stderr,"Whoami\n");
        EXPECT_TRUE(parallel2->next_expression != NULL);
        parallel_exp* parallel3 = parallel2->next_expression;

        EXPECT_TRUE(parallel3->pre_command != NULL);
        redir_exp* redir4 = parallel3->pre_command;
        //file is null command is not
        EXPECT_TRUE(redir4-> pre_command != NULL);
        EXPECT_TRUE(redir4->file_name == NULL);
        pipe_exp* pipe4 = redir4->pre_command;
        EXPECT_TRUE(pipe4->after_comnand == NULL);
        EXPECT_TRUE(pipe4->pre_command != NULL);
        command_exp* cmd4 = pipe4->pre_command;
        EXPECT_TRUE(cmd4 -> args!= NULL);
        EXPECT_EQ(0,strcmp(cmd4->args[0],"whoami"));
        EXPECT_TRUE(cmd4->args[1] == NULL);
        
        //hello
        EXPECT_TRUE(parallel3->next_expression != NULL);
        parallel_exp* parallel4 = parallel3->next_expression;
        redir_exp* redir5 = parallel4->pre_command;
        //file is null command is not
        EXPECT_TRUE(redir5->file_name == NULL);
        EXPECT_TRUE(redir5->pre_command != NULL);
        pipe_exp* pipe5 = redir5->pre_command;
        EXPECT_TRUE(pipe5->after_comnand == NULL);
        EXPECT_TRUE(pipe5->pre_command != NULL);
        command_exp* cmd5 = pipe5->pre_command;

        EXPECT_TRUE(cmd5 -> args!= NULL);
        EXPECT_EQ(0,strcmp(cmd5->args[0],"hello"));
        EXPECT_TRUE(cmd5->args[1] == NULL);

        //recycle
        destory_parallel_exp(outcome);
        destory_token_list(token_list);
}