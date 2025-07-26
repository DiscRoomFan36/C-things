//
// Demo for Parser.h
//
// DiscRoomFan
//
// created - 26/07/2025
//
// build and run:
// $ make run
//


#include <stdio.h>


#define PARSER_IMPLEMENTATION
#include "parser.h"

const char *text_blob =
"int main(void) {\n"
"\n"
"    printf(\"Hello world\\n\");\n"
"\n"
"    return 0;\n"
"}\n";

int main(void) {
    Parser p = new_parser(SV_from_C_Str(text_blob));

    while (parser_peek_next(&p).kind != TK_EOF) {
        Token token = parser_take_next(&p);

        if (token.kind == TK_Ident) {
            printf("Ident: "SV_Fmt"\n", SV_Arg(token.text));
        } else if (token.kind == TK_String) {
            printf("String: "SV_Fmt"\n", SV_Arg(token.text));
        } else {
            printf("%c\n", token.kind);
        }
    }

    return 0;
}


#define STRING_HELPER_IMPLEMENTATION
#include "String_Helper.h"
