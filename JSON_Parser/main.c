
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ints.h"
#define STRING_VIEW_IMPLEMENTATION
#include "String_View.h"
#define ARENA_IMPLEMENTATION
#include "arena.h"


#define internal static

#define todo(text) assert(False && text)


SV read_entire_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    SV result = {0};

    if (file) {
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (result.size >= 0) {
            result.data = malloc(result.size * sizeof(char));

            if (result.data) {
                u64 read_bytes = fread(result.data, sizeof(char), result.size, file);
                assert(read_bytes == (u64) result.size); // Can this ever fail?
            }
        }

        fclose(file);
    }

    return result;
}


internal inline bool32 is_space(char c) {
    if (c == ' ')  return True;
    if (c == '\f') return True;
    if (c == '\n') return True;
    if (c == '\r') return True;
    if (c == '\t') return True;
    if (c == '\v') return True;
    return False;
}

SV SV_chop_whitespace(SV s) {
    s64 i;
    for (i = 0; i < s.size; i++) {
        if (!is_space(s.data[i])) break;
    }
    if (i != 0) SV_advance(&s, i);
    return s;
}

typedef struct JSON_Tokenizer {
    // Where the final object is stored.
    Arena *Object_Arena;

    // for error tracking.
    const char *filename;
    s64 line_num, col_num;
    SV parseing;
} JSON_Tokenizer;


internal void tk_advance(JSON_Tokenizer *tk, s64 count) {
    assert(count <= tk->parseing.size);

    for (s64 i = 0; i < count; i++) {
        char c = *tk->parseing.data;

        if (c == '\n') {
            tk->line_num += 1;
            tk->col_num = 1;
        } else {
            tk->col_num += 1;
        }
    }

    SV_advance(&tk->parseing, count);
}

internal void tk_chop_whitespace(JSON_Tokenizer *tk) {
    SV new_parseing = SV_chop_whitespace(tk->parseing);
    tk_advance(tk, new_parseing.data - tk->parseing.data);
}


void tk_print_file_and_line(JSON_Tokenizer *tk, FILE *out) {
    fprintf(out, "%s:%ld:%ld:", tk->filename, tk->line_num, tk->col_num);
}


// predef structs
typedef struct JSON_String JSON_String;
typedef struct JSON_Number JSON_Number;
typedef struct JSON_Object JSON_Object;
typedef struct JSON_Array  JSON_Array;
typedef struct JSON_Value  JSON_Value;

// predef functions
internal void parse_json_string(JSON_Tokenizer *tk, JSON_String *result);
internal void parse_json_number(JSON_Tokenizer *tk, JSON_Number *result);
internal void parse_json_object(JSON_Tokenizer *tk, JSON_Object *result);
internal void parse_json_array (JSON_Tokenizer *tk, JSON_Array  *result);
internal void parse_json_value (JSON_Tokenizer *tk, JSON_Value  *result);


struct JSON_String {
    // this string is zero terminated for your pleasure
    SV string;
};

struct JSON_Number {
    s64 as_int;
    f64 as_float;
};

struct JSON_Object {
    // TODO maybe this needs to be pointer stable array, if we want to not use to much memory
    JSON_String *keys;
    JSON_Value  *values;
    s64 count;
    s64 capacity;
};

struct JSON_Array {
    JSON_Value *items;
    s64 count;
    s64 capacity;
};


typedef enum {
    VK_None = 0,

    VK_String,
    VK_Number,
    VK_Object,
    VK_Array,
    VK_Bool,
    VK_Null,
} JSON_Value_Kind;

// Is a Tagged Union.
struct JSON_Value {
    JSON_Value_Kind kind;

    union {
        JSON_String string;
        JSON_Number number;
        JSON_Object object;
        JSON_Array array;
        bool32 bool;
    } as;
};


// TODO SV memset?
internal void my_memset(void *dest, u8 to_set, s64 n) {
    char *d = dest;
    for (s64 i = 0; i < n; i++) d[i] = to_set;
}


internal void parse_json_string(JSON_Tokenizer *tk, JSON_String *result) {
    assert(*tk->parseing.data == '"');

    my_memset(result, 0, sizeof(JSON_String));

    tk_advance(tk, 1);

    s64 new_string_size = 0;

    while (True) {
        s64 i;
        for (i = 0; i < tk->parseing.size; i++) {
            if (tk->parseing.data[i] == '\\') break;
            if (tk->parseing.data[i] == '"')  break;
        }

        if (i == tk->parseing.size) {
            tk_print_file_and_line(tk, stderr);
            fprintf(stderr, " unclosed string.\n");

            assert(False);
        }

        if (tk->parseing.data[i] == '\\') {
            todo("Handle control characters");
        } else {
            new_string_size += i;
            break;
        }
    }

    result->string.data = Arena_alloc(tk->Object_Arena, new_string_size+1);
    result->string.size = new_string_size;

    for (s64 i = 0; i < new_string_size; i++) {

        if (*tk->parseing.data == '\\') {
            todo("Handle control characters");
        } else {

            result->string.data[i] = *tk->parseing.data;
            tk_advance(tk, 1);
        }
    }

    result->string.data[new_string_size] = 0;

    tk_advance(tk, 1);
    return;
}

internal void parse_json_number(JSON_Tokenizer *tk, JSON_Number *result) {
    my_memset(result, 0, sizeof(JSON_Number));

    s64 non_digit_char = -1;
    for (s64 i = 1; i < tk->parseing.size; i++) {
        char c = tk->parseing.data[i];

        if (!('0' <= c && c <= '9') && c != '.' && c != 'e' && c != 'E') {
            non_digit_char = i;
            break;
        }
    }

    if (non_digit_char == -1) {
        todo("Unexpected EOF");
    }

    f64 atof_result = atof(tk->parseing.data);
    result->as_float = atof_result;
    result->as_int = (s64) atof_result;

    tk_advance(tk, non_digit_char);

    // TODO write my own number parser.
    /*
    s64 number_length = 0;
    bool32 is_negative = False;
    bool32 seen_first_number = False;
    bool32 first_number_was_zero = False;
    bool32 seen_dot = False;
    bool32 seen_e = False;

    u64 result_high = 0;


    for (s64 i = 0; i < tk->parseing.size; i++) {
        char c = tk->parseing.data[i];

        if (!seen_first_number) {
            if (c == '-') {
                if (i != 0) {
                    todo("Handle Number Error, malformed number, got '-' not in first position");
                }
                is_negative = True;
                continue;
            }

            if (c == '0') {
                first_number_was_zero = True;

            } else if ('1' <= c && c <= '9') {

                first_number_was_zero = False;
                result_high = c - '0';

            } else {

                todo("Unknown symbol in Number parseing");
            }

            seen_first_number = True;

        } else {

            // the number could end...
            if ('0' <= c && c <= '9') {

                if (!seen_dot) {
                    if (first_number_was_zero) {
                        todo("Handle Number Error, first number was zero, but it wasn't followed by a dot.");
                    }

                    result_high = result_high * 10 + (c - '0');
                } else {

                    result_low

                }

            }

        }
    }
    */
}

internal void parse_json_object(JSON_Tokenizer *tk, JSON_Object *result) {
    assert(tk->parseing.data[0] == '{'); // parseing must be here
    tk_advance(tk, 1);

    my_memset(result, 0, sizeof(JSON_Object));

    tk_chop_whitespace(tk);

    if (tk->parseing.size == 0) {
        todo("Handle Error, unexpected end of file");
    }

    if (*tk->parseing.data == '}') {
        tk_advance(tk, 1);
        return;
    }

    while (True) {
        if (*tk->parseing.data != '"') {
            tk_print_file_and_line(tk, stderr);
            fprintf(stderr, " Exected '\"' got '%c'\n", *tk->parseing.data);
            assert(False);
        }

        if (result->count >= result->capacity) {
            // add more capacity
            // TODO temporary Arena for stuff like this?
            s64 new_capacity = result->capacity == 0 ? 32 : result->capacity * 2;

            result->keys   = Arena_realloc(tk->Object_Arena, result->keys,   result->capacity * sizeof(JSON_String), new_capacity * sizeof(JSON_String));
            result->values = Arena_realloc(tk->Object_Arena, result->values, result->capacity * sizeof(JSON_Value),  new_capacity * sizeof(JSON_Value) );

            result->capacity = new_capacity;
        }

        parse_json_string(tk, &result->keys[result->count]);

        tk_chop_whitespace(tk);

        // TK expect
        if (*tk->parseing.data != ':') {
            todo("Handle Error");
        }
        tk_advance(tk, 1);

        parse_json_value(tk, &result->values[result->count]);

        result->count += 1;

        if (tk->parseing.size == 0) {
            todo("Handle Error, unexpected end of file");
        }

        if (*tk->parseing.data == '}') break;

        if (*tk->parseing.data != ',') {
            tk_print_file_and_line(tk, stderr);
            fprintf(stderr, " Expected '}' or ',' got %c\n", *tk->parseing.data);
            assert(False);
        }
        tk_advance(tk, 1);

        tk_chop_whitespace(tk);
    }

    tk_advance(tk, 1);
    return;
}

internal void parse_json_array(JSON_Tokenizer *tk, JSON_Array *result) {
    memset(result, 0, sizeof(JSON_Array));

    assert(*tk->parseing.data == '[');
    tk_advance(tk, 1);

    tk_chop_whitespace(tk);

    while (tk->parseing.size > 0 && *tk->parseing.data != ']') {

        if (result->count >= result->capacity) {
            // add more capacity
            // TODO temporary Arena for stuff like this?
            s64 new_capacity = result->capacity == 0 ? 32 : result->capacity * 2;

            result->items = Arena_realloc(tk->Object_Arena, result->items, result->capacity * sizeof(JSON_Value), new_capacity * sizeof(JSON_Value));

            result->capacity = new_capacity;
        }

        parse_json_value(tk, &result->items[result->count]);
        result->count += 1;

        if (*tk->parseing.data == ',') {
            tk_advance(tk, 1);
        }
    }

    if (tk->parseing.size == 0) {
        todo("Unexpected EOF");
    }

    tk_advance(tk, 1);
}

internal void parse_json_value(JSON_Tokenizer *tk, JSON_Value *result) {
    my_memset(result, 0, sizeof(JSON_Value));

    tk_chop_whitespace(tk);

    if (tk->parseing.size == 0) {
        todo("Handle Error, unexpected EOF");
    }

    switch (*tk->parseing.data) {
    case '"': { // string
        result->kind = VK_String;
        parse_json_string(tk, &result->as.string);
    } break;

    case '{': {
        result->kind = VK_Object;
        parse_json_object(tk, &result->as.object);
    } break;

    case '[': {
        result->kind = VK_Array;
        parse_json_array(tk, &result->as.array);
    } break;

    case 't': case 'f': {
        result->kind = VK_Bool;
        result->as.bool = *tk->parseing.data == 't';

        if (*tk->parseing.data == 't') {
            // make sure it says 'true'
            SV c_str = SV_from_C_Str("true");
            if (!SV_starts_with(tk->parseing, c_str)) {
                todo("Handle Error, Unknown value");
            }
            tk_advance(tk, c_str.size);

        } else {
            // make sure it says 'false'
            SV c_str = SV_from_C_Str("false");
            if (!SV_starts_with(tk->parseing, c_str)) {
                todo("Handle Error, Unknown value");
            }
            tk_advance(tk, c_str.size);
        }
    } break;

    case 'n': {
        result->kind = VK_Null;
        SV c_str = SV_from_C_Str("null");
        if (!SV_starts_with(tk->parseing, c_str)) {
            todo("Handle Error");
        }
        tk_advance(tk, c_str.size);
    } break;


    case '-': case '0':
    case '1': case '2': case '3':
    case '4': case '5': case '6':
    case '7': case '8': case '9': {
        result->kind = VK_Number;
        parse_json_number(tk, &result->as.number);
    } break;

    default: {
        tk_print_file_and_line(tk, stderr);
        fprintf(stderr, "Unknown character '%c'\n", *tk->parseing.data);
        assert(False);
    } break;
    }

    tk_chop_whitespace(tk);

    return;
}


// accepts a file to parse, and a arena to store the new object.
JSON_Object *parse_json_file(const char *filename, Arena *object_arena) {
    SV file = read_entire_file(filename);

    if (!file.data) {
        fprintf(stderr, "Could not read file %s\n", filename);
        return NULL;
    }

    JSON_Tokenizer tk = {
        .filename = filename,

        .Object_Arena = object_arena,
        .parseing = file,

        .line_num = 1,
        .col_num = 1,
    };

    // TODO maybe mark the location, and reset if something fails.
    // now that we return a pointer, maybe null is an error, and we pass it up. (with something printed to stderr)

    // TODO keep track of file and line
    // s64 how_far_forward = new_parseing.data - t->parseing.data;
    tk_chop_whitespace(&tk);

    JSON_Object *result = Arena_alloc(tk.Object_Arena, sizeof(JSON_Object));
    parse_json_object(&tk, result);

    tk_chop_whitespace(&tk);

    if (tk.parseing.size != 0) {
        todo("Junk at end of file");
    }

    SV_free(&file);

    return result;
}


internal void print_spaces(int depth) {
    for (int i = 0; i < depth; i++) printf("    ");
}

void print_json_object(JSON_Object *json_object, int depth);

void print_json_value(JSON_Value *json_value, int depth) {

    switch (json_value->kind) {
    case VK_None: assert(False);

    case VK_String: {
        print_spaces(depth);
        printf("\""SV_Fmt"\"", SV_Arg(json_value->as.string.string));
    } break;

    case VK_Number: {
        print_spaces(depth);
        printf("%f", json_value->as.number.as_float);
    } break;

    case VK_Object: {
        print_json_object(&json_value->as.object, depth);
    } break;

    case VK_Array: {
        print_spaces(depth);
        printf("[\n");
        for (s64 i = 0; i < json_value->as.array.count; i++) {
            print_json_value(&json_value->as.array.items[i], depth+1);
            printf(",\n");
        }
        print_spaces(depth); printf("]");
    } break;

    case VK_Bool: {
        print_spaces(depth);
        printf(json_value->as.bool ? "true" : "false");
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

    const char *file_to_parse_path;

    if (argc == 1) {
        printf("No file provided, useing default\n");
        file_to_parse_path = "test.json";
    } else if (argc != 2) {

        fprintf(stderr, "USAGE: %s [file.json]\n", program_name);
        exit(1);

    } else {

        file_to_parse_path = argv[1];
    }

    // TODO maybe someday we could not load the entire thing, and use the fread interface...
    Arena arena = {0};

    JSON_Object *json_object = parse_json_file(file_to_parse_path, &arena);

    print_json_object(json_object, 0);

    Arena_free(&arena);

    return 0;
}

