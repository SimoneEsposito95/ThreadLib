#include <stdio.h>
#include "test_bthread.h"
#include "test_tqueue.h"

int main() {
    // tqueue tests
    fprintf(stdout, "\nSTART tqueue tests\n");
    //test_tqueue_null();
    //test_tqueue_one_element();
    //test_tqueue_many_elements();

    // bthread tests
    fprintf(stdout, "\nSTART bthread tests\n");
    test_bthread_create();
}