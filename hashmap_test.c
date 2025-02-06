#include <stdio.h>

#include "hashmap.c"

int main(void) {

    HashMap_Str_Int hm = {0};

    add(&hm, "hello", 69);
    add(&hm, "world", 420);

    printf("hello -> %ld\n", get(&hm, "hello"));
    printf("world -> %ld\n", get(&hm, "world"));

    delete(&hm, "hello");

    if (key_exists(&hm, "hello")) {
        printf("\"hello\" is in the hashmap\n");
    } else {
        printf("\"hello\" is NOT in the hashmap\n");
    }

    free(hm.entrys);

    return 0;
}
