
#define BESTED_IMPLEMENTATION
#include "../Bested.h"

int main(void) {
    Arena a = {0};

    int *seven_ints = Arena_Alloc(&a, 7 * sizeof(int));

    for (int i = 0; i < 7; i++) {
        seven_ints[i] = i*i;
    }
    for (int i = 0; i < 7; i++) {
        printf("%d: %d\n", i, seven_ints[i]);
    }

    for (int i = 0; i < 1000000; i++) {
        int *leak = Arena_Alloc_Struct(&a, int);
        *leak = 69;

        if (i % 100000 == 0) {
            Arena_Clear(&a);
        }
    }

    const char *str1 = Arena_sprintf(&a, "look ma! %s\n", "im a printf!");
    const char *str2 = Arena_sprintf(&a, "there are %d leaves in the pile.\n", 65902);

    printf("%s", str1);
    printf("%s", str2);

    Arena_Free(&a);
    return 0;
}

