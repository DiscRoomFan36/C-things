//
// Demo for profiler.h
//
// DiscRoomFan
//
// created - 15/03/2025
//
// build and run:
// $ make && ./main 100000000
//


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#define PROFILE_CODE
#define PROFILER_IMPLEMENTATION
#include "profiler.h"

#define shift(argc, argv) (assert((argc) > 0), (argc)--, *(argv)++)

float randf(void) {
    return (float) rand() / (float) RAND_MAX;
}

int main(int argc, char const **argv) {
    const char *program = shift(argc, argv);

    if (argc == 0) {
        fprintf(stderr, "USAGE: %s <array_size>\n", program);
        return 1;
    }

    size_t array_size = atol(shift(argc, argv));
    if (array_size == 0) {
        fprintf(stderr, "USAGE: %s <array_size>\n", program);
        return 1;
    }

    srand(time(0));

    for (size_t num_iterations = 0; num_iterations < 10; num_iterations++) {

        PROFILER_ZONE("malloc");
            float *array = malloc(array_size * sizeof(float));
        PROFILER_ZONE_END();

        PROFILER_ZONE("initialize the array");
            for (size_t i = 0; i < array_size; i++) {
                array[i] = randf() + randf() - 1;
            }
        PROFILER_ZONE_END();

        // hmmm...
        // PROFILER_ZONE_END();

        PROFILER_ZONE("calculations");

            PROFILER_ZONE("total");
                double total = 0;
                for (size_t i = 0; i < array_size; i++) total += array[i];
                printf("total   : %f\n", total);
            PROFILER_ZONE_END();

            PROFILER_ZONE("total^2");
                double total_2 = 0;
                for (size_t i = 0; i < array_size; i++) total_2 += array[i]*array[i];
                printf("total^2 : %f\n", total_2);
            PROFILER_ZONE_END();

            PROFILER_ZONE("total^3");
                double total_3 = 0;
                for (size_t i = 0; i < array_size; i++) total_3 += array[i]*array[i]*array[i];
                printf("total^3 : %f\n", total_3);
            PROFILER_ZONE_END();

        PROFILER_ZONE_END();

        free(array);
    }


#ifdef PROFILE_CODE

    Profiler_Stats_Array stats = collect_stats();
    Double_Array numbers = {0};

    for (size_t i = 0; i < stats.count; i++) {
        Profiler_Stats stat = stats.items[i];

        numbers.count = 0;
        for (size_t i = 0; i < stat.times.count; i++) {
            profiler_da_append(&numbers, stat.times.items[i]);
        }

        Numerical_Average_Bounds nab = get_numerical_average(numbers);

        printf("%-20s : %f +- %f\n", stat.title, nab.sample_mean, nab.standard_deviation);
    }

    for (size_t i = 0; i < stats.count; i++) {
        profiler_da_free(&stats.items[i].times);
    }
    profiler_da_free(&stats);
    profiler_da_free(&numbers);

#endif

    PROFILER_RESET();
    PROFILER_FREE();

    return 0;
}

