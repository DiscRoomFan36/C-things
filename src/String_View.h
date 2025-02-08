//
// String_View.h - better strings
//
// Fletcher M - 08/02/2025
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
};


// TODO more functions

#endif // STRING_VIEW_H_
