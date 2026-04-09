
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

*TODO explain*



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

although Clamp dose collide with raylib's clamp.

*TODO explain*


### Nice #defines

*TODO explain*


### Just Some Nice Functions

Read_Entire_File()

nanoseconds_since_unspecified_epoch()

debug()

debug_break()

*TODO*


## TODO

- Finish the hashmap implementation.
- replace *_test.c files with a TESTMA.h, file.
