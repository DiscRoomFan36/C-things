//
// Demo for profile.h
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
#define PROFILE_IMPLEMENTATION
#include "profile.h"

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

    PROFILE_ZONE("malloc");
    float *array = malloc(array_size * sizeof(float));

    PROFILE_ZONE("initialize the array");
    for (size_t i = 0; i < array_size; i++) {
        array[i] = randf() + randf() - 1;
    }

    // hmmm...
    // PROFILE_ZONE_END();

    {
        PROFILE_SECTION("calculations");

        PROFILE_ZONE("total");
        double total = 0;
        for (size_t i = 0; i < array_size; i++) total += array[i];
        printf("total   : %f\n", total);

        PROFILE_ZONE("total^2");
        double total_2 = 0;
        for (size_t i = 0; i < array_size; i++) total_2 += array[i]*array[i];
        printf("total^2 : %f\n", total_2);

        PROFILE_ZONE("total^3");
        double total_3 = 0;
        for (size_t i = 0; i < array_size; i++) total_3 += array[i]*array[i]*array[i];
        printf("total^3 : %f\n", total_3);

        PROFILE_SECTION_END();
    }


    PROFILE_PRINT();
    PROFILE_RESET();

    return 0;
}

