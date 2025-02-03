#include <stdio.h>

#include "hashmap.c"

int main(void) {

    HashMap hm = {0};

    add(&hm, "hello", 69);
    add(&hm, "world", 420);

    printf("hello -> %d\n", get(&hm, "hello"));
    printf("world -> %d\n", get(&hm, "world"));

    delete(&hm, "hello");

    // this will assert
    // printf("hello -> %d\n", get(&hm, "hello"));

    free(hm.entrys);

    return 0;
}
