/*
NAME: Ram Goli
ID: 604751659
EMAIL: ramsgoli@gmail.com
*/

#include <stddef.h>
#include <sched.h>
#include <stdio.h>
#include "SortedList.h"

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
    SortedListElement_t *current = list->next;
    while (current->key != NULL && *(current->key) < *(element->key)) {
        if(opt_yield & INSERT_YIELD){
            sched_yield();
        }
        current = current->next;
    }
    element->next = current;
    element->prev = current->prev;
    current->prev = element;
    (element->prev)->next = element;
}

int SortedList_delete(SortedListElement_t *element) {

    if ((element->prev)->next == NULL || (element->next)->prev == NULL) {
        return 1;
    }

    (element->prev)->next = element->next;
    if (opt_yield & DELETE_YIELD) {
        sched_yield();
    }
    (element->next)->prev = element->prev;
    return 0;
}

SortedListElement_t * SortedList_lookup(SortedList_t *list, const char *key) {
    SortedListElement_t *current = list->next;
    while(current->key != key) {
        if (current->key == NULL) {
            return NULL;
        }
        if (opt_yield & LOOKUP_YIELD) {
            sched_yield();	
        } 
        current = current->next;
    }
    return current;
}

int SortedList_length(SortedList_t *list) {
    SortedListElement_t *current = list->next;

    int count;
    for (count = 0; current->key != NULL; count++) {
        if (opt_yield & LOOKUP_YIELD) {
            sched_yield();
        }
        if (current->next == NULL || (current->next)->prev != current) {
            return -1;
        }
        if (current->prev == NULL || (current->prev)->next != current) {
            return -1;		
        }
        current = current->next;
    }
    return count;
}