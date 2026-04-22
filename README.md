
# Bested.h

The Best(ed.h) C standard library ever made.

In a Single-Header-File style.

Please note, I have not namespaced anything, (other than some Struct specific internal stuff). There is a note at the beginning of the Bested.h file about this issue.


## Quickstart

Just #include the file in your project, and your ready to go!

Just remember to #define the implementation somewhere.

```c
#define BESTED_IMPLEMENTATION
#include "Bested.h"

int main(void) {
    const char *c_str = temp_sprintf("It Works! %d", 69502);
    printf("%s\n", c_str);
    return 0;
}
```

### Recommended Compiler Flags:

```c
// the normal ones
-Wall -Wextra -ggdb
// these ones are so it doesn't yell at you because of my weird macro tricks
-Wno-gnu-statement-expression -Wno-gnu-alignof-expression
-Wno-gnu-zero-variadic-macro-arguments -Wno-initializer-overrides

// all in one line
-Wall -Wextra -ggdb -Wno-gnu-statement-expression -Wno-gnu-alignof-expression -Wno-gnu-zero-variadic-macro-arguments -Wno-initializer-overrides
```


## Notable Features:

### The ZEROED macro, for some context in further code snippets.

```c
// I really hate c++ sometimes
#ifdef __cplusplus
    // this allows c++ to do value initialization,
    // witch is spiritually the same thing.
    //
    // also c++ complains if there is only 1 zero.
    #define ZEROED { /* Imagine there was a zero here */ }
#else
    // pretty sure this zero is necessary in C
    #define ZEROED {0}
#endif
```


### Arena & Arena_Pool

These two things solve 99% of all memory problems.

#### Arena's

```c
// Arena's are Zero initialized, this one is ready to go.
Arena a = ZEROED;

s64 *x   = Arena_Alloc(&a, sizeof(s64), .clear_to_zero = true);
// not zero initialized.
Foo *foo = Arena_Alloc_Struct(&a, Foo);

Arena_Mark mark = Arena_Get_Mark(&a);
Arena_Set_To_Mark(&a, mark);

{
    Arena_Initialize_First_Page(&a, 10000);

    // initialize the first page with known memory, who need's malloc.
    local_persist u8 buffer[1 << 16] = ZEROED;
    Arena_Add_Buffer_As_Storage_Space(&a, buffer, sizeof(buffer));

    // maybe you don't want any more space than what you just gave it?
    a.panic_when_trying_to_allocate_new_page = true;
}


const char *hello = Arena_sprintf(&a, "Hello %s\n", "World");

// clears the arena, keeps the memory, all your pointers are dead now.
Arena_Clear(&a);

// yes, the arena dose in fact, allocate memory.
Arena_Free(&a);
```

But in a lot of cases, you might:
    1. make an arena,
    2. allocate stuff,
    3. then free it.
the arena itself has a 1-to-1, allocate-to-free ratio,

But what if there was a better way?

#### Arena_Pool's

```c
// ready to go.
Arena_Pool pool = ZEROED;


// just grab an arena, as many times as you want.
// the pool itself can grow.
//
// tread safe
Arena *a = Pool_Get(&pool);
// tread safe
Pool_Release(&pool, a);

// arena b is probably arena a.
Arena *b = Pool_Get(&pool);


// All the memory in the program cleaned up!
Pool_Free_Arenas(&pool);
```

But, in reality, allocating with arena's is kinda annoying, even with the pool.

If only there was some kind of construct that could allocate many items at once?


### Dynamic Arrays, with settable allocators.

```c
//
// Example:
//   - make a variable
//      Array(Foo) foo_array;
//
//   - make a type
//      typedef Array(Bar) Bar_Array;
//
//   foo_array.items     = /* the array pointer */
//   foo_array.count     = /* number of items in array */
//   foo_array.capacity  = /* the capacity */
//   foo_array.allocator = /* a settable arena allocator */
//
#define Array(Type)                         \
    struct {                                \
        Type *items;                        \
        u64 count;                          \
        u64 capacity;                       \
        Arena *allocator;                   \
    }


typedef struct {
    s32 bar;
} Foo;

typedef Array(Foo) Foo_Array;


// arrays are zero initialized. and can be used in this state.
//
// all memory is malloc()'d, (or BESTED_MALLOC()'d in this case)
Foo_Array foo_array = ZEROED;

// but here is the real magic,
//
// now you can make as many arrays as you want,
// no need to think about the memory.
foo_array.allocator = Pool_Get(&pool);

// foo_array is now a regular array, you can:
{
    // access an element
    foo_array.items[i];

    // get the count
    foo_array.count;

    Array_Append(&foo_array, item);
    Array_Insert(&foo_array, index, item);

    Array_Reserve(&foo_array, num_to_reserve);

    // remove and move.
    Array_Remove(&foo_array, index)
    // the cooler remove
    Array_Swap_And_Remove(&foo_array, 3);

    // iteration helper.
    Array_For_Each(Foo, it, &foo_array) {
        u64 index = it - foo_array.items;
        printf("%zu: %d\n", index, it.bar);
    }
    // or do it yourself.
    for (u64 i = 0; i < foo_array.count; i++) {
        Foo *foo = foo_array.items[i];
    }

    // if you didn't set an allocator. will crash program
    // if you didn't set an allocator and call this.
    Array_Free(&foo_array);
    // or if you did set an allocator:
    Pool_Release(&pool, foo_array.allocator);
}
```

### Type Safe Hash Map's (with settable allocators.)

```c
//
// Example:
//   - make a variable:
//     Hash_Map(u32, f32) id_to_percent_map = ZEROED;
//
//   - make a type:
//     typedef Hash_Map(String, Baz) Baz_Hash_Map;
//
//   id_to_percent_map.count         = /* number of entries in hash map        */
//   id_to_percent_map.hash_function = /* hash function to use for the key     */
//   id_to_percent_map.eq_function   = /* equality function to use for the key */
//   id_to_percent_map.allocator     = /* a settable arena allocator           */
//   id_to_percent_map.default_value = /* the default value when you use Hash_Map_Get_Or_Default() and the key is not in the map */
//
// ```
//     Hash_Map(s32, String) hash_map = {
//         .hash_function = NULL, // the default hash function just takes
//         .eq_function   = NULL, // the bytes of your type and hash's it.
//
//         .default_value = S("NO VALUE"), // a default value
//     };
//
//    Hash_Map(String, s32) reverse_map = {
//         .hash_function = Hash_Map_Hash_String, // some hash functions are provided for the String types,
//         .eq_function   = Hash_Map_Eq_String,   // (as well as 'const char *' type, but who cares about that one.)
//
//         .default_value = 0, // by default this is allready zero.
//    };
// ```
//
#define Hash_Map(Key_Type, Value_Type)      \
    struct {                                \
        struct {                            \
            u64        hash;                \
            Key_Type   key;                 \
            Value_Type value;               \
        } *entries;                         \
                                            \
        /* total number of alive items in hash_map */   \
        u64 count;                          \
        /* total number dead items in hash map */       \
        u64 dead_count;                     \
        u64 capacity;                       \
                                            \
        Hash_Function     hash_function;    \
        Equality_Function eq_function;      \
                                            \
        /* Settable allocator */            \
        Arena *allocator;                   \
                                            \
        /* Default value of new items */    \
        Value_Type default_value;           \
    }


typedef struct {
    u32 kaz;
    f64 lax;
} Boz;

typedef Hash_Map(Boz, String) Boz_To_Type;
Boz_To_Type boz_to_type = {
    .default_value = S("(NONE)"),
    .allocator = &my_arena,
};

Boz friendly_boz = { .kaz = 5, .lax = 1 };
Boz angry_boz = { .kaz = 2, .lax = 7 };

// this will return a pointer value,
//
// (there may or may not be something already there,
// but the default value will not be Mem_Copy()'d
// into there if there was no previous entry with this key..)
*Hash_Map_Put(&boz_to_type, friendly_boz) = S("friendly");
// this is the same thing, but the default is set in the memory.
*Hash_Map_Get_Or_Default(&boz_to_type, angry_boz) = S("angry");

// setting something to default in a kinda funny way.
Hash_Map_Get_Or_Default(&boz_to_type, ((Boz){0, 0}));

// may return null
Hash_Map_Get(&boz_to_type, ...);

if (Hash_Map_Contains(&boz_to_type, friendly_boz)) {
    printf("its here!\n");
}

// i don't like angry things.
Hash_Map_Remove(&boz_to_type, angry_boz);

// remove something but you only have the value.
String *empty_type = Hash_Map_Get(&boz_to_type, ((Boz){0, 0}));
Hash_Map_Remove_By_Value(&boz_to_type, empty_type);


// i want to fit at least 100 items in here.
Hash_Map_Reserve(&boz_to_type, 100);

// clear the hash map, keep the memory
Hash_Map_Clear(&boz_to_type);
// free the memory, don't use if you use an allocator
Hash_Map_Free(&boz_to_type);

// loop over all entries.
Hash_Map_For_Each(it, &boz_to_type) {
    // get key for value.
    String *key = Hash_Map_Key_For(&boz_to_type, it);

    // this operation is fine actually, it just marks the cell as dead,
    // just don't add to the array while in a loop.
    Hash_Map_Remove_By_Value(&boz_to_type, it); 
}
```


### String & String_Builder

#### String

Do you like C's strings?

Of course you don't!

```c
typedef struct {
    char *data;
    u64 length;
} String;

// S() is a helper macro, turn a 'const char *' into a String
String my_string = S("Hello World");

// printing helpers
printf("my_string: "S_Fmt"\n", S_Arg(my_string));

const char *c_string = String_To_C_Str(&arena, my_string);
// useful to pass into fopen() and other c std functions.
const char *c_string = temp_String_To_C_Str(my_string);

String dup_string = String_Duplicate(&arena, my_string, .null_terminate = true);

// in place, don't know why i have this function...
String_To_Upper(my_string);

bool result = String_Eq(my_string, S("Hello World"));
bool result = String_Starts_With(my_string, S("Hello"));
bool result = String_Ends_With  (my_string, S("World"));

bool result = String_Contains_Char(my_string, " ");

// returns -1 on failure.
s64 index  = String_Find_Index_Of_Char(my_string, " ");
// String version
s64 index  = String_Find_Index_Of(my_string, S("World"));

// moves the string forwards,
// no bounds checking is done here.
String_Advance(&my_string, 1);
String next_string = String_Advanced(my_string, 1);

// gets the next line of of a string, times whitespace, skips empty lines.
String line = String_Get_Next_Line(&file_text, &line_num, SGNL_Trim | SGNL_Skip_Empty)


// this function should get its own section.
const char *words       = temp_sprintf("there are %d leaves\n", 69502);
String words_but_string = temp_String_sprintf("there are %d leaves\n", 69502);
```

#### String_Builder

Efficient for medium to large strings.

(aka it always dose at least 1 allocation when adding any data.)

```c
// zero initialized
String_Builder sb = ZEROED;
// works the same as the dynamic arrays.
sb.allocator = Pool_Get(&pool);

// something you can set to change how much
// it allocates when needing a new buffer.
//
// (4 * KILOBYTE) is the default.
sb.base_new_allocation = (4 * KILOBYTE);

// how much data is in here.
//
// also NOTE, string builder is passed by pointer into most things.
u64 count = String_Builder_Count(&sb);

String_Builder_Ptr_And_Size(&sb, ptr, size);
String_Builder_String(&sb, string);
u64 num_printed = String_Builder_printf(&sb, "Hello %s", "world");

String_Builder_Struct_Bytes(&sb, struct_ptr);
String_Builder_Array_Bytes(&sb, foo_array.items, foo_array.count);

// returns a null terminated string.
String result = String_Builder_To_String(&sb);

{ // dose not null terminate the file
    FILE *file = fopen("output.txt", "wb");
    String_Builder_To_File(&sb, file);
    fclose(file);
}


// clears the buffers, dose not remove memory, like Arena_Clear();
String_Builder_Clear(&sb);

// only call this if not using an allocator.
String_Builder_Free(&sb);
```



### Int Types, Yes This Is Important.

typedef's for all integer widths, never again have to wonder what size an integer is. Developer ergonomics.

```c
typedef uint64_t        u64;
typedef uint32_t        u32;
typedef uint16_t        u16;
typedef uint8_t         u8;

typedef int64_t         s64;
typedef int32_t         s32;
typedef int16_t         s16;
typedef int8_t          s8;

// fixed width bool types, might be useful, but probably not.
typedef u64             b64;
typedef u32             b32;
typedef u16             b16;
typedef u8              b8;

typedef float           f32;
typedef double          f64;
```

Also stdbool.h is included as well. so you don't need to #include that yourself. (like i do in every single C file I make.)


### Nice Macro's

NOTE: Clamp() Macro collides with raylib's clamp.


```c

// I always forget how to call typeof()
#define Typeof(x)       __typeof__(x)
// stick the extra Typeof() in there to prevent [-Wgnu-alignof-expression]
#define Alignof(x)      alignof(Typeof(x))


// Not wrapping stuff in () because this is purely text based, nothing fancy can be done here.
#define Member(type, member)        ( ((type*)0)->member )
#define Member_Size(type, member)   sizeof( Member(type, member) )

// offsetof is also here. comes from stddef.h



#define Min(a, b)   ({ Typeof(a) _a = (a); Typeof(b) _b = (b); _a < _b ? _a : _b; })
#define Max(a, b)   ({ Typeof(a) _a = (a); Typeof(b) _b = (b); _a > _b ? _a : _b; })

#define Sign(T, x)  ((T)((x) > 0) - (T)((x) < 0))
#define Abs(x)      (Sign(Typeof(x), (x)) * (x))

// TODO this has the same problem as Min & Max
#define Clamp(x, min, max)              ((x) < (min) ? (min) : (((x) > (max)) ? (max) : (x)))

#define Is_Between(x, lower, upper)     (((lower) <= (x)) && ((x) <= (upper)))


#define Array_Len(array)            (sizeof(array) / sizeof((array)[0]))

// integers only
#define Is_Pow_2(n)                 (((n) != 0) && (((n) & ((n)-1)) == 0))

#define Proper_Mod(x, y) ({ Typeof(y) _y = (y); (((x) % _y) + _y) % _y; })

#define Div_Ceil(x, y)      (((x) + (y) - 1) / (y))
#define Div_Floor(x, y)     ((x) / (y))



// also there is:

// these have printf() formatting
PANIC("cannot happen size should not be negative, was %d", size);
TODO("finish this");

// this accepts no arguments
UNREACHABLE();

```

### Nice #defines

```c
#define PI              3.1415926535897932384626433
#define TAU             6.283185307179586476925286766559
#define E               2.718281828459045235360287471352
#define ROOT_2          1.414213562373095048801688724209

#define KILOBYTE        (1024UL)
#define MEGABYTE        (1024UL * 1024UL)
#define GIGABYTE        (1024UL * 1024UL * 1024UL)
#define TERABYTE        (1024UL * 1024UL * 1024UL * 1024UL)

#define THOUSAND        (1000UL)
#define MILLION         (1000UL * 1000UL)
#define BILLION         (1000UL * 1000UL * 1000UL)
#define TRILLION        (1000UL * 1000UL * 1000UL * 1000UL)


#define MILLISECONDS_PER_SECOND          THOUSAND
#define MICROSECONDS_PER_SECOND         MILLION
#define NANOSECONDS_PER_SECOND          BILLION

// good for usleep
#define MILLISECONDS_PER_MICROSECOND     (MICROSECONDS_PER_SECOND / MILLISECONDS_PER_SECOND)



// ===================================================
//         What static really means
// ===================================================

// Mark a function that must be used in the current compilation block and cannot be seen outside of it.
#define internal            static
// mark a variable inside of a function that persists though function calls.
#define local_persist       static
// casey once said that all this dose is help the compiler deal with
// multithreaded code, by forcing the functions to not store the
// value of the variable into their own registers.
#define global_variable     static
```


### Source Code Location
```c
// this is just pretty handy to carry around.
typedef struct {
    const char *file;
    s32 line;
} Source_Code_Location;

// printf helpers
#define SCL_Fmt         "%s:%d:"
#define SCL_Arg(scl)    scl.file, scl.line

#define Get_Source_Code_Location() ( (Source_Code_Location){ .file = __FILE__, .line = __LINE__ } )

bool source_code_location_eq(Source_Code_Location a, Source_Code_Location b);
```


### Just Some Nice Functions

```c
String file = Read_Entire_File("input.txt", NULL);

u64 time_start = nanoseconds_since_unspecified_epoch();

// debugs a variable
debug(time_start);
// prints "DEBUG: time_start = {NUMBER}"

// puts a common interrupt instruction.
//
// asm("int3");
debug_break()
```


## TODO

- better time manipulation functions.
- get that print_running() function from sudoku project
- Finish the hashmap implementation.
- replace *_test.c files with a TESTMA.h, file.
