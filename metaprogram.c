
// A metaprogram that takes a c/h/cpp file and produces a [file]_meta.h file
//
// TODO more info here


#define BESTED_IMPLEMENTATION
#include "Bested.h"

Arena file_arena = {0};
Arena string_builder_arena = {0};

void print_useage(const char *program_name) {
    printf("USAGE: %s <c_file.[c|h]> <out_path.h>\n", program_name);
}


typedef struct String_Array {
    _Array_Header_;
    String *items;
} String_Array;



int main(int argc, char const *argv[]) {
    const char *program_name = argv[0];


    if (argc != 3) {
        print_useage(program_name);
        return 1;
    }

    String filename = S(argv[1]);
    const char *outpath = argv[2];

    b32 ends_with_c = String_Ends_With(filename, S(".c")) || String_Ends_With(filename, S(".h")) || String_Ends_With(filename, S(".cpp"));
    if (!ends_with_c) {
        printf("File dose not end with 'c' or 'h' or 'cpp'\n");
        print_useage(program_name);
        return 1;
    }


    String file = Read_Entire_File(&file_arena, filename);
    if (!file.data) {
        printf("File dose not exist!\n");
        print_useage(program_name);
    }

    String filename_upper = String_Dup(&string_builder_arena, filename);
    filename_upper = String_Path_to_Filename(filename_upper);
    filename_upper = String_Remove_Extention(filename_upper);
    String_To_Upper(&filename_upper);
    // printf("name: '"S_Fmt"'\n", S_Arg(filename_upper));


    String_Array typedef_definitions = {0};
    String_Array struct_definitions = {0};
    String_Array enum_definitions = {0};
    String_Array function_definitions = {0};

    u64 line_num = 0;
    while (true) {

        String line = String_Get_Next_Line(&file, &line_num, SGNL_All);
        if (line.length == 0) break;
        // printf("%lu: "S_Fmt"\n", line_num, S_Arg(line));

        #define STRUCT_STR      S("struct ")
        #define FUNC_STR        S("function ")
        #define ENUM_STR        S("enum ")
        #define TYPEDEF_STR     S("typedef ")

        if (String_Starts_With(line, TYPEDEF_STR)) {
            // the user has allready typedef'd these things.
            if (String_Starts_With(line, S("typedef struct "))) continue;
            if (String_Starts_With(line, S("typedef enum ")  )) continue;

            Array_Push(&string_builder_arena, &typedef_definitions, line);
            continue;
        }

        if (String_Starts_With(line, STRUCT_STR)) {
            // get the correct name.
            String name = String_Advanced(line, STRUCT_STR.length);
            name.length = String_Find_Index_Of_Char(name, '{');
            ASSERT(name.length != (u64)-1);
            name = String_Trim_Right(name);

            Array_Push(&string_builder_arena, &struct_definitions, name);
            continue;
        }

        if (String_Starts_With(line, ENUM_STR)) {
            String enum_def = String_Advanced(line, ENUM_STR.length);
            enum_def.length = String_Find_Index_Of_Char(enum_def, '{');
            ASSERT(enum_def.length != (u64)-1);
            enum_def = String_Trim_Right(enum_def);

            Array_Push(&string_builder_arena, &enum_definitions, enum_def);
            continue;
        }

        if (String_Starts_With(line, FUNC_STR)) {
            // get the function definition.
            String func_def = String_Advanced(line, FUNC_STR.length);
            func_def.length = String_Find_Index_Of_Char(func_def, '{');
            ASSERT(func_def.length != (u64)-1);
            func_def = String_Trim_Right(func_def);

            Array_Push(&string_builder_arena, &function_definitions, func_def);
            continue;
        }
    }




    String_Builder sb = {0};

    String_Builder_printf(&string_builder_arena, &sb, "\n");
    String_Builder_printf(&string_builder_arena, &sb, "#ifndef "S_Fmt"_META_H_\n", S_Arg(filename_upper));
    String_Builder_printf(&string_builder_arena, &sb, "#define "S_Fmt"_META_H_\n", S_Arg(filename_upper));
    String_Builder_printf(&string_builder_arena, &sb, "\n");

    String_Builder_printf(&string_builder_arena, &sb, "// typedefs\n");
    Array_For_Each(String, typedef_line, &typedef_definitions) {
        String_Builder_printf(&string_builder_arena, &sb, S_Fmt"\n", S_Arg(*typedef_line));
    }

    String_Builder_printf(&string_builder_arena, &sb, "\n");
    String_Builder_printf(&string_builder_arena, &sb, "// structs\n");
    Array_For_Each(String, name, &struct_definitions) {
        String_Builder_printf(&string_builder_arena, &sb, "typedef struct "S_Fmt" "S_Fmt";\n", S_Arg(*name), S_Arg(*name));
    }

    String_Builder_printf(&string_builder_arena, &sb, "\n");
    String_Builder_printf(&string_builder_arena, &sb, "// enums\n");
    Array_For_Each(String, enum_name, &enum_definitions) {
        String_Builder_printf(&string_builder_arena, &sb, "typedef enum "S_Fmt" "S_Fmt";\n", S_Arg(*enum_name), S_Arg(*enum_name));
    }

    String_Builder_printf(&string_builder_arena, &sb, "\n");
    String_Builder_printf(&string_builder_arena, &sb, "// functions\n");
    Array_For_Each(String, func_def, &function_definitions) {
        String_Builder_printf(&string_builder_arena, &sb, S_Fmt";\n", S_Arg(*func_def));
    }

    String_Builder_printf(&string_builder_arena, &sb, "\n");
    String_Builder_printf(&string_builder_arena, &sb, "#endif // "S_Fmt"_META_H_\n", S_Arg(filename_upper));


    FILE *outfile = fopen(outpath, "wb");
    if (outfile) {
        String_Builder_To_File(&sb, outfile);
        printf("saved meta file to %s\n", outpath);
    } else {
        printf("Could not open the outfile at path %s\n", outpath);
        return 1;
    }

    return 0;
}
