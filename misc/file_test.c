
#include "file.h"

int main(void) {
    String_Array files = {0};
    get_all_files_in_directory(".", &files);

    for (u64 i = 0; i < files.count; i++) {
        printf("%zu: %s\n", i, files.items[i]);
    }

    da_free_items(&files);
    da_free(&files);

    File_Result entire_file = read_entire_file("./ints.h");
    printf("%.*s\n", (int) entire_file.size, (char*) entire_file.data);
    free(entire_file.data);

    return 0;
}
