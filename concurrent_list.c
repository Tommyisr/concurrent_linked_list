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
  L->m = malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(L->m,NULL);
  return L;
}

void delete_list(list* list)
{
    node* temp = list->head;
    while (temp){
        list->head = temp->next;
        free(temp);
        temp = list->head;
    }
    free(list);

  // add code here
}

void insert_value(list* list, int value)
{
    node* ptr = list->head;
   if(ptr == NULL) {
       list->head = malloc(sizeof(node));
       list->head->value = value;
       list->head->next = NULL;
       return;

   } else if(ptr->value > value){
       list->head = malloc(sizeof(node));
       list->head->value = value;
       list->head->next = ptr;
       return;
   }

       else {
           while (ptr->next && (ptr->next->value <= value)){
           ptr = ptr->next;
   }
       }

    node* temp = ptr->next;

   ptr->next = malloc(sizeof(node));
   ptr->next->value = value;
   ptr->next->next = temp;



}


void remove_value(list* list, int value)
{
    node* ptr = list->head;

    if(!ptr) {
        return;
    }
  if(ptr->value == value){
      node* temp = ptr->next;
      free(ptr);
      list->head = temp;
      return;
  } else while (ptr->next && ptr->next->value != value){
      ptr = ptr->next;
  }

  if(ptr->next){
  node* temp = ptr->next;
  ptr->next = temp->next;
  free(temp);
  }
}

void print_list(list* list)
{
  node* ptr = list->head;
  while (ptr){
      printf("%d, ", ptr->value);
      ptr = ptr->next;
  }

  printf("\n"); // DO NOT DELETE
}

void count_list(list* list, int (*predicate)(int))
{
  int count = 0; // DO NOT DELETE
  node* ptr = list->head;
  while (ptr){
      if(predicate(ptr->value)) count++;
      ptr = ptr->next;
  }

  // add code here

  printf("%d items were counted\n", count); // DO NOT DELETE
}

