//
// Created by simone on 30.03.20.
//

#ifndef UNTITLED2_BTHREAD_H
#define UNTITLED2_BTHREAD_H
typedef unsigned long int bthread_t;

typedef struct {
} bthread_attr_t;

typedef void *(*bthread_routine) (void *);

int bthread_create(bthread_t *bthread,
                   const bthread_attr_t *attr,
                   void *(*start_routine) (void *),
                   void *arg);
int bthread_join(bthread_t bthread, void **retval);
void bthread_yield();
void bthread_exit(void *retval);
void bthread_sleep(double ms);
void bthread_cancel(bthread_t bthread);
void bthread_testcancel();
void bthread_block_timer_signal();
void bthread_unblock_timer_signal();
#endif //UNTITLED2_BTHREAD_H
