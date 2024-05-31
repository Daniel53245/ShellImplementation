#ifndef UTILS

#define UTILS
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


typedef struct Node{
  void *content;
  size_t size;
  struct Node* prev;
  struct Node* next;
} Node;

Node *new_node(void* content,size_t content_size);
int append_node(Node* head,void* content,size_t content_siz);
int delete_node(Node* head,Node* node);
Node* node_at(Node* head,int index);
Node* find_node(Node* head,void* content,size_t content_size);
void destory_node(Node* target,int free_content);
void destory_list(Node* head);
char *my_strsep(char **stringp, const char *delim);
void ERROR(int errnum, const char * format, ... );
void destory_token_list(Node* target);
void print_token_list(Node* head);
#endif

