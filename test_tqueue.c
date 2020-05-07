//
// Created by simone on 27.04.20.
//

#include <stdio.h>
#include "tqueue.h"

typedef struct {
    char str[10];
    int len;
} data_t;

int test_data(data_t data[], long int data_sz);

void test_tqueue_null() {
    fprintf(stdout, "test_tqueue_null ");
    tqueue_get_data(NULL);
    tqueue_pop(NULL);
    tqueue_size(NULL);
    tqueue_enqueue(NULL, NULL);
    tqueue_at_offset(NULL, 0);
    fprintf(stdout, ": PASSED\n");
}

void test_tqueue_many_elements() {
    fprintf(stdout, "test_tqueue_many_elements ");
    data_t data[] = {
            {"ciao", 1},
            {"pippo", 2},
            {"urchi", 3}
    };
    long int data_sz = sizeof(data) / sizeof(data[0]);
    test_data(data, data_sz);
}

void test_tqueue_one_element() {
    fprintf(stdout, "test_tqueue_one_element ");
    data_t data[] = {"ciao", 10};
    long int data_sz = sizeof(data) / sizeof(data[0]);
    test_data(data, data_sz);
}

int test_data(data_t data[], long int data_sz) {
    TQueue q = NULL;
    for (int i = 0; i < data_sz; ++i) {
        tqueue_enqueue(&q, &data[i]);
    }

    unsigned long int queue_sz = tqueue_size(q);
    if (queue_sz != data_sz) {
        fprintf(stderr, "FAILED: wrong queue size. expected %ld actual %ld\n", data_sz, queue_sz);
        return (1);
    }

    for (int i = 0; i < data_sz; ++i) {
        void *val = tqueue_pop(&q);
        if (((data_t *) val) != &data[i]) {
            fprintf(stderr, "FAILED: wrong return data from pop\n");
            return (1);
        }
    }

    queue_sz = tqueue_size(q);
    if (queue_sz != 0) {
        fprintf(stderr, "FAILED: wrong queue size. expected 0 actual %ld\n", queue_sz);
        return (1);
    }

    fprintf(stdout, ": PASSED\n");
    return 0;
}