
#define MY_STANDARD_IMPLEMENTATION
#include "../my_standard.h"

int main(void) {
    Arena arena = {0};

    String my_string_1 = S("this is a basic SV");
    printf("my_string: "S_Fmt"\n", S_Arg(my_string_1));


    String_Builder builder = {0};
    // force the builder to allocate every time.
    builder.base_new_allocation = 1;

    String my_string_2 = S("this is also a sv, but its about to be put into a string builder (SB)");

    String_Builder_String(&arena, &builder, my_string_2);
    String_Builder_printf(&arena, &builder, " - %d leaves in the pile", 65902);

    for (s64 i = 0; i < 15; i++) {
        String_Builder_printf(&arena, &builder, "\n new string!!!!!!!!!");
    }

    String new_string = String_Builder_To_String(&arena, &builder);

    printf("The string: '"S_Fmt"'\n", S_Arg(new_string));

    Arena_Free(&arena);

    return 0;
}

