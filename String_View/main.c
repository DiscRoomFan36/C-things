//
// Demo for String_View.h
//
// DiscRoomFan
//
// created - 14/05/2025
//
// build and run:
// $ make && ./main
//


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STRING_VIEW_IMPLEMENTATION
#include "String_View.h"

int main(void) {

    SV my_string = SV_from_C_Str("Hello, World");
    printf("my_string: "SV_Fmt"\n", SV_Arg(my_string));

    return 0;
}

