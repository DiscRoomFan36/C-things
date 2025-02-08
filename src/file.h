//
// file.h - tools to handle files
//
// Fletcher M - 08/02/2025
//

#ifndef FILE_H_
#define FILE_H_

#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ints.h"
#include "dynamic_array.h"


typedef struct String_Array {
    char **items;
    u64 count;
    u64 capacity;
} String_Array;


void get_all_files_in_directory(char *directory, String_Array *array) {
    DIR *dir = opendir(directory);

    if (!dir) {
        printf("ERROR when opening directory: %s\n", strerror(errno));
        return;
    }

    while (True) {
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (!entry) break;

        // check its a file
        if (entry->d_type != 8) continue;

        // add the new string
        int name_len = strlen(entry->d_name);
        char *new_string = malloc((name_len+1) * sizeof(char));
        assert(new_string != NULL);

        strcpy(new_string, entry->d_name);
        // new_string[name_len] = '\0';

        da_append(array, new_string);
    }

    if (errno) printf("ERROR when reading directory: %s\n", strerror(errno));

    if (closedir(dir) != 0) {} // an error occurred, we dont actually care...
}

// TODO use String_View
char *read_entire_file(char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("ERROR when opening the file: %s\n", strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    u64 size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *result = malloc((size+1) * sizeof(char));
    assert(result != NULL);

    u64 read_bytes = fread(result, sizeof(char), size, file);
    assert(read_bytes == size);
    result[size] = '\0';


    if (fclose(file)) {} // error, we dont care

    return result;
}


#endif // FILE_H_
