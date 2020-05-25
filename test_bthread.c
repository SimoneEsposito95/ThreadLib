#include <stdio.h>
#include <stdlib.h>
#include "bthread.h"

void* my_routine(void* param) {
    int loops = (int)param;
    int i=0;

    int l = 0;
    bthread_sleep(100);
    for(;l<10000;l++){
        fprintf(stdout, "thread [%d]  -> TEST PRE\n", loops);
    }

    for (; i < 3; ++i) {
        bthread_testcancel();
        bthread_sleep(500);
        fprintf(stdout, "thread [%d]  -> %d\n", loops, i);
        bthread_yield();
    }

    // stop first thread
    bthread_cancel(0);

    return (void*)i;
}


void test_bthread_create() {
    fprintf(stdout, "** test_bthread_create **\n");

    bthread_t tid[3];
    for (int i = 0; i < 3; ++i) {
        bthread_create_priority(&tid[i], NULL, my_routine, (void*)i, i);
        fprintf(stdout, "%i) thread %d created\n", i, tid[i]);
    }

    for (int i = 0; i < 3; ++i) {
        int retval = -1;
        fprintf(stdout, "bthread_join: %d\n", tid[i]);
        bthread_join(tid[i], (void**)&retval);
    }

    fprintf(stdout, ": PASSED\n");
}
