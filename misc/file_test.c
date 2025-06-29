
#include "file.h"

int main(void) {
    String_Array files = {0};
    get_all_files_in_directory("./mytb/", &files);

    for (u64 i = 0; i < files.count; i++) {
        printf("%zu: %s\n", i, files.items[i]);
    }

    da_free_items(&files);
    da_free(&files);


    SV entire_file = read_entire_file("./mytb/ints.h");
    printf(SV_Fmt"\n", SV_Arg(entire_file));
    free(entire_file.data);

    return 0;
}
