cmake_minimum_required(VERSION 3.15)
project(untitled2 C)

set(CMAKE_C_STANDARD 99)

add_executable(untitled2 main.c bthread_private.h bthread.h tqueue.h tqueue.c bthread.c test_bthread.h test_bthread.c test_tqueue.c test_tqueue.h)