
/*
NO UNITY BUILD
Single threaded : 25.250537
Multi buddy     : 6.048996
speedup: 4.174335x

UNITY BUILD
Single threaded : 25.846742
Multi buddy     : 6.391358
speedup: 4.044014x
*/
//
// a roughly 4x speedup from 12 Core Computer.
// kinda bad, but 4x is 4x,
//
// this could be faster if you made some sort of macro system.
// I think, (aka i have basically no justification but what ever),
// the reason its nowhere close to being 10x-11x (no multithreading
// is a perfect speedup) is because the compiler can't optimize
// though too many function pointer barriers.
//
// what i mean is, when the buddy threads get some work, they get a
// function to call on every element in the array, but it comes
// though a function pointer, so it could be any function.
// (but its not)
//
// Jai would fix this problem
//



#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h> // for memcmp
#include <time.h> // for the clock
#include <assert.h> // for assert

#include "ints.h"

#ifndef NO_UNITY
#define MULTI_BUDDY_IMPLEMENTATION

#define IS_UNITY_STRING "UNITY BUILD"
#else
#define IS_UNITY_STRING "NO UNITY BUILD"
#endif

#include "multi_buddy.h"


void mutate_number(void *arg) {
    int *number = (int*)arg;
    int n = *number;

    int m = sqrt(n*n*n);

    *number = (m * 103) % 1027;
}

typedef struct timespec time_unit;
time_unit get_time(void) {
    time_unit item;
    clock_gettime(CLOCK_MONOTONIC, &item);
    return item;
}
double elapsed_time_in_secs(time_unit start, time_unit finish) {
    double elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    return elapsed;
}

#define NUM_BASE 1000000
#define NUM_BUSY_LOOPS 1000

int main(void) {
    printf("\n"IS_UNITY_STRING"\n");

    srandom(time(NULL));

    init_multi_buddy();

    // so the compiler cant get fancy on us
    s64 number_count = NUM_BASE + (random()%10);
    s64 busy_loop_count = NUM_BUSY_LOOPS + (random()%10);

    int *numbers1 = malloc(number_count * sizeof(int));
    int *numbers2 = malloc(number_count * sizeof(int));


    // initialize the arrays
    for (s64 i = 0; i < number_count; i++) {
        int random_number = random() % 1028;
        numbers1[i] = random_number;
        numbers2[i] = random_number;
    }

    double single_thread_time;
    double multi_buddy_time;

    { // standard single threaded
        printf("Running single threaded\n");
        time_unit start = get_time();
        for (s64 j = 0; j < busy_loop_count; j++) {
            for (s64 i = 0; i < number_count; i++) {
                mutate_number(numbers1 + i);
            }
        }
        time_unit end = get_time();

        single_thread_time = elapsed_time_in_secs(start, end);
    }

    { // with multi buddy
        printf("Running multi threaded\n");
        time_unit start = get_time();
        for (s64 j = 0; j < busy_loop_count; j++) {
            run_function_on_array_with_buddies(numbers2, sizeof(int), number_count, mutate_number);
        }
        time_unit end = get_time();

        multi_buddy_time = elapsed_time_in_secs(start, end);
    }

    printf("Single threaded : %f\n", single_thread_time);
    printf("Multi buddy     : %f\n", multi_buddy_time);
    printf("speedup: %fx\n", single_thread_time / multi_buddy_time);

    // test if the arrays are equal
    if (memcmp(numbers1, numbers2, number_count * sizeof(int)) != 0) {
        fprintf(stderr, "-----------------------------------------------------\n");
        fprintf(stderr, "ERROR!!!!!!! The two arrays were not the same!!!!!!!!\n");
        fprintf(stderr, "-----------------------------------------------------\n");
    }

    free(numbers1);
    free(numbers2);

    shut_down_multi_buddy();
    return 0;
}
