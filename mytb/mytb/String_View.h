//
// String_View.h - better strings
//
// Fletcher M - 09/04/2025
//


#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_

#include "ints.h"

typedef struct SV {
    s64 size;
    char *data;
} SV;

#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int) (sv).size, (sv).data 
// Example: printf("my_sv = "SV_Fmt"\n", SV_Arg(my_sv));

typedef struct SV_Array {
    SV *items;
    u64 count;
    u64 capacity;
} SV_Array;


// functions on String views

// takes a C_Str return a SV, dose not allocate
SV SV_from_C_Str(const char *str);
// duplicate a String View, uses malloc
SV SV_dup(SV s);

// free the pointer with 'free' and sets data to NULL
void SV_free(SV *s);

// transforms a SV in place to uppercase
void SV_To_Upper(SV *s);

// SV equality check
bool32 SV_Eq(SV s1, SV s2);
// SV starts with prefix
bool32 SV_starts_with(SV s, SV prefix);
// SV has char, no unicode support. yet?
bool32 SV_contains_char(SV s, char c);

// finds the first occurrence of c in s, -1 on failure
s64 find_index_of_char(SV s, char c);
// finds the first occurrence of needle in s, -1 on failure
// needle must have size > 0
s64 find_index_of(SV s, SV needle);

// finds the line at index, and returns a SV of the line, strips, '\n'
SV get_single_line(SV s, s64 index);

// advance the data, subtract the size, in place
void SV_advance(SV *s, s64 count);
// advance the data, subtract the size, returns a new copy
SV SV_advanced(SV s, s64 count);

// TODO more functions

#endif // STRING_VIEW_H_


#ifdef STRING_VIEW_IMPLEMENTATION

#ifndef STRING_VIEW_IMPLEMENTATION_GUARD_
#define STRING_VIEW_IMPLEMENTATION_GUARD_


// TODO remove? make own functions?
#include <string.h>
// Needed for toupper
// TODO remove
#include <ctype.h>

// only assert in extreme cases.
#include <assert.h>

// TODO accept an allocator? or make SV_USE_MALLOC, or use MALLOC define?
// for 'malloc'
#include <stdlib.h>


SV SV_from_C_Str(const char *str) {
    SV result = {
        .data = (char *) str,
        .size = strlen(str),
    };
    return result;
}

SV SV_dup(SV s) {
    SV result;
    result.data = malloc(s.size);
    result.size = s.size;

    // TODO sv copy?
    for (s64 i = 0; i < s.size; i++) {
        result.data[i] = s.data[i];
    }

    return result;
}


void SV_free(SV *s) {
    if (s->data) { free(s->data); }
    s->data = NULL;
    s->size = 0;
}


void SV_To_Upper(SV *s) {
    for (s64 n = 0; n < s->size; n++) {
        s->data[n] = toupper(s->data[n]);
    }
}


bool32 SV_Eq(SV s1, SV s2) {
    if (s1.size != s2.size) return False;
    for (s64 n = 0; n < s1.size; n++) {
        if (s1.data[n] != s2.data[n]) return False;
    }
    return True;
}

bool32 SV_starts_with(SV s, SV prefix) {
    if (s.size < prefix.size) return False;
    for (s64 i = 0; i < prefix.size; i++) {
        if (s.data[i] != prefix.data[i]) return False;
    }
    return True;
}

bool32 SV_contains_char(SV s, char c) {
    for (s64 i = 0; i < s.size; i++) {
        if (s.data[i] == c) return True;
    }
    return False;
}


// TODO: simd? or dose that happen automagically?
s64 find_index_of_char(SV s, char c) {
    for (s64 i = 0; i < s.size; i++) {
        if (s.data[i] == c) return i;
    }
    return -1;
}

s64 find_index_of(SV s, SV needle) {
    assert(needle.size > 0);

    // easy out
    if (needle.size == 1) return find_index_of_char(s, needle.data[0]);

    while (True) {
        s64 index = find_index_of_char(s, needle.data[0]);
        if (index == -1) return -1;

        // not enough room for needle
        if (s.size - index < needle.size) return -1;

        bool32 flag = True;
        for (s64 i = 1; i < needle.size; i++) {
            if (s.data[index+i] != needle.data[i]) {
                flag = False;
                break;
            }
        }

        if (flag) return index;

        s.data += index + 1;
        s.size -= index + 1;
    }

    return -1;
}


SV get_single_line(SV s, s64 i) {
    SV result = {.data = s.data + i, .size = s.size - i};

    s64 index = find_index_of_char(result, '\n');

    // clamp the length to the far newline
    if (index != -1) { result.size = index; }

    // go back until newline before result.data,
    // and make sure it doesn't go out of bounds
    while (result.data != s.data && result.data[-1] != '\n') {
        result.data -= 1;
        result.size += 1;
    }

    return result;
}


void SV_advance(SV *s, s64 count) {
    s->data += count;
    s->size -= count;
}

SV SV_advanced(SV s, s64 count) {
    return (SV){.data = s.data + count, .size = s.size -= count};
}


#endif // STRING_VIEW_IMPLEMENTATION_GUARD_
#endif // STRING_VIEW_IMPLEMENTATION
