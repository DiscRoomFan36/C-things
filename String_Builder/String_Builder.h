//
// String_Builder.h - An efficient implementation of a the String Builder Data-structure.
//
// By Fletcher M.
//
// Created  : 21/05/2025
// Modified : 12/07/2025
//

#ifndef STRING_BUILDER_H_
#define STRING_BUILDER_H_


#include "ints.h"
// I _could_ put this under a #ifdef, but who cares. String_view.h is good.
// String_Builder only relies on the definition of a SV struct anyway. not any of the functions.
#include "String_View.h"


// How many segments in a single Segment Structure.
// This Should be a power of 2 for speed. (see maybe_expand_to_fit: sb->buffer_index % STRING_BUILDER_NUM_BUFFERS == 0)
//
// (Not putting this one in an ifndef, if you want to change this, change this.)
#define STRING_BUILDER_NUM_BUFFERS 32


// Implementation defines you might be interested in.

// how much to alloc by default when a new segment of memory is needed.
#ifndef STRING_BUILDER_BUFFER_DEFAULT_SIZE
    // 4096 bytes is equal to the default page size in windows and linux. probably.
    #define STRING_BUILDER_BUFFER_DEFAULT_SIZE 4096
#endif

// if you want to define your own 'malloc' and 'free'
#ifndef STRING_BUILDER_MALLOC
    #include <stdlib.h>
    #define STRING_BUILDER_MALLOC(size) malloc(size)
    #define STRING_BUILDER_FREE(ptr)    free(ptr)
#endif

// This macro is called when something goes wrong. Recommended to just use assert.
// But in some cases assert can be compiled out, if this happens, the program will still sortof function.
//
// If the macro dose nothing, (or doesn't exist,) the results of some operations will return NULL/base values.
// or do no work at all. use at your own risk.
#ifndef STRING_BUILDER_PANIC
    #include <assert.h>
    #define STRING_BUILDER_PANIC(error_text) assert(False && error_text)
#endif


// if you dont want a dependency on stdio.h, define this.
// NOTE. functions like SB_printf relies on stdio.h
#ifndef SB_NO_STDIO
#define STRING_BUILDER_USE_STDIO_
#endif


// -----------------------
//     Data-structures
// -----------------------


// String Builder internal structure
// holds the segments of the being built string.
typedef struct Character_Buffer {
    char *data;
    s64 count;
    s64 capacity;
} Character_Buffer;

// String Builder internal structure
// Holds a bunch character buffers, and a pointer to the next Segment as well,
// Acts like a linked list node.
typedef struct Segment {
    // the next segment in a linked list
    struct Segment *next;

    // all the buffers that hold the parts of a string.
    Character_Buffer buffers[STRING_BUILDER_NUM_BUFFERS];
} Segment;


// A data-structure that can efficiently grow a string.
// good for medium to large strings. small strings (< 4096 bytes) are not recommended, as this structure will allocate once.
typedef struct String_Builder {
    // how much to allocate when a new segment is needed.
    s64 base_new_allocation;


    // the current segment were working on.
    // if its pointing to NULL, (aka you just zero initalized it), it will be set to a pointer to 'first_segment_holder'
    Segment *current_segment;

    // last buffer in use.
    s64 buffer_index;
    // the first in a linked list.
    Segment first_segment_holder;
} String_Builder;


// -----------------------------------------
//        String Builder Functions
// -----------------------------------------

// current size of the string being built.
//
// this calculation runs though all of the segments, so dont spam it maybe.
s64 SB_count(String_Builder *sb);
// how much space the builder has.
// NOTE. some space will be wasted, for performance reasons.
s64 SB_capacity(String_Builder *sb);

// malloc's a new string with all the stuff in the builder,
//
// A trailing NULL byte is appended, so you can pass it into functions that expect a C String
SV SB_to_SV(String_Builder *sb);

// reset the String Builder, ready to be used again
void SB_reset(String_Builder *sb);
// free()'s all the memory that the String builder allocated.
// NOTE. This dose NOT free the pointer to sb. It's probably stack/globally allocated.
void SB_free(String_Builder *sb);

// append a single pointer and size, dose not check for NULL byte
void SB_add_pointer_and_size(String_Builder *sb, char *ptr, s64 size);
// append a single C String
void SB_add_C_str(String_Builder *sb, const char *c_str);
// append a single String View, dose not check for NULL byte
void SB_add_SV(String_Builder *sb, SV sv);


// functions that use the C standard library,
// Define NO_STDIO to remove this dependency.
#ifdef STRING_BUILDER_USE_STDIO_

#include <stdio.h> // for FILE, and printf's (i dont want to make my own right now.)

// Appends the string buffer to a file.
//
// Dose not write a trailing NULL byte, (as apposed to SB_to_SV)
void SB_to_file(String_Builder *sb, FILE *file);


// printf into the string builder
s64 SB_printf(String_Builder *sb, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

#endif // STRING_BUILDER_USE_STDIO_


#endif // STRING_BUILDER_H_



#ifdef STRING_BUILDER_IMPLEMENTATION

#ifndef STRING_BUILDER_IMPLEMENTATION_GUARD_
#define STRING_BUILDER_IMPLEMENTATION_GUARD_


#define local static


// ----------------------
//    helper functions
// ----------------------

local void SB_memcpy(char *dest, const char *src, s64 n) {
    for (s64 i = 0; i < n; i++) dest[i] = src[i];
}
local s64 SB_strlen(const char *str) {
    s64 n = 0;
    while (*str++) n++;
    return n;
}
local void SB_memset(void *dest, u8 to_set, s64 n) {
    char *d = dest;
    for (s64 i = 0; i < n; i++) d[i] = to_set;
}

local void free_buffer(Character_Buffer *buffer) {
    if (buffer->data) {
        STRING_BUILDER_FREE(buffer->data);
        buffer->data     = NULL;
        buffer->count    = 0;
        buffer->capacity = 0;
    }
}


// Might grow the string builder.
// Returns a pointer to a buffer that has enough space.
local Character_Buffer *maybe_expand_to_fit(String_Builder *sb, s64 size) {
    // no need to null check sb here, only null check where the user could reach.

    if (size < 0) {
        STRING_BUILDER_PANIC("negative size passed into maybe_expand_to_fit()");
        return NULL; // gotta NULL check every call site. only needed when assert is compiled out.
    }

    // set the current segment to the first segment if its null (aka just after zero initialization)
    if (sb->current_segment == NULL) sb->current_segment = &sb->first_segment_holder;

    // now, check all the segments to see if they have space,
    Character_Buffer *last_buffer = &sb->current_segment->buffers[sb->buffer_index % STRING_BUILDER_NUM_BUFFERS];
    while (True) {
        // if the segment hasn't been allocated
        if (last_buffer->count == 0) break;

        // if the segent has enough room to hold the new thing
        if (size + last_buffer->count <= last_buffer->capacity) break;

        // else move onto the next segment
        sb->buffer_index += 1;

        if (sb->buffer_index % STRING_BUILDER_NUM_BUFFERS == 0) {
            // we ran off the end. use linked list pointer
            if (!sb->current_segment->next) {
                // we need to make a new segment holder
                // make sure to init to 0
                sb->current_segment->next = STRING_BUILDER_MALLOC(sizeof(Segment));

                if (!sb->current_segment->next) {
                    STRING_BUILDER_PANIC("String Builder - maybe_expand_to_fit: failed to malloc a new segment holder");
                    return NULL;
                }

                SB_memset(sb->current_segment->next, 0, sizeof(Segment));
            }
            sb->current_segment = sb->current_segment->next;
        }

        last_buffer = &sb->current_segment->buffers[sb->buffer_index % STRING_BUILDER_NUM_BUFFERS];
    }


    if (last_buffer->capacity == 0) {
        if (sb->base_new_allocation < 0) {
            STRING_BUILDER_PANIC("Base new allocation < 0 is not allowed.");
            return NULL;
        }

        // if base_new_allocation is not zero, use it, else use default macro.
        s64 default_size = (sb->base_new_allocation > 0) ? sb->base_new_allocation : STRING_BUILDER_BUFFER_DEFAULT_SIZE;
        s64 to_add_size = (size < default_size) ? default_size : size;

        last_buffer->data = STRING_BUILDER_MALLOC(to_add_size);

        if (!last_buffer->data) {
            STRING_BUILDER_PANIC("maybe_expand_to_fit: failed to malloc a new segment with to_add_size");
            return NULL;
        }

        last_buffer->count = 0;
        last_buffer->capacity = to_add_size;
    }

    return last_buffer;
}



// ---------------------------------------------
//    String Builder Function Implementations
// ---------------------------------------------

s64 SB_count(String_Builder *sb) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_count()");
        return 0;
    }

    s64 count = 0;

    { // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
        s64 buffer_index = sb->buffer_index;
        Segment *current_segment = &sb->first_segment_holder;
        // get the stuff in the full segments.
        while (buffer_index > STRING_BUILDER_NUM_BUFFERS) {
            for (s64 i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
                Character_Buffer *buffer = &current_segment->buffers[i];
                count += buffer->count;
            }
            buffer_index -= STRING_BUILDER_NUM_BUFFERS;
            current_segment = current_segment->next;

            if (!current_segment) {
                STRING_BUILDER_PANIC("SB_count: something internal went wrong.");
                return 0;
            }
        }
        // finish off the non full segment
        for (s64 i = 0; i <= buffer_index; i++) {
            Character_Buffer *buffer = &current_segment->buffers[i];
            count += buffer->count;
        }
    }

    return count;
}

s64 SB_capacity(String_Builder *sb) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_capacity()");
        return 0;
    }

    s64 capacity = 0;

    Segment *current_segment = &sb->first_segment_holder;
    while (current_segment) {
        for (size_t i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
            Character_Buffer *buffer = &current_segment->buffers[i];
            capacity += buffer->capacity;
        }
        current_segment = current_segment->next;
    }

    return capacity;
}


SV SB_to_SV(String_Builder *sb) {
    SV result = {0};

    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_to_SV()");
        return result;
    }

    s64 new_string_size = SB_count(sb);

    result.data = STRING_BUILDER_MALLOC(new_string_size+1); // +1 for null byte, (for c_string compatiblity)
    if (!result.data) {
        STRING_BUILDER_PANIC("SB_malloc failed when trying to allocate enough memory to hold to result string from SB_to_SV()");
        return result;
    }

    result.size = new_string_size;

    // put all the strings into the new buffer.
    s64 index = 0;
    { // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
        s64 buffer_index = sb->buffer_index;
        Segment *current_segment = &sb->first_segment_holder;
        // get the stuff in the full segments.
        while (buffer_index > STRING_BUILDER_NUM_BUFFERS) {
            for (s64 i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
                Character_Buffer *buffer = &current_segment->buffers[i];
                SB_memcpy(result.data + index, buffer->data, buffer->count);
                index += buffer->count;
            }
            buffer_index -= STRING_BUILDER_NUM_BUFFERS;
            current_segment = current_segment->next;

            if (!current_segment) {
                STRING_BUILDER_PANIC("SB_to_SV: something internal went wrong.");
                STRING_BUILDER_FREE(result.data);
                result.data = NULL;
                result.size = 0;
                return result;
            }
        }
        // finish off the non full segment
        for (s64 i = 0; i <= buffer_index; i++) {
            Character_Buffer *buffer = &current_segment->buffers[i];
            SB_memcpy(result.data + index, buffer->data, buffer->count);
            index += buffer->count;
        }
    }

    if (index != new_string_size) {
        STRING_BUILDER_PANIC("SB_to_SV: something internal went wrong (2)");
        // return; // we just fall through here, something bad is already going on, at least you get your string back I guess?
    }
    result.data[index] = '\0'; // Null byte for c string's

    return result;
}


void SB_reset(String_Builder *sb) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_reset()");
        return;
    }

    // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
    s64 buffer_index = sb->buffer_index;
    Segment *current_segment = &sb->first_segment_holder;
    // get the stuff in the full segments.
    while (buffer_index > STRING_BUILDER_NUM_BUFFERS) {
        for (s64 i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
            Character_Buffer *buffer = &current_segment->buffers[i];
            buffer->count = 0;
        }
        buffer_index -= STRING_BUILDER_NUM_BUFFERS;
        current_segment = current_segment->next;

        if (!current_segment) {
            STRING_BUILDER_PANIC("SB_reset: something internal went wrong.");
            return;
        }
    }
    // finish off the non full segment
    for (s64 i = 0; i <= buffer_index; i++) {
        Character_Buffer *buffer = &current_segment->buffers[i];
        buffer->count = 0;
    }

    sb->buffer_index = 0;
    sb->current_segment = &sb->first_segment_holder;
}

void SB_free(String_Builder *sb) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_free()");
        return;
    }

    { // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
        s64 buffer_index = sb->buffer_index;
        Segment *current_segment = &sb->first_segment_holder;
        // get the stuff in the full segments.
        while (buffer_index > STRING_BUILDER_NUM_BUFFERS) {
            for (s64 i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
                Character_Buffer *buffer = &current_segment->buffers[i];
                free_buffer(buffer);
            }
            buffer_index -= STRING_BUILDER_NUM_BUFFERS;
            current_segment = current_segment->next;

            if (!current_segment) {
                STRING_BUILDER_PANIC("SB_free: something internal went wrong.");
                return;
            }
        }
        // finish off the non full segment
        for (s64 i = 0; i <= buffer_index; i++) {
            Character_Buffer *buffer = &current_segment->buffers[i];
            free_buffer(buffer);
        }
    }

    { // free the segment pointers
        Segment *current_segment = sb->first_segment_holder.next;
        while (current_segment) {
            Segment *tmp = current_segment->next;
            STRING_BUILDER_FREE(current_segment);
            current_segment = tmp;
        }
    }

    sb->buffer_index = 0;
    // so that one branch in maybe_expand_to_fit has less work
    sb->current_segment = &sb->first_segment_holder;
}



void SB_add_pointer_and_size(String_Builder *sb, char *ptr, s64 size) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_add_pointer_and_size()");
        return;
    }

    if (size < 0) {
        STRING_BUILDER_PANIC("negative 'size' passed into SB_add_pointer_and_size()");
        return;
    }

    // if it has zero size, its fine for 'ptr' to be null
    if (size == 0) return;

    if (!ptr) {
        STRING_BUILDER_PANIC("NULL 'ptr' passed into SB_add_pointer_and_size(), when 'size' was not zero.");
        return;
    }

    Character_Buffer *buffer = maybe_expand_to_fit(sb, size);

    if (!buffer) {
        return; // maybe_expand_to_fit has already raised the panic.
    }

    SB_memcpy(buffer->data + buffer->count, ptr, size);
    buffer->count += size;
}

void SB_add_C_str(String_Builder *sb, const char *c_str) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_add_C_str()");
        return;
    }
    if (!c_str) {
        STRING_BUILDER_PANIC("NULL 'c_ptr' passed into SB_add_C_str()");
        return;
    }

    SB_add_pointer_and_size(sb, (char*)c_str, SB_strlen(c_str));
}

void SB_add_SV(String_Builder *sb, SV sv) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_add_SV()");
        return;
    }
    if (sv.size < 0) {
        STRING_BUILDER_PANIC("negative 'size' passed into SB_add_SV()");
        return;
    }

    SB_add_pointer_and_size(sb, sv.data, sv.size);
}


#ifdef STRING_BUILDER_USE_STDIO_

// stdio is already in the header
#include <stdarg.h> // for va_arg's in printf like functions


void SB_to_file(String_Builder *sb, FILE *file) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_to_file()");
        return;
    }
    if (!file) {
        STRING_BUILDER_PANIC("NULL 'file' passed into SB_to_file()");
        return;
    }

    // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
    s64 buffer_index = sb->buffer_index;
    Segment *current_segment = &sb->first_segment_holder;
    // get the stuff in the full segments.
    while (buffer_index > STRING_BUILDER_NUM_BUFFERS) {
        for (s64 i = 0; i < STRING_BUILDER_NUM_BUFFERS; i++) {
            Character_Buffer *buffer = &current_segment->buffers[i];
            fwrite(buffer->data, sizeof(char), buffer->count, file);
        }
        buffer_index -= STRING_BUILDER_NUM_BUFFERS;
        current_segment = current_segment->next;

        if (!current_segment) {
            STRING_BUILDER_PANIC("SB_to_file: something internal went wrong.");
            return;
        }
    }

    // finish off the non full segment
    for (s64 i = 0; i <= buffer_index; i++) {
        Character_Buffer *buffer = &current_segment->buffers[i];
        fwrite(buffer->data, sizeof(char), buffer->count, file);
    }
}

s64 SB_printf(String_Builder *sb, const char *format, ...) {
    if (!sb) {
        STRING_BUILDER_PANIC("NULL 'sb' passed into SB_printf()");
        return 0;
    }
    if (!format) {
        STRING_BUILDER_PANIC("NULL 'format' passed into SB_printf()");
        return 0;
    }

    va_list args;

    va_start(args, format);
        s64 formatted_size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    // early out.
    if (formatted_size == 0) return 0;

    // +1 because printf also puts a trailing '\0' byte.
    // we ignore that for the rest of the code.
    Character_Buffer *buffer = maybe_expand_to_fit(sb, formatted_size+1);

    if (!buffer) {
        return 0; // maybe_expand_to_fit has already raised the panic.
    }

    va_start(args, format);
        vsprintf(buffer->data + buffer->count, format, args);
    va_end(args);

    buffer->count += formatted_size;

    return formatted_size;
}

#endif // STRING_BUILDER_USE_STDIO_



#endif // STRING_BUILDER_IMPLEMENTATION_GUARD_

#endif // STRING_BUILDER_IMPLEMENTATION

