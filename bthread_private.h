//
// Created by simone on 30.03.20.
//

#include "bthread.h"
#include "tqueue.h"
#include <setjmp.h>

#ifndef UNTITLED2_BTHREAD_PRIVATE_H

typedef enum { __BTHREAD_READY = 0, __BTHREAD_BLOCKED, __BTHREAD_SLEEPING, __BTHREAD_ZOMBIE
} bthread_state;


typedef struct {
    bthread_t tid;
    bthread_routine body;
    void* arg;
    bthread_state state;
    bthread_attr_t attr;
    char* stack;
    void* retval;
    double wake_up_time;
    int cancel_req;
    int priority;
    jmp_buf context;
} __bthread_private;

typedef struct {
    TQueue queue;
    TQueue current_item;
    jmp_buf context;
    bthread_t current_tid;
    uint  quantum_counter;
    bthread_scheduling_routine scheduling_routine;
} __bthread_scheduler_private;

static int bthread_check_if_zombie(bthread_t bthread, void **retval);
static TQueue bthread_get_queue_at(bthread_t bthread);
void bthread_cleanup(); // Private

void bthread_set_current_policy();

#endif //UNTITLED2_BTHREAD_PRIVATE_H
