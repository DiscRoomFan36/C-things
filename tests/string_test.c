

#define BESTED_IMPLEMENTATION
#include "../Bested.h"

int main(void) {
    Split_Once_Result split_result = Split_Once(S("Hello, World!"), S(", W"));
    printf("left: ["S_Fmt"], right: ["S_Fmt"], ok: [%s]\n", S_Arg(split_result.left), S_Arg(split_result.right), split_result.ok ? "true" : "false");


    String text = S("There are 65902 leaves in the pile.");
    String_Array split_by_result = ZEROED;
    String_Split_By(text, S(" "), &split_by_result);

    Array_For_Each(word, &split_by_result) {
        u64 index = word - split_by_result.items;
        printf("%ld -> "S_Fmt"\n", index, S_Arg(*word));
    }
}

