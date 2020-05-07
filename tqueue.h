//
// Created by simone on 30.03.20.
//

#ifndef UNTITLED2_TQUEUE_H
#define UNTITLED2_TQUEUE_H

struct TQueueNode;
typedef struct TQueueNode* TQueue;

/* Adds a new element at the end of the list, returns its position */
unsigned long int tqueue_enqueue(TQueue* q, char *data);

/* Removes and returns the element at the beginning of the list, NULL if the queue is empty */
void* tqueue_pop(TQueue* q);

/* Returns the number of elements in the list */
unsigned long int tqueue_size(TQueue q);

/* Returns a 'view' on the list starting at (a positive) offset distance, 14. * NULL if the queue is empty */
TQueue tqueue_at_offset(TQueue q, unsigned long int offset);

/* Returns the data on the first node of the given list */
void* tqueue_get_data(TQueue q);

#endif //UNTITLED2_TQUEUE_H
