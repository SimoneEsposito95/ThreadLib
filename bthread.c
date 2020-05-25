//
// Created by simone on 21.04.20.
//

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <sys/time.h>
#include <stdbool.h>
#include <signal.h>
#include "bthread.h"
#include "bthread_private.h"

#define STACK_SIZE 2000000
#define save_context(CONTEXT) sigsetjmp(CONTEXT, 1)
#define restore_context(CONTEXT) siglongjmp(CONTEXT, 1)
#define QUANTUM_USEC 1000


void bthread_printf(const char* format, ...)
{
    bthread_block_timer_signal();
    va_list args;
    va_start (args, format);
    vprintf (format, args);
    va_end (args);
    bthread_unblock_timer_signal();
}

static void bthread_setup_timer()
{
    static bool initialized = false;
    if (!initialized) {
        bthread_block_timer_signal();
        signal(SIGVTALRM, (void (*)()) bthread_yield);
        struct itimerval time;
        time.it_interval.tv_sec = 0;
        time.it_interval.tv_usec = QUANTUM_USEC;
        time.it_value.tv_sec = 0;
        time.it_value.tv_usec = QUANTUM_USEC;
        initialized = true;
        setitimer(ITIMER_VIRTUAL, &time, NULL);
    }
}



double get_current_time_millis()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000 ;
}

__bthread_scheduler_private* bthread_get_scheduler(){
    static __bthread_scheduler_private* instance = NULL;

    if (instance == NULL)
    {
        instance =  malloc(sizeof(__bthread_scheduler_private));
        instance->queue = NULL;
        instance->current_item = NULL;
        instance->current_tid = 0;
        instance->scheduling_routine = __POLICY_PRIORITY;
        instance->quantum_counter = 0;
    }

    return instance;
}

/* Creates a new thread structure and puts it at the end of the queue.
 * The thread identifier (stored in the buffer pointed by bthread) corresponds to the position in the queue.
 * The thread is not started when calling this function.
 * Attributes passed through the attr argument are ignored (thus it is possible to pass a NULL pointer).
 * The stack pointer for new created threads is NULL */
int bthread_create(bthread_t *bthread,
                   const bthread_attr_t *attr,
                   void *(*start_routine) (void *),
                   void *arg){
    __bthread_private* newThread = malloc(sizeof(__bthread_private));
    newThread->tid = tqueue_enqueue(&bthread_get_scheduler()->queue,newThread);
    newThread->body = start_routine;
    newThread->arg = arg;
    newThread->state=__BTHREAD_READY;
    newThread->attr=*attr;
    newThread->stack = NULL;
    newThread->cancel_req = 0;
    newThread->priority = 0;
    *bthread = newThread->tid;
    return 0;
}

int bthread_create_priority(bthread_t *bthread,
                   const bthread_attr_t *attr,
                   void *(*start_routine) (void *),
                   void *arg,
                   int priority){
    __bthread_private* newThread = malloc(sizeof(__bthread_private));
    newThread->tid = tqueue_enqueue(&bthread_get_scheduler()->queue,newThread);
    newThread->body = start_routine;
    newThread->arg = arg;
    newThread->state=__BTHREAD_READY;
    newThread->attr=*attr;
    newThread->stack = NULL;
    newThread->cancel_req = 0;
    newThread->priority = priority;
    *bthread = newThread->tid;
    return 0;
}

int bthread_join(bthread_t bthread, void **retval) {
    bthread_setup_timer(); //aggiunta per preemption

    volatile __bthread_scheduler_private *scheduler = bthread_get_scheduler();
    scheduler->current_item = scheduler->queue;

    save_context(scheduler->context);

    if (bthread_check_if_zombie(bthread, retval))
        return 0;
    __bthread_private *tp;

    do {
        bthread_set_current_policy();
        //scheduler->current_item = tqueue_at_offset(scheduler->current_item, 1);
        tp = (__bthread_private *) tqueue_get_data(scheduler->current_item);
        if(tp->state == __BTHREAD_SLEEPING && tp->wake_up_time < get_current_time_millis())
            tp->state = __BTHREAD_READY;
    } while (tp->state != __BTHREAD_READY);


    if (tp->stack) {
        restore_context(tp->context);
    } else {
        tp->stack = (char *) malloc(sizeof(char) * STACK_SIZE);
        unsigned long target = tp->stack + STACK_SIZE - 1;
#if __APPLE__
        // OSX requires 16 bytes aligned stack
    target -= (target % 16);
#endif
#if __x86_64__
        asm __volatile__("movq %0, %%rsp"::"r"((intptr_t) target));
#else
        asm __volatile__("movl %0, %%esp" :: "r"((intptr_t) target));
#endif
        bthread_unblock_timer_signal();
        bthread_exit(tp->body(tp->arg));

    }
}

void bthread_yield(){
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);

    volatile int i = save_context(tp->context);


    if (!i) {
        fprintf(stdout, "YIELD: tid: %lu  state: %d\n", tp->tid, tp->state);
        restore_context(scheduler->context);
    }
    bthread_unblock_timer_signal();
}

void bthread_exit(void *retval){
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    tp->retval = retval;
    tp->state = __BTHREAD_ZOMBIE;
    restore_context(scheduler->context);
}

static int bthread_check_if_zombie(bthread_t bthread, void **retval){
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = (__bthread_private*) tqueue_get_data(bthread_get_queue_at(bthread));
    if(tp->state != __BTHREAD_ZOMBIE){
        return 0;
    }
    if(retval != NULL){ //sbagliato: retval parametro
        *retval = tp->retval;
    }
    free(tp->stack);
    //tqueue_pop((bthread_get_queue_at(bthread));
    scheduler->current_item = scheduler->queue;
    return 1;
}

static TQueue bthread_get_queue_at(bthread_t bthread){
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    TQueue tQueue = scheduler->current_item;
    for (int i = 0; i <tqueue_size(tQueue); i++) {
        __bthread_private* dum = tqueue_get_data(tQueue);
        if(dum != NULL){
            if(dum->tid == bthread)
                return tQueue;
            tQueue = tqueue_at_offset(tQueue,1);
        }
    }
    return NULL;
}

void bthread_sleep(double ms){
    bthread_block_timer_signal();
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    tp->wake_up_time = get_current_time_millis()+ms;
    tp->state = __BTHREAD_SLEEPING;

    volatile int i = save_context(tp->context);

    if (!i) {
        fprintf(stdout, "SLEEP: tid: %lu  state: %d\n", tp->tid, tp->state);
        restore_context(scheduler->context);
    }
    bthread_unblock_timer_signal();
}

void bthread_cancel(bthread_t bthread){
    TQueue tq = bthread_get_queue_at(bthread);
    __bthread_private* tp = (__bthread_private*) tqueue_get_data(tq);
    tp->cancel_req = 1;
}

void bthread_testcancel(){
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    __bthread_private* tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);
    if(tp->cancel_req){
        bthread_exit((void*)-1);
    }
}


void bthread_block_timer_signal(){
    sigset_t x;
    sigemptyset (&x);
    sigaddset(&x, SIGVTALRM);
    sigprocmask(SIG_BLOCK, &x, NULL);
}
void bthread_unblock_timer_signal(){
    sigset_t x;
    sigemptyset (&x);
    sigaddset(&x, SIGVTALRM);
    sigprocmask(SIG_UNBLOCK, &x, NULL);
}


void bthread_set_current_policy(){
    volatile __bthread_scheduler_private* scheduler = bthread_get_scheduler();
    if(scheduler->scheduling_routine == __POLICY_RANDOM) {
        ulong size = tqueue_size(scheduler->current_item);
        int next_random = rand() % size;
        scheduler->current_item = tqueue_at_offset(scheduler->current_item, next_random);
        fprintf(stdout, "RANDOM: %d\n", next_random);
    }else if(scheduler->scheduling_routine == __POLICY_PRIORITY){
        __bthread_private* tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);
        if(scheduler->quantum_counter > 0 /*&& tp->state == __BTHREAD_READY*/){
            scheduler->quantum_counter--;
        } else{
            scheduler->current_item = tqueue_at_offset(scheduler->current_item, 1);
            __bthread_private* tp = (__bthread_private*) tqueue_get_data(scheduler->current_item);
            scheduler->quantum_counter = tp->priority;
        }
    }else{
        scheduler->current_item = tqueue_at_offset(scheduler->current_item, 1);
    }
}