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

int main(void) {
    String_Builder builder = {0};
    // force the builder to allocate every time.
    builder.base_new_allocation = 1;

    SV my_string = SV_from_C_Str("Hello, World");

    SB_add_SV(&builder, my_string);

    SB_printf(&builder, " - %d leaves in the pile", 65902);

    for (s64 i = 0; i < 100; i++) {
        SB_printf(&builder, "\n new string!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    }

    SV new_string = SB_to_SV(&builder);

    printf("The string: '"SV_Fmt"'\n", SV_Arg(new_string));

    SV_free(&new_string);
    SB_free(&builder);

    return 0;
}

