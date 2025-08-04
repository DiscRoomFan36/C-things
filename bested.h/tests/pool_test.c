
#define BESTED_IMPLEMENTATION
#include "../bested.h"

#include <stdatomic.h>

int main(void) {
    Arena_Pool pool = {0};

    // TODO better pool test.

    Arena *arena = Pool_Get(&pool);

    int *seven_ints = Arena_Alloc_Array(arena, 7, int);

    for (size_t i = 0; i < 7; i++) {
        seven_ints[i] = i*i;
    }
    for (size_t i = 0; i < 7; i++) {
        printf("%zu: %d\n", i, seven_ints[i]);
    }

    for (size_t i = 0; i < 1000000; i++) {
        int *leak = Arena_Alloc_Struct(arena, int);
        *leak = 69;

        if (i % 100000 == 0) {
            Arena_Clear(arena);
        }
    }

    const char *str1 = Arena_sprintf(arena, "look ma! %s\n", "im a printf!");
    const char *str2 = Arena_sprintf(arena, "there are %d leaves in the pile.\n", 65902);

    printf("%s", str1);
    printf("%s", str2);

    Pool_Release(&pool, arena);

    Pool_Free_Arenas(&pool);
    return 0;
}

