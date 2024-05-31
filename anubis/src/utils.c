#include <stdlib.h>
#include "utils.h"
#include "tokeniser.h"
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
//_DEBUT setting for error function
int _DEBUG = 0;



/**
 * @brief new_node create a new node with correspinding content
 * @param content the content of the node
 * @param content_size the size of the content
 * @return pinter to new node NULL on failure
 * The return memory address is mantained by the malloc
*/
Node *new_node(void* content,size_t content_size){
        Node *node = malloc(sizeof(Node));
        if(node == NULL){//filer in malloc
                return NULL;
        }
        node -> content = malloc(content_size);
        if (node->content == NULL){
                free(node);
                return NULL;
        }
        memcpy(node->content,content,content_size);
        node->size = content_size;
        node -> next = NULL;
        node -> prev = NULL;
        return node;
}

/**
 * @brief append_node appedn a node wit correspinding conten
 * into the list
 * call the new_node function to create a new node
 * @param head the head of the list
 * @param content the content of the node
 * @param content_size the size of the content
 * @return 0 on success -1 on failure
*/
int append_node(Node* head,void* content,size_t content_siz){
        Node* current = head;
        while(current->next != NULL){
        current = current->next;
        }
        current->next = new_node(content,content_siz);
        if (current->next == NULL)
        {
                ERROR(errno,"append_node: failed to append node");
                return -1;
        }
        current->next->prev = current;
        return 0;
}

/**
 * @brief delete_node delete a node from the list
 * @param head the head of the list
 * @param node the node to be deleted
*/
int delete_node(Node* head,Node* node){
        if (node == NULL)
        {
                return -1;
        }
        if (node->prev != NULL)
        {
                node->prev->next = node->next;
        }
        if (node->next != NULL)
        {
                node->next->prev = node->prev;
        }
        free(node->content);
        free(node);
        return 0;
}
/**
 * @brief find find a node with correspinding content
 * @param content the content of the node
 * @param content_size the size of the content
 * @return pointer to the node NULL on failure
*/
Node* find_node(Node* head,void* content,size_t content_size){
        Node* current = head;
        while(current != NULL){
                if (memcmp(current->content,content,content_size) == 0)
                {
                        return current;
                }
                current = current->next;
        }
        return NULL;
}

void destory_node(Node* target,int free_content){
        if (target == NULL)
        {
                return;
        }
        if(free_content == 1){
                free(target->content);
        }
        free(target);
        return;
}

/**
 * @brief destory_list destory the list
 * @param head the head of the list
 * frees all the memory allocated to the node
*/
void destory_list(Node* head){
        Node* current = head;
        while(current != NULL){
                Node* next = current->next;
                free(current->content);
                free(current);
                current = next;
        }
        return ;
}


/**
 * @brief strsep see https://codebrowser.dev/glibc/glibc/string/strsep.c.html
 * 
 * @param stringp 
 * @param delim 
 * @return char* 
 */
char *my_strsep (char **stringp, const char *delim)
{
  char *begin, *end;
 
  begin = *stringp;
  if (begin == NULL)
    return NULL;
 
  /* Find the end of the token.  */
  end = begin + strcspn (begin, delim);
 
  if (*end)
    {
      /* Terminate the token and set *STRINGP past NUL character.  */
      *end++ = '\0';
      *stringp = end;
    }
  else
    /* No more delimiters; this is the last token.  */
    *stringp = NULL;
 
  return begin;
}
//above code is from external source


/**
 * @brief ERROR function provided in the assignment
 * @param errnum error number
 * @param format format string
*/



void ERROR(int errnum, const char * format, ... )
{
    if(_DEBUG) {
        va_list args;
        va_start (args, format);
        vfprintf(stderr, format, args);
        va_end (args);
        if(errnum > 0) fprintf(stderr, " : %s", strerror(errnum));
        fprintf(stderr, "\n"); 
        return; 
    }

    fprintf(stderr, "An error has occurred\n"); 
}

/**
 * @brief destory_token_list destory list of node containing Token
 * @param target the list to be destoryed
 * depricated, shoudl be used for testing only
*/
void destory_token_list(Node* target){
        do{
                Node* temp = target;
                Token* temp_token = (Token*)temp->content;
                destory_token(temp_token);
                target = target->next;
                destory_node(temp,0);
        }while(target!=NULL);
}

/**
 * @brief find the node at ceartin position
 * @param head the head of the list
 * @param index the index of the node
 * @return pointer to the node NULL on failure
 * 
*/
Node* node_at(Node* head,int index){
        Node* current = head;
        if(index == 0){
                return head;
        }
        for(int i = 0; i < index; i++){
                if(current == NULL){
                        return NULL;
                }
                current = current->next;
        }
        return current;
}

void print_token_list(Node* head){
        Node* current = head;
        while(current != NULL){
                Token* temp_token = (Token*)current->content;
                fprintf(stderr,"> Token: %s %i -",temp_token->content,temp_token->type);
                current = current->next;
        }
        fprintf(stderr,"\n");
        return;
}