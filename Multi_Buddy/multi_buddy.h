
#ifndef MULTI_BUDDY_H_
#define MULTI_BUDDY_H_

// TODO just slap ints.h into this file?
#include "ints.h"


// call at the start of your program to start some background threads.
void init_multi_buddy(void);

// call at the end to shut down the background threads.
void shut_down_multi_buddy(void);

// the type of the function you pass into 'run_function_on_array_with_buddies()'.
//
// A function that takes a pointer to something.
// will be called with every element in the array.
typedef void (*Buddy_Work_Function)(void*);

// run 'func' on every element in the array 'array'.
//
// uses multithreading to get a bit of a speed bump.
//
// works better with higher element counts
void run_function_on_array_with_buddies(void *array, s64 item_size, s64 count, Buddy_Work_Function func);


#endif // MULTI_BUDDY_H_



#ifdef MULTI_BUDDY_IMPLEMENTATION


#include <pthread.h> // for all the thread stuff.
#include <unistd.h> // to get the number of cores.
#include <assert.h> // for 'assert'
#include <stdatomic.h> // for atomic add


#define local static


// maximum amount of buddies, who has a more than 64 cores anyway? (No, GPU's dont count.)
#define NUM_MAX_BUDDY_THREADS 64


// How many times it runs the function in a batch.
// This is way faster than running one at a time.
//
// Although it could degenerate if one run of the function takes a long time,
// or the array your working on is to small for the batch size.
#define THREAD_BATCH_SIZE 512


// so you can call the init and shut down functions multiple times
bool32 has_been_initalized = False;

// the idents of the buddy threads.
//
// we wont use all the slots, i just dont want to dynamically allocate.
pthread_t thread_idents[NUM_MAX_BUDDY_THREADS];
// how many threads are active.
s64 buddy_thread_count = 0;


// what the thread's wait on when not doing anything
// This get initalized in init_multi_buddy
pthread_barrier_t work_barrier;

bool32 shut_down_buddy_workers = False;

// These are the variables that are given to every thread, so they know what work to do.
void *work_array;
s64 work_item_size;
s64 work_count;
Buddy_Work_Function work_function;


// the thread's use this to quickly grab batches of work to do.
_Atomic s64 atomic_counter;


// the function all buddy threads run. will be waiting for some input
local void *buddy_thread_function(void *arg) {
    // id is a number from [0..buddy_thread_count-1]
    s64 id = (s64) arg;
    assert(0 <= id && id < buddy_thread_count);

    // printf("Hello from thread %ld\n", id);

    while (True) {
        // wait for some work.
        pthread_barrier_wait(&work_barrier);

        // shut down
        if (shut_down_buddy_workers) break;

#if 1
        // its working time!
        while (True) {
            s64 current_item = atomic_fetch_add(&atomic_counter, THREAD_BATCH_SIZE);

            // if we've don all the items, exit
            if (current_item >= work_count) break;

            // else there is some work to do.
            char *array_as_chars = work_array;

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
        }
#else
        // calculate how much work to do.

        s64 average_work  = work_count / buddy_thread_count;
        s64 leftover_work = work_count % buddy_thread_count;

        // how much work it should do.
        s64 my_work_to_do = average_work + (id < leftover_work);
        // what index in the array this thread starts at.
        s64 my_work_start = (average_work*id) + (id < leftover_work ? id : leftover_work);

        char *array_as_chars = work_array;

        // chunking for speed.
        while (my_work_to_do >= THREAD_BATCH_SIZE) {
            for (size_t i = 0; i < THREAD_BATCH_SIZE; i++) {
                work_function(array_as_chars + (work_item_size * (i + my_work_start)));
            }
            my_work_to_do -= THREAD_BATCH_SIZE;
            my_work_start += THREAD_BATCH_SIZE;
        }

        for (s64 i = 0; i < my_work_to_do; i++) {
            work_function(array_as_chars + (work_item_size * (i + my_work_start)));
        }
#endif

        // we are done working! now wait for the others.
        pthread_barrier_wait(&work_barrier);
    }

    // finishing up
    return NULL;
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

    buddy_thread_count = how_many_threads;

    for (s64 i = 0; i < how_many_threads; i++) {
        pthread_create(thread_idents + i, NULL, buddy_thread_function, (void*) i);
    }
}

void shut_down_multi_buddy(void) {
    assert(has_been_initalized);

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
    assert(has_been_initalized);

    work_array = array;
    work_item_size = item_size;
    work_count = count;
    work_function = func;

    // set to zero.
    atomic_counter = 0;

    // TODO #if this.
/*
    // distribute work evenly.
    s64 work_amounts[NUM_MAX_BUDDY_THREADS];
    for (s64 i = 0; i < buddy_thread_count; i++) {
        work_amounts[i] = work_count / buddy_thread_count;
        // the leftover work.
        if (i < work_count % buddy_thread_count) work_amounts[i] += 1;
    }

    { // test the distribution.
        s64 total = 0;
        for (s64 i = 0; i < buddy_thread_count; i++) total += work_amounts[i];
        // printf("Total: %ld, work_count: %ld\n", total, work_count);
        assert(total == work_count);
    }
*/


    // start all the threads.
    pthread_barrier_wait(&work_barrier);

    // wait for them to finish.
    pthread_barrier_wait(&work_barrier);
}



#endif // MULTI_BUDDY_IMPLEMENTATION
