//
// Demo for String_Builder.h
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

#define STRING_BUILDER_IMPLEMENTATION
#include "String_Builder.h"

int main(int argc, char const **argv) {

    String_Builder builder = {0};

    SV my_string = SV_from_C_Str("Hello, World");

    SB_add_SV(&builder, my_string);

    return 0;
}

