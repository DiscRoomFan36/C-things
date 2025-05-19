
#ifndef MULTI_BUDDY_H_
#define MULTI_BUDDY_H_

// TODO just slap ints.h into this file?
#include "ints.h"


struct void_star_array {
    void *items;
    s64 count;
    s64 capacity;
};


// call at the start of your program to start some background threads.
void init_multi_buddy(void);

// call at the end to shut down the background threads.
void shut_down_multi_buddy(void);


// how many times it runs the function in a batch.
// this is way faster than running one at a time.
//
// although it could degenerate if one run of the function takes a long time,
// or the array your working on is to small, to small for the batch size.
#define THREAD_BATCH_SIZE 512


typedef void (*Buddy_Work_Function)(void*);

void run_function_on_array_with_buddies(void *array, s64 item_size, s64 count, Buddy_Work_Function func);


#endif // MULTI_BUDDY_H_



#ifdef MULTI_BUDDY_IMPLEMENTATION


#include <pthread.h> // for all the thread stuff.
#include <unistd.h> // to get the number of cores.
#include <assert.h> // for 'assert'
#include <stdatomic.h> // for atomic add


#define local static


// so you can call the init and shut down functions multiple times
bool32 has_been_initalized = False;

#define NUM_MAX_BUDDY_THREADS 64

// the idents of the buddy threads.
//
// we wont use all the slots, i just dont want to dynamically allocate.
pthread_t thread_idents[NUM_MAX_BUDDY_THREADS];
// how many threads are active.
s64 buddy_thread_count = 0;


// what the thread's wait on when not doing anything
pthread_barrier_t work_barrier;

bool32 shut_down_buddy_workers = False;

void *work_array;
s64 work_item_size;
s64 work_count;
// a function that takes in a pointer to void, and returns void.
Buddy_Work_Function work_function;

_Atomic s64 atomic_counter;


// the function all buddy threads run. will be waiting for some input
local void *buddy_thread_function(void *arg) {
    s64 id = (s64) arg;
    (void) id;

    // printf("Hello from thread %ld\n", id);

    while (True) {
        // wait for some work.
        pthread_barrier_wait(&work_barrier);

        // shut down
        if (shut_down_buddy_workers) return NULL;

        // its working time!
        while (True) {
            s64 current_item = atomic_fetch_add(&atomic_counter, THREAD_BATCH_SIZE);

            // if we've don all the items, exit
            if (current_item >= work_count) break;

            // else their is some work to do.
            char *array_as_chars = work_array;

#if 1
            if (current_item + THREAD_BATCH_SIZE <= work_count) {
                // best case we have the full batch amount of work to do.
                // the compiler will hopefully optimize this into something cool.
                for (s64 i = 0; i < THREAD_BATCH_SIZE; i++) {
                    work_function(array_as_chars + (work_item_size * (current_item + i)));
                }
            } else {
                // just do the rest of the work.
                // this loop is slower, because the end condition is not known at compile time.
                for (s64 i = 0; i < work_count - current_item; i++) {
                    work_function(array_as_chars + (work_item_size * (current_item + i)));
                }
            }
#else
            for (s64 i = 0; i < THREAD_BATCH_SIZE && current_item + i < work_count; i++) {
                    work_function(array_as_chars + (work_item_size * (current_item + i)));
            }
#endif
        }

        // we are done working! now wait for the others.
        pthread_barrier_wait(&work_barrier);
    }
}

void init_multi_buddy(void) {
    if (has_been_initalized) return;
    has_been_initalized = True;

    // determine how many threads we should spawn
    s64 numCPU = sysconf(_SC_NPROCESSORS_ONLN);
    assert(numCPU > 0);
    // just spawns number of threads equal to number of cores.
    s64 how_many_threads = (numCPU < NUM_MAX_BUDDY_THREADS) ? numCPU : NUM_MAX_BUDDY_THREADS;

    // init the barrier
    pthread_barrier_init(&work_barrier, NULL, how_many_threads+1);

    shut_down_buddy_workers = False;

    for (s64 i = 0; i < how_many_threads; i++) {
        pthread_create(thread_idents + i, NULL, buddy_thread_function, (void*) i);
    }
    buddy_thread_count = how_many_threads;
}

void shut_down_multi_buddy(void) {
    shut_down_buddy_workers = True;
    // wake the workers
    pthread_barrier_wait(&work_barrier);

    for (s64 i = 0; i < buddy_thread_count; i++) {
        pthread_join(thread_idents[i], NULL);
    }
    buddy_thread_count = 0;

    has_been_initalized = False;
}



void run_function_on_array_with_buddies(void *array, s64 item_size, s64 count, Buddy_Work_Function func) {
    work_array = array;
    work_item_size = item_size;
    work_count = count;
    work_function = func;

    // set to zero.
    atomic_counter = 0;

    // start all the threads.
    pthread_barrier_wait(&work_barrier);

    // wait for them to finish.
    pthread_barrier_wait(&work_barrier);
}



#endif // MULTI_BUDDY_IMPLEMENTATION
