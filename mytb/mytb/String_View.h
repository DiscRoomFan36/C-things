//
// String_View.h - better strings
//
// Fletcher M - 09/04/2025
//


#ifndef STRING_VIEW_H_
#define STRING_VIEW_H_

#include "ints.h"

typedef struct SV {
    u64 size;
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

// transforms a SV in place to uppercase
void SV_To_Upper(SV *s);

// SV equality check
bool32 SV_Eq(SV s1, SV s2);
// SV starts with prefix
bool32 SV_starts_with(SV s, SV prefix);
// SV has char, no unicode support. yet?
bool32 SV_contains_char(SV s, char c);


// TODO more functions

#endif // STRING_VIEW_H_

#ifdef STRING_VIEW_IMPLEMENTATION

#ifndef STRING_VIEW_IMPLEMENTATION_GUARD_
#define STRING_VIEW_IMPLEMENTATION_GUARD_


// TODO remove? make own functions?
#include "string.h"
// Needed for toupper
// TODO remove
#include "ctype.h"


SV SV_from_C_Str(const char *str) {
    SV result = {
        .data = (char *) str,
        .size = strlen(str),
    };
    return result;
}


void SV_To_Upper(SV *s) {
    for (u64 n = 0; n < s->size; n++) {
        s->data[n] = toupper(s->data[n]);
    }
}


bool32 SV_Eq(SV s1, SV s2) {
    if (s1.size != s2.size) return False;
    for (u64 n = 0; n < s1.size; n++) {
        if (s1.data[n] != s2.data[n]) return False;
    }
    return True;
}

bool32 SV_starts_with(SV s, SV prefix) {
    if (s.size < prefix.size) return False;
    for (u64 i = 0; i < prefix.size; i++) {
        if (s.data[i] != prefix.data[i]) return False;
    }
    return True;
}

bool32 SV_contains_char(SV s, char c) {
    for (u64 i = 0; i < s.size; i++) {
        if (s.data[i] == c) return True;
    }
    return False;
}


#endif // STRING_VIEW_IMPLEMENTATION_GUARD_
#endif // STRING_VIEW_IMPLEMENTATION
