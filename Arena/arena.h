//
// arena.h
//
// DiscRoomFan
// created  - 03/03/2025
// modified - 16/05/2025
//
// This is originally inspired by:
// - https://github.com/tsoding/arena
//
// Implemented Arena Data-Structure
//

#ifndef ARENA_H_
#define ARENA_H_


// for number definitions, aka s64 and u64
#include "ints.h"


#ifndef ARENA_ASSERT
    #include <assert.h>
    #define ARENA_ASSERT assert
#endif

#ifndef ARENA_REGION_DEFAULT_CAPACITY
    #define ARENA_REGION_DEFAULT_CAPACITY (8*1024)
#endif // ARENA_REGION_DEFAULT_CAPACITY


// When you return a chunk of memory to the user, its important that the data is aligned to a
// register / pointer boundary. This means that when the user goes to read or write their ints
// or something, they dont have to do a stupid read across registers / memory locations, to ge a single number.
// If you just did the most 'efficient' thing and alloc just the right amount of data and return the next section,
// you could not be aligned with a register boundary, and thus it would take a different kind of read by the CPU
// to get the memory to you, and to send it back down. This size should be something friendly to the CPU,
// (thats why we don't just use char or u8 here.)
//
// I use a 64-bit number here, (not just whatever the 'int' type is), so its easily known and mostly future proof.
// If you need 128-bit aligned pointers, just modify this file, or just only use the aligned parts,
// and waste a bit of memory, its fine to do so, this arena dose it all the time.
typedef u64   boundary_aligned_type;


typedef struct Region {
    struct Region *next;
    s64 count;
    s64 capacity;

    // used in Arena_free()
    bool32 do_not_free_this;
    // extra padding bytes.
    u8 padding[4];

    boundary_aligned_type data[];
} Region;

typedef struct Arena {
    Region *first, *last;

    // how much to allocate when a new Region is required,
    // == 0: ARENA_REGION_DEFAULT_CAPACITY
    //  > 0: is how much to allocate, in number of boundary_aligned_type's,
    //  < 0: is gonna assert, cuz you probably didn't set this bit of memory correctly.
    s64 minimum_allocation_size;

    // TODO the following flags should be put into an enum flag.

    // If this is set to True, (anything thats not zero),
    // when performing allocating functions, (such as Arena_alloc),
    // it may instead return null when trying to allocate memory for a new Region,
    // instead of calling its panic function, (by default panic is just assert(false))
    //
    // You may want this if you want your programs to be bullet proof,
    // to not panic at the first sign of trouble.
    bool32 dont_panic_when_allocation_failure;

    // If this is set to True, if it runs out of memory in its current Region, it will panic.
    // Note. Arena_initialize_first_page() will still trigger this function.
    //
    // Uou may want this if you want more control over when your program allocates,
    // and want your program to have a fixed amount of memory usage.
    bool32 panic_when_trying_to_allocate_new_page;
} Arena;


// Initialize's the first page, so you can know when this first malloc happens.
// Will do nothing if the first page is already created.
//
// If 'first_page_size_in_bytes' is set to 0, the default is used, see 'minimum_allocation_size' comment above.
//
// 'first_page_size_in_bytes' is for creating a Temporary arena, with the first page being absolutely massive, to reduce calls to malloc.
//
// 'first_page_size_in_bytes' will be rounded up to the nearest 'boundary_aligned_type'
void Arena_initialize_first_page(Arena *a, s64 first_page_size_in_bytes);

// Sets the last Region (or the first Region if the arena is not yet initalized),
// to the provided buffer.
//
// This is useful if you dont want the arena to allocate at all, just provide a large buffer,
// and set the 'panic_when_trying_to_allocate_new_page' flag, and now this arena will never allocate.
//
// Care has been taken, so that when Arena_free is called, the pointer to the buffer provided here will not be free'd.
void Arena_add_buffer_as_storeage_space(Arena *a, void *buffer, s64 buffer_size_in_bytes);

// alloc some memory for use in an arena, uninitialized
void *Arena_alloc(Arena *a, s64 bytes);
// alloc some memory for use in an arena, initalized to 0
void *Arena_calloc(Arena *a, s64 bytes);
// realloc and move some memory, dose not free the old pointer.
// (because it most likely came from this arena.)
//
// old_size is needed because we do not keep track of allocations like malloc dose,
// and need it to move the old values.
//
// (NOTE. This function acts like malloc if old_ptr is NULL, and old_size will be unused)
void *Arena_realloc(Arena *a, void *old_ptr, s64 old_size, s64 new_size);

// resets the arena, keeps the memory.
void Arena_reset(Arena *a);
// free the memory.
void Arena_free(Arena *a);



#ifndef ARENA_DA_INIT_CAP
    // if there was already something set, use it.
    #ifdef DA_INIT_CAP
        #define ARENA_DA_INIT_CAP DA_INIT_CAP
    #else
        // else just use the default
        #define ARENA_DA_INIT_CAP 32
    #endif
#endif


#define arena_da_append(a, da, item)                                                                                               \
    do {                                                                                                                           \
        if ((da)->count >= (da)->capacity) {                                                                                       \
            __typeof__((da)->capacity) old_cap = (da)->capacity*sizeof(*(da)->items);                                              \
            (da)->capacity = (da)->capacity == 0 ? ARENA_DA_INIT_CAP : (da)->capacity*2;                                           \
            (da)->items = (__typeof__((da)->items)) Arena_realloc((a), (da)->items, old_cap, (da)->capacity*sizeof(*(da)->items)); \
            ARENA_ASSERT((da)->items != NULL && "Buy More RAM lol");                                                               \
        }                                                                                                                          \
                                                                                                                                   \
        (da)->items[(da)->count++] = (item);                                                                                       \
    } while (0)


#endif // ARENA_H_


#ifdef ARENA_IMPLEMENTATION

#ifndef ARENA_IMPLEMENTATION_GUARD
#define ARENA_IMPLEMENTATION_GUARD


// For 'malloc' and 'free'.
// We should remove this and use something like
//
// 'ARENA_BASE_ALLOC' and 'ARENA_BASE_FREE'
//
// macros, so you could replace them with something else,
// and just have the default use stdlib's 'malloc' and 'free'
//
// This is also why we dont use memcpy and memset elsewhere, for some future proofing.
#include <stdlib.h>


// marks functions that only exists within this file.
#define local static

// This is a well known pattern for compilers.
// It will probably be replaced by the real memcpy in a release build.
local void *arena_memcpy(void *dest, const void *src, s64 n) {
    char *d = dest;
    const char *s = src;
    for (; n; n--) *d++ = *s++;
    return d;
}

// This is a well known pattern for compilers, ditto above.
local void *arena_memset(void *dest, u8 c, s64 n) {
    u8 *d = dest;
    for (; n; n--) *d++ = c;
    return dest;
}


// makes a new Region, may return null.
// Uses malloc.
local Region *new_region(s64 capacity) {
    // Region uses come of the end thing.
    Region *new_region = malloc(sizeof(Region) + sizeof(boundary_aligned_type)*capacity);

    if (new_region) {
        new_region->next     = NULL;
        new_region->count    = 0;
        new_region->capacity = capacity;
        // this should be free'd because it's been malloc'd
        new_region->do_not_free_this = False;
    }

    return new_region;
}
// Uses free.
local void free_region(Region *r) {
    if (!r->do_not_free_this) {
        free(r);
    }
}


void Arena_initialize_first_page(Arena *a, s64 first_page_size_in_bytes) {
    if (a->first != NULL) return;

    // just a funny hack.
    s64 tmp_min_alloc_size = a->minimum_allocation_size;
    a->minimum_allocation_size = first_page_size_in_bytes;

    Arena_alloc(a, 0);

    a->minimum_allocation_size = tmp_min_alloc_size;

    // if (first_page_size_in_bytes == 0) {
    //     // just let arena alloc take care of it.
    //     Arena_alloc(a, 0);
    // } else {

    //     // get boundary_aligned_type number of bytes
    //     s64 size = (first_page_size_in_bytes + sizeof(boundary_aligned_type) - 1) / sizeof(boundary_aligned_type);
    //     // allocate a new region with size.
    //     a->first = new_region(size);
    //     a->last = a->first;
    // }
}


void Arena_add_buffer_as_storeage_space(Arena *a, void *buffer, s64 buffer_size_in_bytes) {
    ARENA_ASSERT(buffer != NULL && "why did you pass this to us.");
    ARENA_ASSERT(buffer_size_in_bytes >= 0 && "must have positive buffer size");

    ARENA_ASSERT((u64) buffer_size_in_bytes > sizeof(Region) && "The passed in buffer must be big enough to contain the Region, preferably much bigger");

    s64 real_allocatable_space = buffer_size_in_bytes - sizeof(Region);

    // this is rounded down so we dont overrun the buffer.
    s64 size_in_boundary_aligned = real_allocatable_space / sizeof(boundary_aligned_type);

    Region *new_region = buffer;
    new_region->count = 0;
    new_region->capacity = size_in_boundary_aligned;
    new_region->next = NULL;

    // the user will free this (or its some static memory)
    new_region->do_not_free_this = True;

    // check if this is the only memory in the arena.
    if (a->last == NULL) {
        ARENA_ASSERT(a->first == NULL);

        a->first = new_region;
        a->last  = new_region;

        return;
    }

    // find the last Region.
    Region *p;
    for (p = a->last; p->next != NULL; p = p->next);

    // set new last.
    p->next = new_region;
}


void *Arena_alloc(Arena *a, s64 bytes) {
    // round up the the nearest boundary aligned size
    // sizeof(boundary_aligned_type) is a power of 2, so hopefully this "/" doesn't actually exist, and is just a right shift
    s64 size = (bytes + sizeof(boundary_aligned_type) - 1) / sizeof(boundary_aligned_type);

    // get the default size of the next region.
    //     <0 is an error,
    //     0 is the macro 'ARENA_REGION_DEFAULT_CAPACITY',
    //     and >0 is your own custom size.
    ARENA_ASSERT(a->minimum_allocation_size >= 0 && "minimum allocation size must be greater than or equal to zero");
    s64 default_size = (a->minimum_allocation_size == 0) ? ARENA_REGION_DEFAULT_CAPACITY : a->minimum_allocation_size;

    s64 to_alloc_if_no_room = (size > default_size) ? size : default_size;

    // aka there is no memory yet.
    if (a->last == NULL) {
        ARENA_ASSERT(a->first == NULL);

        if (a->panic_when_trying_to_allocate_new_page) {
            ARENA_ASSERT(False && "Arena_alloc: attempted to allocate new memory, but that has been disallowed. (when there was no memory to begin with.)");
        }

        a->first = new_region(to_alloc_if_no_room);
        if (a->first == NULL) {
            if (a->dont_panic_when_allocation_failure) return NULL;
            ARENA_ASSERT(a->first && "Arena_alloc: attempted to allocate new memory, got null. (when there was no memory to begin with.)");
        }
        a->last = a->first;

        a->last->count = size;
        return a->last->data;
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

        if (a->panic_when_trying_to_allocate_new_page) {
            ARENA_ASSERT(False && "Arena_alloc: attempted to allocate new memory, but that has been disallowed.");
        }

        Region *last_last = a->last;

        a->last = new_region(to_alloc_if_no_room);
        if (a->last == NULL) {
            if (a->dont_panic_when_allocation_failure) return NULL;
            ARENA_ASSERT(a->last && "Arena_alloc: attempted to allocate new memory, got null.");
        }
        a->last->count = size;

        last_last->next = a->last;
        return a->last->data;
    }
}

void *Arena_calloc(Arena *a, s64 bytes) {
    void *memory = Arena_alloc(a, bytes);
    // we actually miss the trailing bytes with this, but who cares.
    arena_memset(memory, 0, bytes);
    return memory;
}


void *Arena_realloc(Arena *a, void *old_ptr, s64 old_size, s64 new_size) {
    if (new_size <= old_size) return old_ptr;

    void *new_ptr = Arena_alloc(a, new_size);

    // explicit check for null, old version just let memcpy do its thing,
    // and it "worked" but it would probably break in some hard to figure out way
    if (old_ptr != NULL && old_size != 0) {
        // memcpy the old stuff
        arena_memcpy(new_ptr, old_ptr, old_size);
    }

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


#endif // ARENA_IMPLEMENTATION_GUARD

#endif // ARENA_IMPLEMENTATION
