
#ifndef STRING_BUILDER_H_
#define STRING_BUILDER_H_


#include "ints.h"
#include "String_View.h"


// how much to allocate in a single page, is equal to the default page size in windows and linux.
#define SEGMENT_DEFAULT_SIZE 4096
#define NUM_SEGMENTS 32


typedef struct Segment {
    char *data;
    s64 count;
    s64 capacity;
} Segment;

typedef struct Segments_Holder {
    // the next segment in a linked list
    struct Segments_Holder *next;

    Segment segments[NUM_SEGMENTS];
} Segments_Holder;

typedef struct String_Builder {
    // how much to allocate when a new segment is needed.
    s64 base_new_allocation;


    // the first in a linked list.
    Segments_Holder first_segment_holder;
    // the current segment were working on.
    Segments_Holder *current_segment;

    s64 segment_index; // last segment in use.

} String_Builder;


SV SB_to_SV(String_Builder *sb);

void SB_reset(String_Builder *sb);
void SB_free(String_Builder *sb);

void SB_add_pointer_and_size(String_Builder *sb, char *ptr, s64 size);
void SB_add_C_str(String_Builder *sb, const char *c_str);
void SB_add_SV(String_Builder *sb, SV sv);

// printf into the string builder
s64 SB_printf(String_Builder *sb, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

#endif // STRING_BUILDER_H_


#ifdef STRING_BUILDER_IMPLEMENTATION


#include <stdlib.h> // for malloc
#include <stdarg.h> // for va_arg's in printf like functions

#define local static



// might grow the string builder, and
// returns a pointer to a segment that has enough space.
local Segment *maybe_expand_to_fit(String_Builder *sb, s64 size) {
    // if (size == 0) return;

    // set the current segment to the first segment if its null (aka just after zero initialization)
    if (sb->current_segment == NULL) sb->current_segment = &sb->first_segment_holder;

    // this is so we dont have to keep modding by NUM_SEGMENTS
    // TODO is this needed? num segments is a pow of 2
    s64 working_segment_index = sb->segment_index % NUM_SEGMENTS;

    // Segments_Holder *current_segment = sb->current_segment;
    // // skip over the full segments
    // // TODO just keep a pointer to the last segment holder for speed.
    // while (working_segment_index >= NUM_SEGMENTS) {
    //     current_segment = current_segment->next;
    //     assert(current_segment);
    //     working_segment_index -= NUM_SEGMENTS;
    // }

    // now, check all the segments to see if they have space, there could be more segments after this on so big copypasta loop incoming.
    Segment *last_segment = &sb->current_segment->segments[working_segment_index];
    while (True) {
        // if the segment hasn't been allocated
        if (last_segment->count == 0) break;

        // if the segent has enough room to hold the new thing
        if (size + last_segment->count <= last_segment->capacity) break;

        // else move onto the next segment
        working_segment_index += 1;
        sb->segment_index += 1; // this two are tied together

        if (working_segment_index == NUM_SEGMENTS) {
            // we ran off the end. use linked list pointer
            if (!sb->current_segment->next) {
                // we need to make a new segment holder
                // make sure to init to 0
                sb->current_segment->next = calloc(1, sizeof(Segments_Holder));
                assert(sb->current_segment->next && "maybe_expand_to_fit: failed to malloc a new segment holder");
            }
            sb->current_segment = sb->current_segment->next;
            working_segment_index -= NUM_SEGMENTS;
        }

        last_segment = &sb->current_segment->segments[working_segment_index];
    }

    assert(last_segment);
    assert((last_segment->capacity == 0) || (size + last_segment->count <= last_segment->capacity));

    if (last_segment->capacity == 0) {
        assert(sb->base_new_allocation >= 0 && "Base new allocation < 0 is not allowed.");

        // if base_new_allocation is not zero, use it, else use default macro.
        s64 default_size = (sb->base_new_allocation > 0) ? sb->base_new_allocation : SEGMENT_DEFAULT_SIZE;
        s64 to_add_size = (size < default_size) ? default_size : size;

        last_segment->data = malloc(to_add_size);
        assert(last_segment->data && "maybe_expand_to_fit: failed to malloc a new segment with to_add_size");
        last_segment->count = 0;
        last_segment->capacity = to_add_size;
    }

    return last_segment;
}

local void SB_strcpy(char *dest, const char *src, s64 n) {
    for (s64 i = 0; i < n; i++) dest[i] = src[i];
}
local s64 SB_strlen(const char *str) {
    s64 n = 0;
    while (*str++) n++;
    return n;
}




SV SB_to_SV(String_Builder *sb) {
    // find the total size of the new string view
    s64 total_size = 0;

    { // @Copypasta This loop over all segments is a little gnarly.
        s64 segment_index = sb->segment_index;
        Segments_Holder *current_segment = &sb->first_segment_holder;
        // get the stuff in the full segments.
        while (segment_index > NUM_SEGMENTS) {
            for (s64 i = 0; i < NUM_SEGMENTS; i++) {
                Segment *segment = &current_segment->segments[i];
                total_size += segment->count;
            }
            segment_index -= NUM_SEGMENTS;
            current_segment = current_segment->next;
            assert(current_segment);
        }
        // finish off the non full segment
        for (s64 i = 0; i <= segment_index; i++) {
            Segment *segment = &current_segment->segments[i];
            total_size += segment->count;
        }
    }

    SV result = {
        .data = malloc(total_size+1), // +1 for null byte, (for c_string compatiblity)
        .size = total_size,
    };

    // put all the strings into the new buffer.
    s64 index = 0;
    { // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
        s64 segment_index = sb->segment_index;
        Segments_Holder *current_segment = &sb->first_segment_holder;
        // get the stuff in the full segments.
        while (segment_index > NUM_SEGMENTS) {
            for (s64 i = 0; i < NUM_SEGMENTS; i++) {
                Segment *segment = &current_segment->segments[i];
                SB_strcpy(result.data + index, segment->data, segment->count);
                index += segment->count;
            }
            segment_index -= NUM_SEGMENTS;
            current_segment = current_segment->next;
            assert(current_segment);
        }
        // finish off the non full segment
        for (s64 i = 0; i <= segment_index; i++) {
            Segment *segment = &current_segment->segments[i];
            SB_strcpy(result.data + index, segment->data, segment->count);
            index += segment->count;
        }
    }

    assert(index == total_size);
    result.data[index] = '\0'; // Null byte for c string's

    return result;
}


void SB_reset(String_Builder *sb) {
    // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
    s64 segment_index = sb->segment_index;
    Segments_Holder *current_segment = &sb->first_segment_holder;
    // get the stuff in the full segments.
    while (segment_index > NUM_SEGMENTS) {
        for (s64 i = 0; i < NUM_SEGMENTS; i++) {
            Segment *segment = &current_segment->segments[i];
            segment->count = 0;
        }
        segment_index -= NUM_SEGMENTS;
        current_segment = current_segment->next;
        assert(current_segment);
    }
    // finish off the non full segment
    for (s64 i = 0; i <= segment_index; i++) {
        Segment *segment = &current_segment->segments[i];
        segment->count = 0;
    }

    sb->segment_index = 0;
    sb->current_segment = &sb->first_segment_holder;
}

local void free_segment(Segment *segment) {
    if (segment->data) {
        free(segment->data);
        segment->data     = NULL;
        segment->count    = 0;
        segment->capacity = 0;
    }
}

void SB_free(String_Builder *sb) {
    // @Copypasta This loop over all segments is a little gnarly. maybe a helper macro would not go astray...
    s64 segment_index = sb->segment_index;
    Segments_Holder *current_segment = &sb->first_segment_holder;
    // get the stuff in the full segments.
    while (segment_index > NUM_SEGMENTS) {
        for (s64 i = 0; i < NUM_SEGMENTS; i++) {
            Segment *segment = &current_segment->segments[i];
            free_segment(segment);
        }
        segment_index -= NUM_SEGMENTS;
        current_segment = current_segment->next;
        assert(current_segment);
    }
    // finish off the non full segment
    for (s64 i = 0; i <= segment_index; i++) {
        Segment *segment = &current_segment->segments[i];
        free_segment(segment);
    }

    { // free the segment pointers
        Segments_Holder *current_segment = sb->first_segment_holder.next;
        while (current_segment) {
            Segments_Holder *tmp = current_segment->next;
            free(current_segment);
            current_segment = tmp;
        }
    }

    sb->segment_index = 0;
    // so that one branch in maybe_expand_to_fit has less work
    sb->current_segment = &sb->first_segment_holder;
}





void SB_add_pointer_and_size(String_Builder *sb, char *ptr, s64 size) {
    if (size == 0) return;

    Segment *segment = maybe_expand_to_fit(sb, size);

    SB_strcpy(segment->data + segment->count, ptr, size);
    segment->count += size;
}

void SB_add_C_str(String_Builder *sb, const char *c_str) {
    SB_add_pointer_and_size(sb, (char*)c_str, SB_strlen(c_str));
}

void SB_add_SV(String_Builder *sb, SV sv) {
    SB_add_pointer_and_size(sb, sv.data, sv.size);
}


s64 SB_printf(String_Builder *sb, const char *format, ...) {
    va_list args;

    va_start(args, format);
        s64 formatted_size = vsnprintf(NULL, 0, format, args);
    va_end(args);

    // +1 because printf also puts a trailing '\0' byte.
    // we ignore that for the rest of the code.
    Segment *segment = maybe_expand_to_fit(sb, formatted_size+1);

    va_start(args, format);
        s64 n = vsprintf(segment->data + segment->count, format, args);
    va_end(args);

    assert(n == formatted_size);

    segment->count += formatted_size;

    return formatted_size;
}


#endif // STRING_BUILDER_IMPLEMENTATION

