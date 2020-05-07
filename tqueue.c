//
// Created by simone on 30.03.20.
//

#include <stddef.h>
#include "tqueue.h"
#include <stdlib.h>

typedef struct TQueueNode {
    struct TQueueNode* next;
    void* data;
} TQueueNode;


unsigned long int tqueue_enqueue(TQueue* q, char *data){
    unsigned long int index = 0;
    TQueueNode* temp;
    if(q==NULL){
        return 0;
    }

    if(*q == NULL){
        temp = malloc(sizeof(TQueueNode));
        temp->data = data;
        temp->next = temp;
        *q = temp;
        return index;
    }

    temp = *q;
    while(temp->next != *q){
        temp = temp->next;
        index++;
    }

    index++;
    TQueueNode* newElement =  malloc(sizeof(TQueueNode));
    newElement->next = *q;
    newElement->data = data;

    temp->next = newElement;

    return index;
}

void* tqueue_pop(TQueue* q){
    if(q == NULL){
        return NULL;
    }
    void* data = (*q)->data;

    TQueueNode* temp = *q;
    if(temp == temp->next){ //case only 1 element
        *q = NULL;
        return data;
    }

    TQueueNode* second = temp->next;
    while(temp->next != *q){
        temp = temp->next;
    }

    temp->next = second;
    *q = second;

    return data;

}


unsigned long int tqueue_size(TQueue q){
    long l = 0;

    if(q == NULL){
        return NULL;
    }

    TQueueNode* temp = q;

    do{
        l = l+1;
        temp = temp->next;
    }
    while(temp != q);

    return l;
}

TQueue tqueue_at_offset(TQueue q, unsigned long int offset){
    if(q == NULL){
        return NULL;
    }

    TQueueNode* temp = q;
    for (int i = 0; i < offset; ++i) {
        temp = temp->next;
    }
    return temp;
}

void* tqueue_get_data(TQueue q){
    if(q == NULL){
        return NULL;
    }

    return (void *)q->data;
}