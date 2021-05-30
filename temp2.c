#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"

struct node {
    int value;
    struct node *next;
    pthread_mutex_t m;
};

struct list {
    node *head;
    pthread_mutex_t list_mutex;
};

void print_node(node* node)
{
    // DO NOT DELETE
    if(node)
    {
        printf("%d ", node->value);
    }
}

void free_node(node* node)
{
    pthread_mutex_destroy(&node->m);
    free(node);
}

list* create_list()
{
    list* mylist;
    mylist=(list*)malloc(sizeof(struct list));
    mylist->head = NULL;
    pthread_mutex_init(&mylist->list_mutex,NULL);
    return mylist;
}

_Noreturn void delete_list(list* list)
{
    node* temp;
    if(list->head!=NULL){
        while(list->head!=NULL){
            temp=list->head;
            list->head=list->head->next;
            free_node(temp);
        }
        pthread_mutex_destroy(&list->list_mutex);
        free(list);
    }
}

void insert_value(list* list, int value)
{
    node *newNode, *temp;
    newNode=(node*)malloc(sizeof(struct node));
    newNode->value=value;
    newNode->next=NULL;
    pthread_mutex_init(&(newNode->m),NULL);
    pthread_mutex_lock(&newNode->m);
    temp=list->head;

    //*** if we inseret value to the empty list ***
    if(list->head==NULL){
        pthread_mutex_lock(&list->list_mutex);		//*** lock list to prevent several threads insert firs value to the empty list ***
        list->head=newNode;
        newNode->next=NULL;
        pthread_mutex_unlock(&newNode->m); 			//*** unlock the new node to be avaible to others threads ***
        pthread_mutex_unlock(&list->list_mutex);	//*** unlock list to allow add more items(list is not empty now) ***
        return;
    }

    pthread_mutex_lock(&temp->m);

    //*** If we add a new node to the start of a list ***
    if(list->head->value >= value){
        newNode->next=list->head;
        list->head=newNode;
        pthread_mutex_unlock(&temp->m);
        pthread_mutex_unlock(&newNode->m);

    }
    else{

        //*** Find the relevant place for new node ***
        while(temp->next!=NULL && temp->next->value < value){
            pthread_mutex_lock(&temp->next->m);
            pthread_mutex_unlock(&temp->m);
            temp=temp->next;
        }

        //*** If we want to add item not to the end of a list, we have to lock successor ***
        //*** And we have to check that successor is exist ***
        if(temp->next!=NULL)
            pthread_mutex_lock(&temp->next->m);

        //*** Add a new node by updating the relevant pointers ***
        newNode->next=temp->next;
        temp->next=newNode;

        //*** Unlock successor(if exist), predecessor and new node ***
        pthread_mutex_unlock(&temp->m);
        pthread_mutex_unlock(&newNode->m);
        if(newNode->next!=NULL);
        pthread_mutex_unlock(&temp->next->m);

    }
}

void remove_value(list* list, int value)
{
    node *temp, *candidate;
    temp=list->head;

    //*** if the list is empty ***
    if(list->head==NULL){
        return;
    }


    pthread_mutex_lock(&temp->m);

    //*** if we delete from the start of the list ***
    if(list->head->value == value){
        list->head = list->head->next;
        free_node(temp);
    }

    else{
        while(temp->next !=NULL && temp->next->value != value){
            pthread_mutex_lock(&temp->next->m);
            pthread_mutex_unlock(&temp->m);
            temp=temp->next;
        }
        //*** If value does not exist in the list do nothing***
        if(temp->next==NULL)
            return;

        pthread_mutex_lock(&temp->next->m);
        candidate=temp->next;
        temp->next=candidate->next;
        free_node(candidate);
        pthread_mutex_unlock(&temp->m);
    }
}

void print_list(list* list)
{
    node *temp;

    if(list!=NULL && list->head!=NULL){
        temp=list->head;
        pthread_mutex_lock(&temp->m);
        while(temp->next!=NULL){
            pthread_mutex_lock(&temp->next->m);
            print_node(temp);
            pthread_mutex_unlock(&temp->m);
            temp=temp->next;
        }
        print_node(temp);
        pthread_mutex_unlock(&temp->m);
    }

    printf("\n"); // DO NOT DELETE
}

void count_list(list* list, int (*predicate)(int))
{
    int count = 0; // DO NOT DELETE
    node *temp;
    if(list->head!=NULL){
        temp=list->head;
        pthread_mutex_lock(&temp->m);
        while(temp->next!=NULL){
            pthread_mutex_lock(&temp->next->m);
            count+=predicate(temp->value);
            pthread_mutex_unlock(&temp->m);
            temp=temp->next;
        }
        count+=predicate(temp->value);
        pthread_mutex_unlock(&temp->m);
    }

    printf("%d items were counted\n", count); // DO NOT DELETE
}
