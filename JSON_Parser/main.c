
#include <stdio.h>

#include "ints.h"
#define STRING_VIEW_IMPLEMENTATION
#include "String_View.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"

#define JSON_PARSER_IMPLEMENTATION
#include "JSON_parser.h"


internal void print_spaces(int depth) {
    for (int i = 0; i < depth; i++) printf("    ");
}

void print_json_value(JSON_Value *json_value, int depth);
void print_json_object(JSON_Object *json_object, int depth);


void print_json_value(JSON_Value *json_value, int depth) {
    switch (json_value->kind) {
    case VK_None: assert(False);

    case VK_String: {
        print_spaces(depth);
        printf("\""SV_Fmt"\"", SV_Arg(json_value->as_string.string));
    } break;

    case VK_Number: {
        print_spaces(depth);
        printf("%f", json_value->as_number.as_float);
    } break;

    case VK_Object: {
        print_json_object(&json_value->as_object, depth);
    } break;

    case VK_Array: {
        print_spaces(depth);
        printf("[\n");
        for (s64 i = 0; i < json_value->as_array.count; i++) {
            print_json_value(&json_value->as_array.items[i], depth+1);
            printf(",\n");
        }
        print_spaces(depth); printf("]");
    } break;

    case VK_Bool: {
        print_spaces(depth);
        printf(json_value->as_bool ? "true" : "false");
    } break;

    case VK_Null: {
        print_spaces(depth);
        printf("null");
    } break;
    }
}

void print_json_object(JSON_Object *json_object, int depth) {
    print_spaces(depth);
    printf("{\n");

    for (s64 i = 0; i < json_object->count; i++) {
        print_spaces(depth+1);
        printf(SV_Fmt" : \n", SV_Arg(json_object->keys[i].string));

        print_json_value(&json_object->values[i], depth + 2);
        printf(",\n");
    }

    print_spaces(depth);
    printf("}");

    if (depth == 0) printf("\n");
}


int main(int argc, char const **argv) {

    const char *program_name = argv[0];

    const char *filepath;

    if (argc == 1) {
        printf("No file provided, useing default\n");
        filepath = "test.json";
    } else if (argc != 2) {

        fprintf(stderr, "USAGE: %s [file.json]\n", program_name);
        exit(1);

    } else {

        filepath = argv[1];
    }

    // TODO maybe someday we could not load the entire thing, and use the fread interface...
    Arena arena = {0};

    JSON_Object *json_object = parse_json_file(filepath, &arena);


    print_json_object(json_object, 0);


    Arena_free(&arena);
    return 0;
}

