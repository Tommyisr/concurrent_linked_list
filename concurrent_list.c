#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "concurrent_list.h"

struct node {
  int value;
  node* next;
  pthread_mutex_t* m;
  // add more fields
};

struct list {
    node* head;
    pthread_mutex_t* m;
};

void print_node(node* node)
{
  // DO NOT DELETE
  if(node)
  {
    printf("%d ", node->value);
  }
}

list* create_list()
{
  // add code here
  list* L = malloc(sizeof(list));
  L->head = NULL;
  L->m = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(L->m,NULL);
  return L;
}

void delete_node(node* node)
{

    pthread_mutex_destroy(node->m);
    free(node->m);
    free(node);
}

node* create_node(int value){

    node* tmp = malloc(sizeof(node));
    tmp->next = NULL;
    tmp->value = value;
    tmp->m = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(tmp->m,NULL);
    return tmp;

}

 void delete_list(list* list) {


    if (list) {

        pthread_mutex_lock(list->m);
        node *prev = list->head;

        // check that list isn't empty
        if (!prev) {
            pthread_mutex_unlock(list->m);
            pthread_mutex_destroy(list->m);
            free(list->m);
            free(list);
        } else {
            node *curr = prev->next;
            pthread_mutex_lock(prev->m);
            if (curr) pthread_mutex_lock(curr->m);

            while (curr) {

                pthread_mutex_unlock(prev->m);
                delete_node(prev);
                prev = curr;
                curr = curr->next;
                if (curr) pthread_mutex_lock(curr->m);

            }
            pthread_mutex_unlock(prev->m);
            delete_node(prev);

            pthread_mutex_unlock(list->m);
            pthread_mutex_destroy(list->m);
            free(list->m);
            free(list);
        }

    }

}






void insert_value(list* list, int value)
{

    if(!list) return;

    pthread_mutex_lock(list->m);

    node* prev = list->head;

   if(!prev) {
       list->head = create_node(value);
       pthread_mutex_unlock(list->m);
       return;

   }

   pthread_mutex_lock(prev->m);

    if(prev->value > value){
        list->head = create_node(value);
        list->head->next = prev;
        pthread_mutex_unlock(list->m);
        pthread_mutex_unlock(prev->m);
        return;

    }

    pthread_mutex_unlock(list->m);

    node* curr = prev->next;
   if(curr) pthread_mutex_lock(curr->m);


    while(curr && curr->value <= value){
        pthread_mutex_unlock(prev->m);
        prev = curr;
        curr = prev->next;
        if(curr) pthread_mutex_lock(curr->m);

    }

    prev->next = create_node(value);
    prev->next->next = curr;
    pthread_mutex_unlock(prev->m);
    if(curr) pthread_mutex_unlock(curr->m);




}


void remove_value(list* list, int value)
{
    if(!list) return;

    pthread_mutex_lock(list->m);

    node* prev = list->head;
    if(!prev) {
        pthread_mutex_unlock(list->m);
        return;
    }
    pthread_mutex_lock(prev->m);
    node* curr = prev->next;

    if(prev->value == value){
        list->head = curr;
        pthread_mutex_unlock(prev->m);
        delete_node(prev);
        pthread_mutex_unlock(list->m);
        return;

    }

    pthread_mutex_unlock(list->m);
    if(curr) pthread_mutex_lock(curr->m);

    while(curr){

        if(curr->value == value){
            prev->next = curr->next;
            pthread_mutex_unlock(curr->m);
            delete_node(curr);
            pthread_mutex_unlock(prev->m);
            return;
        }

        pthread_mutex_unlock(prev->m);
        prev = curr;
        curr = curr->next;
        if(curr) pthread_mutex_lock(curr->m);

    }
    pthread_mutex_unlock(prev->m);



}




void print_list(list* list)
{
    if(!list) return;
    pthread_mutex_lock(list->m);

    node* prev = list->head;
    if(!prev) {
        pthread_mutex_unlock(list->m);
        return;
    }

    pthread_mutex_lock(prev->m);
    pthread_mutex_unlock(list->m);

    node* curr = prev->next;
    if(curr)   pthread_mutex_lock(curr->m);

    printf("%d, ", prev->value);

    while (curr){
      printf("%d, ", curr->value);
        pthread_mutex_unlock(prev->m);

        prev = curr;
        curr = curr->next;
        if(curr) pthread_mutex_lock(curr->m);
    }

    pthread_mutex_unlock(prev->m);

  printf("\n"); // DO NOT DELETE
}

void count_list(list* list, int (*predicate)(int))
{
    if(!list) return;

  int count = 0; // DO NOT DELETE

  pthread_mutex_lock(list->m);
  node* prev = list->head;
    if(!prev) {
        pthread_mutex_unlock(list->m);
        return;
    }

    pthread_mutex_lock(prev->m);
    pthread_mutex_unlock(list->m);

    if(predicate(prev->value)) count++;

    node* curr = prev->next;
    if(curr)     pthread_mutex_lock(curr->m);


  while (curr){
      if(predicate(curr->value)) count++;
      pthread_mutex_unlock(prev->m);
      prev = curr;
      curr = curr->next;
      if(curr) pthread_mutex_lock(curr->m);

  }
    pthread_mutex_unlock(prev->m);


  printf("%d items were counted\n", count); // DO NOT DELETE
}

