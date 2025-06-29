//
// String_View.h - better strings
//
// Fletcher M - 30/06/2025
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


// You can define your own malloc and free.
#ifndef SV_MALLOC
    #include <stdlib.h>
    #define SV_MALLOC(size)    malloc(size)
    #define SV_FREE(ptr)       free(ptr)
#endif


// functions on String views

// takes a C_Str return a SV, dose not allocate
SV SV_from_C_Str(const char *str);
// duplicate a String View, uses SV_MALLOC.
// SV.data could be null if SV_MALLOC fails, (but size will also be 0 so its good?)
SV SV_dup(SV s);

// free the pointer with 'SV_FREE' and sets data to NULL
void SV_free(SV *s);

// transforms a SV in place to uppercase
void SV_To_Upper(SV *s);

// SV equality check
bool32 SV_Eq(SV s1, SV s2);
bool32 SV_starts_with(SV s, SV prefix);
bool32 SV_starts_with_c_str(SV s, const char *prefix);

bool32 SV_contains_char(SV s, char c);
// finds the first occurrence of c in s, -1 on failure
s64 SV_find_index_of_char(SV s, char c);
// finds the first occurrence of needle in s, -1 on failure
// if needle.size == 0, returns -1.
s64 SV_find_index_of(SV s, SV needle);

// finds the line at index, and returns a SV of the line, strips, '\n'
SV SV_get_single_line(SV s, s64 index);

// advance the data, subtract the size, in place.
// dose not check for null, or do any bounds checking. thats on you.
void SV_advance(SV *s, s64 count);
// advance the data, subtract the size, returns a new copy.
// dose not check for null, or do any bounds checking. thats on you.
SV SV_advanced(SV s, s64 count);

typedef bool32 (*char_to_bool)(char);
// returns True if c is one of the ASCII whitespace characters.
bool32 SV_is_space(char c);
// returns a SV with the front chopped off, according to the test function.
// use SV_is_space to chop the whitespace off of the front.
SV SV_chop_while(SV s, char_to_bool test_char_function);


// TODO more functions

#endif // STRING_VIEW_H_

#ifdef STRING_VIEW_IMPLEMENTATION

#ifndef STRING_VIEW_IMPLEMENTATION_GUARD_
#define STRING_VIEW_IMPLEMENTATION_GUARD_



u64 SV_strlen(const char *str) {
    if (!str) return 0;
    for (u64 i = 0;; i++) {
        if (str[i] == 0) return i;
    }
}


SV SV_from_C_Str(const char *str) {
    SV result = {
        .data = (char *) str,
        .size = SV_strlen(str),
    };
    return result;
}

SV SV_dup(SV s) {
    SV result;
    result.data = SV_MALLOC(s.size);
    if (result.data) {
        result.size = s.size;

        // TODO sv copy?
        for (s64 i = 0; i < s.size; i++) {
            result.data[i] = s.data[i];
        }
    }

    return result;
}


void SV_free(SV *s) {
    if (s->data) { SV_FREE(s->data); }
    s->data = NULL;
    s->size = 0;
}


void SV_To_Upper(SV *s) {
    for (s64 n = 0; n < s->size; n++) {
        if ('a' <= s->data[n] && s->data[n] <= 'z') {
            s->data[n] += 'A' - 'a';
        }
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

bool32 SV_starts_with_c_str(SV s, const char *prefix) {
    return SV_starts_with(s, SV_from_C_Str(prefix));
}


bool32 SV_contains_char(SV s, char c) {
    return SV_find_index_of_char(s, c) != -1;
}

// TODO: simd? or dose that happen automagically?
s64 SV_find_index_of_char(SV s, char c) {
    for (s64 i = 0; i < s.size; i++) {
        if (s.data[i] == c) return i;
    }
    return -1;
}

s64 SV_find_index_of(SV s, SV needle) {
    if (needle.size == 0) return -1;

    // easy out
    if (needle.size == 1) return SV_find_index_of_char(s, needle.data[0]);

    while (True) {
        s64 index = SV_find_index_of_char(s, needle.data[0]);
        if (index == -1) return -1;

        // check if not enough room for needle
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


SV SV_get_single_line(SV s, s64 i) {
    // clamp i if its to big. this handles a lot more errors than your thinking about.
    if (i > s.size) i = s.size;

    SV result = {.data = s.data + i, .size = s.size - i};

    s64 index = SV_find_index_of_char(result, '\n');

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
    return (SV){.data = s.data + count, .size = s.size - count};
}


bool32 SV_is_space(char c) {
    if (c == ' ')  return True;
    if (c == '\f') return True;
    if (c == '\n') return True;
    if (c == '\r') return True;
    if (c == '\t') return True;
    if (c == '\v') return True;
    return False;
}

SV SV_chop_while(SV s, char_to_bool test_char_function) {
    s64 i;
    for (i = 0; i < s.size; i++) {
        if (!test_char_function(s.data[i])) break;
    }
    s.data += i;
    s.size -= i;
    return s;
}



#endif // STRING_VIEW_IMPLEMENTATION_GUARD_
#endif // STRING_VIEW_IMPLEMENTATION
