#include <stdio.h>

#include "hashmap_macro.c"

bool eq_int(int a, int *b) { return (a) == (*b); }
HASH_INT hash_int(int to_hash) { return to_hash * 23478956239456; }

int main(void) {

    HASHMAP(int, int) hm_1 = {0};

    hm_1.equality_function = eq_int;
    hm_1.hash_function = hash_int;

    add(&hm_1, 69, 420);
    add(&hm_1, 120, 230);

    printf("69 -> %d\n", get(&hm_1, 69));




    free(hm_1.entrys);

    return 0;
}
