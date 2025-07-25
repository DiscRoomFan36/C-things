//
// String_Helpers.h - better strings
//
// Author   - Fletcher M
//
// Created  - 12/07/2025
// Modified - 25/07/2025
//


#ifndef STRING_HELPER_H
#define STRING_HELPER_H



#ifndef INTS_DOT_H
#define INTS_DOT_H

    //
    // ints.h - just some ints that are nice to have
    //
    // Author   - Fletcher M
    //
    // Created  - 21/03/2025
    // Modified - 11/07/2025
    //

    #include <stdint.h>

    typedef uint64_t   u64;
    typedef uint32_t   u32;
    typedef uint16_t   u16;
    typedef uint8_t    u8;

    typedef int64_t    s64;
    typedef int32_t    s32;
    typedef int16_t    s16;
    typedef int8_t     s8;

    typedef u32        bool32;

    typedef float      f32;
    typedef double     f64;

    #define True       (0 == 0)
    #define False      (0 != 0)

#endif // INTS_DOT_H



// Formating for printf()
#define SV_Fmt "%.*s"
#define SV_Arg(sv) (int) (sv).size, (sv).data 
// Example: printf("my_sv = "SV_Fmt"\n", SV_Arg(my_sv));


// if you want to define your own 'malloc' and 'free'
#ifndef STRING_HELPER_MALLOC
    #include <stdlib.h>
    #define STRING_HELPER_MALLOC(size)    malloc(size)
    #define STRING_HELPER_FREE(ptr)       free(ptr)
#endif


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

typedef struct SV {
    s64 size;
    char *data;
} SV;


typedef struct SV_Array {
    SV *items;
    s64 count;
    s64 capacity;
} SV_Array;


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



// -----------------------
//    Function headers
// -----------------------


// -----------------------------------------
//          String View Functions
// -----------------------------------------

// works the same as strlen,
// returns 0 on null ptr
u64 SV_strlen(const char *str);
// works the same as memset.
void *SV_memset(void *dest, u8 to_set, s64 n);

// functions on String views

// takes a C_Str return a SV, dose not allocate
SV SV_from_C_Str(const char *str);
// duplicate a String View, uses STRING_HELPER_MALLOC, dose not zero terminate the string.
// SV.data could be null if STRING_HELPER_MALLOC fails, (but size will also be 0 so its good?)
SV SV_dup(SV s);

// free the pointer with 'STRING_HELPER_FREE' and sets data to NULL
void SV_free(SV *s);

// transforms a SV in place to uppercase
void SV_To_Upper(SV *s);

// SV equality check
bool32 SV_Eq(SV s1, SV s2);
bool32 SV_Eq_c_str(SV s1, const char *s2);
bool32 SV_starts_with(SV s, SV prefix);
bool32 SV_starts_with_c_str(SV s, const char *prefix);

bool32 SV_contains_char(SV s, char c);
// finds the first occurrence of c in s, -1 on failure
s64 SV_find_index_of_char(SV s, char c);
// finds the first occurrence of needle in s, -1 on failure
// if needle.size == 0, returns -1.
s64 SV_find_index_of(SV s, SV needle);

// finds the line at index, and returns a SV of the line, strips, '\n'
SV SV_get_single_line(SV s, s64 index);

// advance the data, subtract the size, in place.
// dose not check for null, or do any bounds checking. thats on you.
void SV_advance(SV *s, s64 count);
// advance the data, subtract the size, returns a new copy.
// dose not check for null, or do any bounds checking. thats on you.
SV SV_advanced(SV s, s64 count);

typedef bool32 (*char_to_bool)(char);
// returns True if c is one of the ASCII whitespace characters.
bool32 SV_is_space(char c);
// remove ASCII whitespace characters from the right of the SV.
SV SV_trim_right(SV s);
// returns a SV with the front chopped off, according to the test function.
// use SV_is_space to chop the whitespace off of the front.
SV SV_chop_while(SV s, char_to_bool test_char_function);

// gets the next line in parseing, and advances parseing and line_num to match
// bool's toggle removeing comments that start with '#',
// trimming the output (from the right), and skiping over empty lines.
//
// 'result == parseing' signifies the end of the file.
// OR result.size == 0 if skip_empty is toggled.
SV SV_get_next_line(SV *parseing, s64 *line_num, bool32 remove_comments, bool32 trim, bool32 skip_empty);

// TODO more SV functions



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
// macro to easily add a struct into a String_builder.
#define SB_add_struct(sb, struct_ptr) SB_add_pointer_and_size((sb), (void*) (struct_ptr), sizeof(*(struct_ptr)))


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



#endif // STRING_HELPER_H



#ifdef STRING_HELPER_IMPLEMENTATION

#ifndef STRING_HELPER_IMPLEMENTATION_GUARD_
#define STRING_HELPER_IMPLEMENTATION_GUARD_


#define internal static


u64 SV_strlen(const char *str) {
    if (!str) return 0;
    for (u64 i = 0;; i++) {
        if (str[i] == 0) return i;
    }
}

void *SV_memset(void *dest, u8 to_set, s64 n) {
    u8 *d = dest;
    for (s64 i = 0; i < n; i++) d[i] = to_set;
    return (u8*) dest + n;
}


SV SV_from_C_Str(const char *str) {
    SV result = {
        .data = (char *) str,
        .size = SV_strlen(str),
    };
    return result;
}

SV SV_dup(SV s) {
    SV result;
    result.data = STRING_HELPER_MALLOC(s.size);
    if (result.data) {
        result.size = s.size;

        // TODO sv copy?
        for (s64 i = 0; i < s.size; i++) {
            result.data[i] = s.data[i];
        }
    }

    return result;
}


void SV_free(SV *s) {
    if (s->data) { STRING_HELPER_FREE(s->data); }
    s->data = NULL;
    s->size = 0;
}


void SV_To_Upper(SV *s) {
    for (s64 n = 0; n < s->size; n++) {
        if ('a' <= s->data[n] && s->data[n] <= 'z') {
            s->data[n] += 'A' - 'a';
        }
    }
}


bool32 SV_Eq(SV s1, SV s2) {
    if (s1.size != s2.size) return False;
    for (s64 n = 0; n < s1.size; n++) {
        if (s1.data[n] != s2.data[n]) return False;
    }
    return True;
}

bool32 SV_Eq_c_str(SV s1, const char *s2) {
    return SV_Eq(s1, SV_from_C_Str(s2));
}

bool32 SV_starts_with(SV s, SV prefix) {
    if (s.size < prefix.size) return False;
    for (s64 i = 0; i < prefix.size; i++) {
        if (s.data[i] != prefix.data[i]) return False;
    }
    return True;
}

bool32 SV_starts_with_c_str(SV s, const char *prefix) {
    return SV_starts_with(s, SV_from_C_Str(prefix));
}


bool32 SV_contains_char(SV s, char c) {
    return SV_find_index_of_char(s, c) != -1;
}

// TODO: simd? or dose that happen automagically?
s64 SV_find_index_of_char(SV s, char c) {
    for (s64 i = 0; i < s.size; i++) {
        if (s.data[i] == c) return i;
    }
    return -1;
}

s64 SV_find_index_of(SV s, SV needle) {
    if (needle.size == 0) return -1;

    // easy out
    if (needle.size == 1) return SV_find_index_of_char(s, needle.data[0]);

    while (True) {
        s64 index = SV_find_index_of_char(s, needle.data[0]);
        if (index == -1) return -1;

        // check if not enough room for needle
        if (s.size - index < needle.size) return -1;

        bool32 flag = True;
        for (s64 i = 1; i < needle.size; i++) {
            if (s.data[index+i] != needle.data[i]) {
                flag = False;
                break;
            }
        }

        if (flag) return index;

        s.data += index + 1;
        s.size -= index + 1;
    }

    return -1;
}


SV SV_get_single_line(SV s, s64 i) {
    // clamp i if its to big. this handles a lot more errors than your thinking about.
    if (i > s.size) i = s.size;

    SV result = {.data = s.data + i, .size = s.size - i};

    s64 index = SV_find_index_of_char(result, '\n');

    // clamp the length to the far newline
    if (index != -1) { result.size = index; }

    // go back until newline before result.data,
    // and make sure it doesn't go out of bounds
    while (result.data != s.data && result.data[-1] != '\n') {
        result.data -= 1;
        result.size += 1;
    }

    return result;
}


void SV_advance(SV *s, s64 count) {
    s->data += count;
    s->size -= count;
}

SV SV_advanced(SV s, s64 count) {
    return (SV){.data = s.data + count, .size = s.size - count};
}


bool32 SV_is_space(char c) {
    if (c == ' ')  return True;
    if (c == '\f') return True;
    if (c == '\n') return True;
    if (c == '\r') return True;
    if (c == '\t') return True;
    if (c == '\v') return True;
    return False;
}

SV SV_trim_right(SV s) {
    while (s.size > 0 && SV_is_space(s.data[s.size-1])) {
        s.size -= 1;
    }
    return s;
}

SV SV_chop_while(SV s, char_to_bool test_char_function) {
    s64 i;
    for (i = 0; i < s.size; i++) {
        if (!test_char_function(s.data[i])) break;
    }
    s.data += i;
    s.size -= i;
    return s;
}


SV SV_get_next_line(SV *parseing, s64 *line_num, bool32 remove_comments, bool32 trim, bool32 skip_empty) {
    // this could inf loop if parseing.size is negative
    assert(parseing->size >= 0);

    SV next_line = *parseing;

    while (parseing->size) {
        s64 line_end = SV_find_index_of_char(*parseing, '\n');

        next_line = *parseing;

        if (line_end != -1) {
            next_line.size = line_end;
            SV_advance(parseing, line_end+1);
        } else {
            SV_advance(parseing, parseing->size);
        }

        *line_num += 1;

        if (remove_comments) {
            // remove comments
            s64 comment_index = SV_find_index_of_char(next_line, '#');
            if (comment_index != -1) { next_line.size = comment_index; }
        }

        if (trim) next_line = SV_trim_right(next_line);

        // 'if your not skipping empty' OR 'if there is something on the line', return it.
        if ((!skip_empty) || (next_line.size > 0)) break;
    }

    return next_line;
}



// -------------------------------------
//    String Buffer Helper Functions
// -------------------------------------

internal void SB_memcpy(char *dest, const char *src, s64 n) {
    for (s64 i = 0; i < n; i++) dest[i] = src[i];
}
internal s64 SB_strlen(const char *str) {
    s64 n = 0;
    while (*str++) n++;
    return n;
}
internal void SB_memset(void *dest, u8 to_set, s64 n) {
    u8 *d = dest;
    for (s64 i = 0; i < n; i++) d[i] = to_set;
}

internal void free_buffer(Character_Buffer *buffer) {
    if (buffer->data) {
        STRING_HELPER_FREE(buffer->data);
        buffer->data     = NULL;
        buffer->count    = 0;
        buffer->capacity = 0;
    }
}


// Might grow the string builder.
// Returns a pointer to a buffer that has enough space.
internal Character_Buffer *maybe_expand_to_fit(String_Builder *sb, s64 size) {
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
                sb->current_segment->next = STRING_HELPER_MALLOC(sizeof(Segment));

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

        last_buffer->data = STRING_HELPER_MALLOC(to_add_size);

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

    result.data = STRING_HELPER_MALLOC(new_string_size+1); // +1 for null byte, (for c_string compatiblity)
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
                STRING_HELPER_FREE(result.data);
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
            STRING_HELPER_FREE(current_segment);
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
        // TODO figure out how to do the thing we did in Arena.h for its sprintf
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



#endif // STRING_HELPER_IMPLEMENTATION_GUARD_
#endif // STRING_HELPER_IMPLEMENTATION



