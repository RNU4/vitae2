#ifndef DOUBLE_LINKED_LIST_H_   /* Include guard */
#define DOUBLE_LINKED_LIST_H_
#include <stdio.h>
#include <stdlib.h>


typedef struct  node{
    void* value;
    struct node* next;
    struct node* prev;
    /* data */
}node_t;


node_t* create_new_node(void* value){
    node_t *result = (node_t*)malloc(sizeof(node_t));
    result->value=value;
    result->next=NULL;
    result->prev=NULL;
    return result;
}

node_t* insert_at_head(node_t** head,node_t* node_to_insert){
    node_to_insert->next = *head;
    if (*head != NULL){
        (*head)->prev = node_to_insert;
    }
    *head = node_to_insert;
    node_to_insert->prev = NULL;
    return node_to_insert;
}

node_t* find_node(node_t*head, void* value){
    node_t*tmp=head;
    while(tmp!=NULL){
        if (tmp->value == value) return tmp;
        tmp=tmp->next;

    }
    return NULL;
}

node_t* find_node_index(node_t*head, int index){
    node_t*tmp=head;
    int i = 0;
    while(tmp!=NULL){
        if (i == index) return tmp;
        tmp=tmp->next;
        i++;
    }
    return NULL;
}

void insert_after_node(node_t *node_to_insert_after,node_t*newnode){
    newnode->next=node_to_insert_after->next;
    if (newnode->next != NULL){
        newnode->next->prev = node_to_insert_after;
    }
    newnode->prev = node_to_insert_after;
    node_to_insert_after->next=newnode;
}

void remove_node(node_t** head, node_t* node_to_remove){
    if (*head == node_to_remove){
        
        
        *head=node_to_remove->next;
        
        if (*head != NULL){
            (*head)->prev = NULL;
        }
       // if (*head == node_to_remove->next) node_to_remove->next = NULL; //break loop
       // printf("awdwadawdawdadwwad");
        return;
    }else{
        node_to_remove->prev->next = node_to_remove->next;
        if (node_to_remove->next != NULL){
            node_to_remove->next->prev = node_to_remove->prev;
        }
        node_to_remove->next = NULL;
        node_to_remove->prev = NULL;
    }
    //free(node_to_remove);
    return;
}


#endif