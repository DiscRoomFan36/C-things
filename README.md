
# Bested.h

The Best(ed.h) C standard library ever made.

In a Single-Header-File style.

Please note, I have not namespaced anything, (other than Struct specific internal stuff), good luck with that. But who needs other dependencies anyway?


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

But in alot of cases, you might:
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
typedef struct {
    s32 bar;
} Foo;

typedef struct {
    _Array_Header_;
    Foo *items;
} Foo_Array;

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

b32 result = String_Eq(my_string, S("Hello World"));
b32 result = String_Starts_With(my_string, S("Hello"));
b32 result = String_Ends_With  (my_string, S("World"));

b32 result = String_Contains_Char(my_string, " ");

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


#define MILISECONDS_PER_SECOND          THOUSAND
#define MICROSECONDS_PER_SECOND         MILLION
#define NANOSECONDS_PER_SECOND          BILLION

// good for usleep
#define MILISECONDS_PER_MICROSECOND     (MICROSECONDS_PER_SECOND / MILISECONDS_PER_SECOND)



// ===================================================
//         What static really means
// ===================================================

// Mark a function that must be used in the current compilation block and cannot be seen outside of it.
#define internal            static
// mark a variable inside of a function that persists though function calls.
#define local_persist       static
// casey once said that all this dose is help the compiler deal with
// multithreaded code, by forceing the functions to not store the
// value of the variable into their own registers.
#define global_variable     static
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
