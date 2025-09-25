
#define BESTED_IMPLEMENTATION
#include "../Bested.h"

int main(void) {
    Arena arena = {0};

    String my_string_1 = S("this is a basic SV");
    printf("my_string: "S_Fmt"\n", S_Arg(my_string_1));


    String_Builder builder = {0};
    builder.allocator = &arena;
    // force the builder to allocate every time.
    builder.base_new_allocation = 1;

    String my_string_2 = S("this is also a sv, but its about to be put into a string builder (SB)");

    String_Builder_String(&builder, my_string_2);
    String_Builder_printf(&builder, " - %d leaves in the pile", 65902);

    for (s64 i = 0; i < 15; i++) {
        String_Builder_printf(&builder, "\n new string!!!!!!!!!");
    }

    String new_string = String_Builder_To_String(&builder);

    printf("The string: '"S_Fmt"'\n", S_Arg(new_string));

    // we gave this an allocator allready, so dont bother freeing it.
    // String_Builder_Free(&builder);

    Arena_Free(&arena);

    return 0;
}

