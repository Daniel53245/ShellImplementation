#include <gtest/gtest.h>
#include <string.h>

extern "C"{
        #include "../src/utils.h"
}


TEST(Node_list_test,creation){

  char* test_string = "This is test string";
  size_t len =strlen(test_string)+1;
  //fprintf(stderr,"call new_node");
  Node *test_node = new_node(test_string,len);
  //fprintf(stderr,"end new_node\n");
  if (test_node == NULL)
  {
    printf("test_node is NULL\n");
    EXPECT_FALSE(test_node==NULL);
    //g test raise problem
  }
  EXPECT_EQ(test_node->size,len);
  //corepare the content of the new_node
  EXPECT_EQ(strcmp((char*)test_node->content,test_string),0); 
  //free the list
  destory_list(test_node);
}

TEST(Node_list_test,find_content){
  //test contents
  char* test_string = "This is test string1";
  char* test_string2 = "This is test string2";
  char* test_string3 = "This is test string3";
  size_t len = size_t(strlen(test_string))+1;
  //create a new node
  Node *test_node = new_node(test_string,len);
  append_node(test_node,test_string2,len);
  append_node(test_node,test_string3,len);
  //find the node
  Node *found_node = find_node(test_node,test_string2,len);
  EXPECT_EQ(strcmp((char*)found_node->content,test_string2),0);
}

TEST(Node_list_test,delete_node){
  //test contents
  char* test_string = "This is test string1";
  char* test_string2 = "This is test string2";
  char* test_string3 = "This is test string3";
  size_t len = strlen(test_string)+1;
  //create a new node
  Node *test_node = new_node(test_string,len);
  append_node(test_node,test_string2,len);
  append_node(test_node,test_string3,len);
  //find the node
  Node *found_node = find_node(test_node,test_string2,len);
  //delete the node
  delete_node(test_node,found_node);
  //find the node again
  found_node = find_node(test_node,test_string2,len);
  EXPECT_TRUE(found_node==NULL);
  //free the list
  destory_list(test_node);
}

TEST(Node_list_test,append_node){

  //test contents
  char* test_string = "This is test string1";
  char* test_string2 = "This is test string2";
  char* test_string3 = "This is test string3";
  size_t len = (strlen(test_string)+1);
  //create a new node
  Node *test_node = new_node(test_string,len);
  append_node(test_node,test_string2,len);
  append_node(test_node,test_string3,len);
  //find the node
  Node *found_node = find_node(test_node,test_string2,len);
  EXPECT_EQ(strcmp((char*)found_node->content,test_string2),0);
  //free the list
  destory_list(test_node);
}

TEST(Node_list_test ,node_at){
  //test contents
  char* test_string = "This is test string1";
  char* test_string2 = "This is test string2";
  char* test_string3 = "This is test string3";
  size_t len = strlen(test_string)+1;
  //create a new node
  Node *test_node = new_node(test_string,len);
  append_node(test_node,test_string2,len);
  append_node(test_node,test_string3,len);
  //find the node
  Node *found_node = node_at(test_node,1);
  EXPECT_EQ(strcmp((char*)found_node->content,test_string2),0);
  //free the list
  destory_list(test_node);
}
