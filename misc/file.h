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
#include <assert.h>

#include "ints.h"
#include "dynamic_array.h"


typedef struct String_Array {
    char **items;
    u64 count;
    u64 capacity;
} String_Array;


#ifndef DT_REG
#define DT_REG 8
#endif // DT_REG

#ifndef DT_DIR
#define DT_DIR 4
#endif // DT_DIR


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
        if (entry->d_type != DT_REG) continue;

        // add the new string
        int name_len = strlen(entry->d_name);
        char *new_string = malloc((name_len+1) * sizeof(char));
        assert(new_string != NULL);

        strcpy(new_string, entry->d_name);

        da_append(array, new_string);
    }

    if (errno) printf("ERROR when reading directory: %s\n", strerror(errno));

    if (closedir(dir) != 0) {} // an error occurred, we dont actually care...
}

void get_all_dirs_in_directory(char *directory, String_Array *array) {
    DIR *dir = opendir(directory);

    if (!dir) {
        printf("ERROR when opening directory: %s\n", strerror(errno));
        return;
    }

    while (True) {
        errno = 0;
        struct dirent *entry = readdir(dir);
        if (!entry) break;

        // check its a direct
        if (entry->d_type != DT_DIR) continue;

        // add the new string
        int name_len = strlen(entry->d_name);
        char *new_string = malloc((name_len+1) * sizeof(char));
        assert(new_string != NULL);

        strcpy(new_string, entry->d_name);

        da_append(array, new_string);
    }

    if (errno) printf("ERROR when reading directory: %s\n", strerror(errno));

    if (closedir(dir) != 0) {} // an error occurred, we dont actually care...
}



typedef struct {
    void *data;
    // number of bytes
    long long size;
} File_Result;

// TODO use String_View
File_Result read_entire_file(char *filename) {
    FILE *file = fopen(filename, "rb");
    File_Result result = {0};

    if (file) {
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (result.size >= 0) {
            result.data = malloc(result.size);
            if (result.data) {
                size_t read_bytes = fread(result.data, 1, result.size, file);
                assert(read_bytes == (size_t) result.size);
            }
        }

        fclose(file);
    }

    return result;
}


#endif // FILE_H_
