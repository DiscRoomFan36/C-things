
#include <stdio.h>

#define ARENA_IMPLEMENTATION
#include "arena.h"

int main(void) {

    Arena a = {0};

    int *seven_ints = Arena_alloc(&a, sizeof(int)*7);

    for (size_t i = 0; i < 7; i++) {
        seven_ints[i] = i*i;
    }
    for (size_t i = 0; i < 7; i++) {
        printf("%zu: %d\n", i, seven_ints[i]);
    }

    for (size_t i = 0; i < 1000000; i++) {
        int *leak = Arena_alloc(&a, sizeof(int));
        *leak = 69;

        if (i % 100000 == 0) {
            Arena_reset(&a);
        }
    }

    const char *str1 = Arena_sprintf(&a, "look ma! %s\n", "im a printf!");
    const char *str2 = Arena_sprintf(&a, "there are %d leaves in the pile.\n", 65902);

    printf("%s", str1);
    printf("%s", str2);

    Arena_free(&a);
    return 0;
}

