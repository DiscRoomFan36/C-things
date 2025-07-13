//
// Demo for String_Helper.h
//
// DiscRoomFan
//
// created - 14/05/2025
//
// build and run:
// $ make run
//


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define STRING_HELPER_IMPLEMENTATION
#include "String_Helper.h"

int main(void) {

    SV my_string_1 = SV_from_C_Str("this is a basic SV");
    printf("my_string: "SV_Fmt"\n", SV_Arg(my_string_1));


    String_Builder builder = {0};
    // force the builder to allocate every time.
    builder.base_new_allocation = 1;

    SV my_string_2 = SV_from_C_Str("this is also a sv, but its about to be put into a string builder (SB)");

    SB_add_SV(&builder, my_string_2);
    SB_printf(&builder, " - %d leaves in the pile", 65902);

    for (s64 i = 0; i < 15; i++) {
        SB_printf(&builder, "\n new string!!!!!!!!!");
    }

    SV new_string = SB_to_SV(&builder);

    printf("The string: '"SV_Fmt"'\n", SV_Arg(new_string));

    SV_free(&new_string);
    SB_free(&builder);


    return 0;
}

