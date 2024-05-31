#include <gtest/gtest.h>
extern "C"{
        #include "../src/tokeniser.h"
        #include "../src/utils.h"
        #include <string.h>
}

TEST(TokenTest,test_token_content){
  char test_content[] = "This is test string";
  Token *test_token = new_token(test_content,1);
  EXPECT_EQ(strlen(test_content),strlen(test_token->content));
}

TEST(TokenTest,str_partition_test_simple){
  char test_content[] = "This is test string";
  int* test_index = (int*)malloc(sizeof(int));
  char **test_partition = str_partition(test_content,test_index);
  EXPECT_EQ(4,*test_index);
  EXPECT_EQ(0,strcmp(test_partition[0],"This"));
  EXPECT_EQ(0,strcmp(test_partition[1],"is"));
  EXPECT_EQ(0,strcmp(test_partition[2],"test"));
  EXPECT_EQ(0,strcmp(test_partition[3],"string"));
  
}

TEST(TokenTest,TokenTest_str_partition_test_singfle_Test){
  char test_content[] = "This";
  int* test_index = (int*)malloc(sizeof(int));
  char **test_partition = str_partition(test_content,test_index);
  EXPECT_EQ(1,*test_index);
  EXPECT_EQ(0,strcmp(test_partition[0],"This"));
}

TEST(Tokenise_str,simple){
  char test_content[] = "This is test string";
  Node *out_come = tokenise_str(test_content);
  Token* token1 = (Token*)out_come->content;
  fprintf(stderr,"token1->content:%s\n",token1->content);
  EXPECT_EQ(0,strcmp(token1->content,"This"));
  EXPECT_EQ(token1->type,Token_NORMAL);
  EXPECT_FALSE(out_come->next==NULL);

  Token* token2 = (Token*)out_come->next->content;
  EXPECT_EQ(0,strcmp(token2->content,"is"));
  EXPECT_FALSE(out_come->next->next==NULL);
  EXPECT_EQ(token2->type,Token_NORMAL);

  Token* token3 = (Token*)out_come->next->next->content;
  EXPECT_EQ(0,strcmp(token3->content,"test"));
  EXPECT_FALSE(out_come->next->next->next==NULL);
  EXPECT_EQ(token3->type,Token_NORMAL);

  Token* token4 = (Token*)out_come->next->next->next->content;
  EXPECT_EQ(0,strcmp(token4->content,"string"));
  EXPECT_TRUE(out_come->next->next->next->next==NULL);
  EXPECT_EQ(token4->type,Token_NORMAL);
  //recyclying memory
  do{
    Node* temp = out_come;
    Token* temp_token = (Token*)temp->content;
    destory_token(temp_token);
    out_come = out_come->next;
    destory_node(temp,0);
  }while(out_come!=NULL);
  
}

TEST(Tokenise_str,special_symbals){
  char test_content[] = "| > &";
  Node *out_come = tokenise_str(test_content);

  EXPECT_FALSE(out_come==NULL);
  Token* token1 = (Token*)out_come->content;
  EXPECT_EQ(token1->type,Token_PIPE);
  EXPECT_EQ(0,strcmp(token1->content,"|"));

  EXPECT_FALSE(out_come->next == NULL);
  Token* token2 = (Token*)out_come->next->content;
  EXPECT_EQ(0,strcmp(token2->content,">"));
  EXPECT_EQ(token2->type,Token_REDIRECT);

  EXPECT_FALSE(out_come->next->next == NULL);
  Token* token3 = (Token*)out_come->next->next->content;
  EXPECT_EQ(token3->type,Token_BACKGROUND);
  EXPECT_EQ(0,strcmp(token3->content,"&"));

  //recycley memory 
  destory_token_list(out_come);
}

TEST(Tokenise_str,builtin_commands){
  char test_content[] = "cd exit path";
  Node *out_come = tokenise_str(test_content);
  EXPECT_FALSE(out_come==NULL);
  Token* token1 = (Token*)out_come->content;
  EXPECT_EQ(token1->type,Token_BUILTIN);
  EXPECT_EQ(0,strcmp(token1->content,"cd"));

  EXPECT_FALSE(out_come->next == NULL);
  Token* token2 = (Token*)out_come->next->content;
  EXPECT_EQ(0,strcmp(token2->content,"exit"));
  EXPECT_EQ(token2->type,Token_BUILTIN);

  EXPECT_FALSE(out_come->next->next == NULL);
  Token* token3 = (Token*)out_come->next->next->content;
  EXPECT_EQ(token3->type,Token_BUILTIN);
  EXPECT_EQ(0,strcmp(token3->content,"path"));

  //recycley memory
  destory_token_list(out_come);
}

TEST(Tokenise_str,mixed_string){
  char* test_content = "cd | ls > test.txt &";
  Node *out_come = tokenise_str(test_content);

  EXPECT_FALSE(out_come==NULL);
  Token* token1 = (Token*)node_at(out_come,0)->content;
  EXPECT_EQ(token1->type,Token_BUILTIN);
  EXPECT_EQ(0,strcmp(token1->content,"cd"));

  EXPECT_FALSE(node_at(out_come,1) == NULL);
  Token* token2 = (Token*)node_at(out_come,1)->content;
  EXPECT_EQ(token2->type,Token_PIPE);

  EXPECT_FALSE(node_at(out_come,2) == NULL);
  Token* token3 = (Token*)node_at(out_come,2)->content;
  EXPECT_EQ(token3->type,Token_NORMAL);
  EXPECT_EQ(0,strcmp(token3->content,"ls"));

  EXPECT_FALSE(node_at(out_come,3) == NULL);
  Token* token4 = (Token*)node_at(out_come,3)->content;
  EXPECT_EQ(token4->type,Token_REDIRECT);
  EXPECT_EQ(0,strcmp(token4->content,">"));

  //recycpe memory
  destory_token_list(out_come);
}
