
#include "../src/file.h"

int main(void) {
    String_Array files = {0};
    get_all_files_in_directory("./src/", &files);

    for (u64 i = 0; i < files.count; i++) {
        printf("%zu: %s\n", i, files.items[i]);
    }

    da_free_items(&files);
    da_free(&files);


    char *entire_file = read_entire_file("./src/ints.h");
    assert(entire_file != NULL);
    printf("%s\n", entire_file);
    free(entire_file);

    return 0;
}
