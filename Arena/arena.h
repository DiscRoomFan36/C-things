//
// arena.h
//
// DiscRoomFan
// created  - 03/03/2025
// modified - 22/04/2025
//
// This is inspired by / taken from:
// - https://github.com/tsoding/arena
//
// Implemented Arena Data-Structure
//

#ifndef ARENA_H_
#define ARENA_H_


// for number definitions, aka s64
#include "ints.h"


#ifndef ARENA_ASSERT
#include <assert.h>
#define ARENA_ASSERT assert
#endif

#ifndef ARENA_REGION_DEFAULT_CAPACITY
#define ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY

typedef struct Region Region;

struct Region {
    Region *next;
    s64 count;
    s64 capacity;
    uintptr_t data[];
};

typedef struct Arena {
    Region *first, *last;
} Arena;


// alloc some memory for use in an arena
void *Arena_alloc(Arena *a, s64 bytes);
// realloc and move some memory, dose not free the old pointer.
void *Arena_realloc(Arena *a, void *old_ptr, s64 old_size, s64 new_size);
// resets the arena, keeps the memory.
void Arena_reset(Arena *a);
// free the memory.
void Arena_free(Arena *a);



#ifndef ARENA_DA_INIT_CAP
// if there was already something set, use it.
# ifdef DA_INIT_CAP
#  define ARENA_DA_INIT_CAP DA_INIT_CAP
# else
// else just use the default
#  define ARENA_DA_INIT_CAP 32
# endif
#endif


#define arena_da_append(a, da, item)                                                                                               \
    do {                                                                                                                           \
        if ((da)->count >= (da)->capacity) {                                                                                       \
            __typeof__((da)->capacity) old_cap = (da)->capacity*sizeof(*(da)->items);                                              \
            (da)->capacity = (da)->capacity == 0 ? ARENA_DA_INIT_CAP : (da)->capacity*2;                                           \
            (da)->items = (__typeof__((da)->items)) Arena_realloc((a), (da)->items, old_cap, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy More RAM lol");                                                                     \
        }                                                                                                                          \
                                                                                                                                   \
        (da)->items[(da)->count++] = (item);                                                                                       \
    } while (0)


#endif // ARENA_H_


#ifdef ARENA_IMPLEMENTATION

// for 'malloc'
#include <stdlib.h>

// other helpers, not in header

void *arena_memcpy(void *dest, const void *src, s64 n) {
    char *d = dest;
    const char *s = src;
    for (; n; n--) *d++ = *s++;
    return d;
}


// Guarantees a new Region, (Because it asserts)
Region *new_region(s64 capacity) {
    // Region uses come of the end thing.
    Region *new_region = malloc(sizeof(Region) + sizeof(uintptr_t)*capacity);
    ARENA_ASSERT(new_region);

    new_region->next     = NULL;
    new_region->count    = 0;
    new_region->capacity = capacity;

    return new_region;
}
void free_region(Region *r) {
    free(r);
}


void *Arena_alloc(Arena *a, s64 size_bytes) {
    // round up the the nearest ptr size
    s64 size = (size_bytes + sizeof(uintptr_t) - 1) / sizeof(uintptr_t);

    s64 to_alloc_if_no_room = (size > ARENA_REGION_DEFAULT_CAPACITY) ? size : ARENA_REGION_DEFAULT_CAPACITY;

    if (a->last == NULL) {
        ARENA_ASSERT(a->first == NULL);

        a->first = new_region(to_alloc_if_no_room);
        a->last = a->first;

        a->last->count = size;
        return &a->last->data[0];
    }

    // find room, or find the end
    while ((a->last->count + size > a->last->capacity) && (a->last->next != NULL)) {
        a->last = a->last->next;
    }

    if (a->last->count + size <= a->last->capacity) {
        // if there is space, alloc
        a->last->count += size;
        return &a->last->data[a->last->count - size];

    } else {
        // else we need a new region
        ARENA_ASSERT(a->last->next == NULL);

        Region *last_last = a->last;

        a->last = new_region(to_alloc_if_no_room);
        a->last->count = size;

        last_last->next = a->last;
        return a->last->data;
    }
}

void *Arena_realloc(Arena *a, void *old_ptr, s64 old_size, s64 new_size) {
    if (new_size <= old_size) return old_ptr;

    void *new_ptr = Arena_alloc(a, new_size);

    // memcpy the old stuff
    arena_memcpy(new_ptr, old_ptr, old_size);

    return new_ptr;
}

void Arena_reset(Arena *a) {
    if (a->first == NULL) return;

    // for all the region's between first and last
    for (Region *r = a->first; r != a->last; r = r->next) {
        r->count = 0;
    }
    a->last->count = 0;

    // and reset the last pointer.
    a->last = a->first;
}
void Arena_free(Arena *a) {
    // this is slightly more complicated than it would be,
    // but you cant use the thing you just free'd, for some reason,
    // because it get's messed with by 'free'
    Region *r = a->first;
    while (r) {
        Region *next = r->next;
        free_region(r);
        r = next;
    }

    a->first = NULL;
    a->last  = NULL;
}


#endif // ARENA_IMPLEMENTATION
