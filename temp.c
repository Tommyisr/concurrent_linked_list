#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include "concurrent_list.h"

struct node {
    int value;
    node* next;
    pthread_mutex_t *mutex;
};

struct list {
    pthread_mutex_t *mutex;
    node* root;
};

void print_node(node* node)
{
    // DO NOT DELETE
    if(node)
    {
        printf("%d ", node->value);
    }
}

unsigned short thread_id(void)
{
    pthread_t self_id = pthread_self();
    return ((unsigned long long)self_id * 2654435761) % 0xFFFF;
}

list* create_list()
{
    srand(time(NULL));
    unsigned short id = thread_id();
    //printf("[%u] create_list()\n", id);
    list* lst = (list*) malloc(sizeof(list));
    lst->root = NULL;
    lst->mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(lst->mutex, NULL);
    // printf("[%u] DEBUG: lst: %p\n", id, lst);
    // printf("[%u] DEBUG: lst->root: %p\n", id, lst->root);
    // printf("[%u] DEBUG: lst->mutex: %p\n", id, lst->mutex);
    return lst;
}

void delete_node(node* node)
{
    unsigned short id = thread_id();
    //printf("[%u] delete_node(%p)\n", id, node);
    pthread_mutex_destroy(node->mutex);
    free(node->mutex);
    free(node);
}

_Noreturn void delete_list(list* list)
{
    /*
     * This call can easily race with another function!
     * assumed this is called in between joins
     */
    unsigned short id = thread_id();
    //printf("[%u] delete_list()\n", id);
    // first lock the list, and empty it out by setting root to NULL.
    pthread_mutex_lock(list->mutex);
    pthread_mutex_t* list_mutex = list->mutex;
    //printf("[%u] delete_list(): free list.\n", id);
    node* p = list->root;
    list->root = NULL;
    free(list);
    pthread_mutex_unlock(list_mutex);
    // this can race on list mutex access
    // but p isn't reachable anymore.
    pthread_mutex_destroy(list_mutex);
    free(list_mutex);
    // free the list chain
    if (p)
    {
        int i = 0;
        pthread_mutex_lock(p->mutex);
        while (p->next != NULL)
        {
            i++;
            node* q = p;
            // pthread_mutex_t* prev_mutex = p->mutex;
            pthread_mutex_lock(p->next->mutex);
            p = p->next;
            q->next = NULL;
            //printf("[%u] delete_list(): free node %d.\n", id, i - 1);
            pthread_mutex_unlock(q->mutex);
            delete_node(q);
        }
        //printf("[%u] delete_list(): free node %d.\n", id, i);
        pthread_mutex_unlock(p->mutex);
        delete_node(p);
    }
}

void insert_value(list* list, int value)
{
    /*
     * Insert a new node to the linked list, keeping the list sorted.
     */
    unsigned short id = thread_id();
    //printf("[%u] insert_value()\n", id);
    // first we create the new node
    node* nod = (node*) malloc(sizeof(node));
    nod->next = NULL;
    nod->value = value;
    nod->mutex = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(nod->mutex, NULL);
    // //printf("[%u] DEBUG: nod: %p\n", id, nod);
    // //printf("[%u] DEBUG: nod->next: %p\n", id, nod->next);
    //printf("[%u] DEBUG: nod->value: %d\n", id, nod->value);
    // //printf("[%u] DEBUG: nod->mutex: %p\n", id, nod->mutex);
    //printf("[%u] insert_value(): trying to acquire list mutex\n", id);
    pthread_mutex_lock(list->mutex);
    //printf("[%u] insert_value(): acuired!\n", id);
    //sleep((float)(rand() % 30) / 100.0f);
    // cases:
    // 1. empty list, set list root.
    // 2. insert new first node, replace list->root.
    // 3. find node to insert after.
    // first lock entire list, then lock root, release list lock,
    // then lock root->next, release root, lock root->next->next, etc...
    // never check, or change list/node attributes before locking it's mutex!!
    node* p = list->root;
    if (!p)
    {
        // list is empty
        //printf("[%u] insert_value(): 1. empty list!\n", id);
        list->root = nod;
        //printf("[%u] insert_value(): releasing list...!\n", id);
        pthread_mutex_unlock(list->mutex);
    } else {
        int i = 0;
        //printf("[%u] insert_value(): trying to acquire node%d(%p) mutex\n", id, i, p);
        pthread_mutex_lock(p->mutex);
        //printf("[%u] insert_value(): node%d acuired!\n", id, i);
        //printf("[%u] insert_value(): releasing list...!\n", id);
        pthread_mutex_unlock(list->mutex);
        if (nod->value <= p->value) {
            // insert new first node
            //printf("[%u] insert_value(): 2. insert new root\n", id);
            nod->next = list->root;
            list->root = nod;
        } else {
            //printf("[%u] insert_value(): 3. searching...\n", id);
            // find node to insert after
            while (p->next != NULL)
            {
                i++;
                //printf("[%u] insert_value(): trying to acquire node%d(%p) mutex\n", id, i, p);
                pthread_mutex_lock(p->next->mutex);
                //printf("[%u] insert_value(): node%d acuired!\n", id, i);
                if (nod->value <= p->next->value)
                {
                    // insertion point found!
                    //printf("[%u] insert_value(): releasing node %d...!\n", id, i);
                    pthread_mutex_unlock(p->next->mutex);
                    break;
                } else {
                    // advence once in the chain, using hand over
                    pthread_mutex_t* prev_mutex = p->mutex;
                    p = p->next;
                    //printf("[%u] insert_value(): releasing node %d...! [hand_over]\n", id, i - 1);
                    pthread_mutex_unlock(prev_mutex);
                }
            }
            //printf("[%u] insert_value(): found! node %d (%p)\n", id, i, p);
            nod->next = p->next;
            p->next = nod;
            // node inserted, safe to release mutex.
        }
        //printf("[%u] insert_value(): releasing node %d...!\n", id, i);
        pthread_mutex_unlock(p->mutex);
    }
}

void remove_value(list* list, int value)
{
    unsigned short id = thread_id();
    //printf("[%u] remove_value()\n", id);
    // cases:
    // 1. empty list, do nothing.
    // 2. remove root.
    // 3. remove node in the middle (or last one).

    // first lock the entire list!
    //printf("[%u] remove_value(): trying to acquire list mutex\n", id);
    pthread_mutex_lock(list->mutex);
    //printf("[%u] remove_value(): acuired!\n", id);
    // check if empty
    node* p = list->root;
    if (!p)
    {
        // list is empty
        //printf("[%u] remove_value(): 1. empty list! releasing list...\n", id);
        pthread_mutex_unlock(list->mutex);
    } else {
        int i = 0;
        //printf("[%u] remove_value(): trying to acquire node%d(%p) mutex\n", id, i, p);
        pthread_mutex_lock(p->mutex);
        //printf("[%u] remove_value(): node%d acuired!\n", id, i);
        if (p->value == value)
        {
            //printf("[%u] remove_value(): 2. root matches!\n", id);
            list->root = p->next;
            //printf("[%u] remove_value(): releasing list\n", id);
            pthread_mutex_unlock(list->mutex);
            //printf("[%u] remove_value(): releasing node %d (about to destroy)\n", id, i);
            pthread_mutex_unlock(p->mutex);
            // can't reach p anymore, safe to destroy
            delete_node(p);
        } else {
            //printf("[%u] remove_value(): releasing list\n", id);
            pthread_mutex_unlock(list->mutex);
            node *q = p->next;
            while (q != NULL)
            {
                i++;
                //printf("[%u] remove_value(): trying to acquire node%d(%p) mutex\n", id, i, q);
                pthread_mutex_lock(q->mutex);
                if (q->value == value)
                {
                    //printf("[%u] remove_value(): 3. found match %d\n", id, i);
                    p->next = q->next;
                    //printf("[%u] remove_value(): releasing node %d\n", id, i - 1);
                    pthread_mutex_unlock(p->mutex);
                    //printf("[%u] remove_value(): releasing node %d (about to destroy)\n", id, i);
                    pthread_mutex_unlock(q->mutex);
                    // can't reach q anymore, safe to destroy
                    delete_node(q);
                    return;
                }
                // addvance one node
                //printf("[%u] remove_value(): releasing node %d\n", id, i - 1);
                pthread_mutex_unlock(p->mutex);
                p = q;
                q = q->next;
            }
            // node not found, release p and return
            //printf("[%u] remove_value(): releasing node %d (node not found)\n", id, i);
            pthread_mutex_unlock(p->mutex);
        }
    }
}

void print_list(list* list)
{
    unsigned short id = thread_id();
    //printf("[%u] print_list()\n", id);
    if (list)
    {
        pthread_mutex_lock(list->mutex);
        node* p = list->root;
        if (!p)
        {
            //printf("<empty>");
            pthread_mutex_unlock(list->mutex);
        } else {
            pthread_mutex_lock(p->mutex);
            pthread_mutex_unlock(list->mutex);
            print_node(p);
            //printf("[%d|-]->", p->value);
            while (p->next != NULL)
            {
                pthread_mutex_t* prev_mutex = p->mutex;
                pthread_mutex_lock(p->next->mutex);
                p = p->next;
                print_node(p);
                pthread_mutex_unlock(prev_mutex);
                //printf("[%d|-]->", p->value);
            }
            pthread_mutex_unlock(p->mutex);
            //printf("NULL");
        }
    }
    printf("\n"); // DO NOT DELETE
}

void count_list(list* list, int (*predicate)(int))
{
    int count = 0; // DO NOT DELETE

    unsigned short id = thread_id();
    //printf("[%u] count_list()\n", id);
    pthread_mutex_lock(list->mutex);
    node* p = list->root;
    if (!p)
    {
        //printf("[%u] <empty>", id);
        pthread_mutex_unlock(list->mutex);
    } else {
        pthread_mutex_lock(p->mutex);
        pthread_mutex_unlock(list->mutex);
        if (predicate(p->value)) {
            count++;
        }
        //printf("[%u] pred(%d), count=%d\n", id, p->value, count);
        while (p->next != NULL)
        {
            pthread_mutex_t* prev_mutex = p->mutex;
            pthread_mutex_lock(p->next->mutex);
            p = p->next;
            pthread_mutex_unlock(prev_mutex);
            if (predicate(p->value)) {
                count++;
            }
            //printf("[%u] pred(%d), count=%d\n", id, p->value, count);
        }
        pthread_mutex_unlock(p->mutex);
    }

    printf("%d items were counted\n", count); // DO NOT DELETE
}
