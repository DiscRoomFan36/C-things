//
// dynamic_array.h - simple dynamic array
// Credit to tsoding for showing me the way: @Tsoding - https://www.youtube.com/c/Tsoding
//
// Fletcher M - 10/03/2025
//

#ifndef DYNAMIC_ARRAY_H_
#define DYNAMIC_ARRAY_H_

#include <assert.h>

// You can redefine realloc and free.
// take care when useing these if you have multiple compilation units.
#ifndef DA_REALLOC
    #include <stdlib.h>
    #define DA_REALLOC(old_ptr, old_size, new_size)    realloc((old_ptr), (new_size))
    #define DA_FREE(ptr)                               free(ptr)
#endif


#define DA_INIT_CAP 32

#define da_append(da, item)                                                                                                                           \
    do {                                                                                                                                              \
        if ((da)->count >= (da)->capacity) {                                                                                                          \
            __typeof__((da)->capacity) new_capacity = (da)->capacity == 0 ? DA_INIT_CAP : (da)->capacity*2;                                           \
            (da)->items = (__typeof__((da)->items)) DA_REALLOC((da)->items, (da)->capacity*sizeof(*(da)->items), new_capacity*sizeof(*(da)->items));  \
            assert((da)->items != NULL && "Buy More RAM lol");                                                                                        \
            (da)->capacity = new_capacity;                                                                                                            \
        }                                                                                                                                             \
                                                                                                                                                      \
        (da)->items[(da)->count++] = (item);                                                                                                          \
    } while (0)


#define da_stamp_and_remove(da, index)                      \
    do {                                                    \
        (da)->items[(index)] = (da)->items[(da)->count-1];  \
        (da)->count -= 1;                                   \
    } while (0)


#define da_free(da)                             \
    do {                                        \
        if ((da)->items) DA_FREE((da)->items);  \
        (da)->items    = 0;                     \
        (da)->count    = 0;                     \
        (da)->capacity = 0;                     \
    } while (0)


#define da_free_items(da)                        \
    do {                                         \
        for (u64 i = 0; i < (da)->count; i++) {  \
            DA_FREE((da)->items[i]);             \
        }                                        \
        (da)->count = 0;                         \
    } while (0)


#endif // DYNAMIC_ARRAY_H_
