
#ifndef STRING_BUILDER_H_
#define STRING_BUILDER_H_


#include "ints.h"
#include "String_View.h"

typedef struct Segment {
    char *data;
    s64 count;
    s64 capacity;
} Segment;

#define SEGMENT_DEFAULT_SIZE 512
#define NUM_SEGMENTS 32

typedef struct String_Builder {
    Segment segments[NUM_SEGMENTS];
    s8 segment_count; // how many segments are in use
} String_Builder;


SV SB_to_SV(String_Builder *sb);

void SB_reset(String_Builder *sb);
void SB_free(String_Builder *sb);

void SB_add_pointer_and_size(String_Builder *sb, char *ptr, s64 size);
void SB_add_C_str(String_Builder *sb, const char *c_str);
void SB_add_SV(String_Builder *sb, SV sv);


#endif // STRING_BUILDER_H_


#ifdef STRING_BUILDER_IMPLEMENTATION


#include <stdlib.h> // for malloc

#define local static



local void maybe_expand_to_fit(String_Builder *sb, s64 size) {
    if (size == 0) return;

    bool32 should_expand = (sb->segment_count == 0); // if no segments, make the first segment
    if (!should_expand) {
        // check if the last segment has enough room
        Segment *last_segment = &sb->segments[sb->segment_count-1];
        should_expand = (size + last_segment->count > last_segment->capacity);
    }

    if (should_expand) {
        s64 to_add_size = (size < SEGMENT_DEFAULT_SIZE) ? SEGMENT_DEFAULT_SIZE : size;
        sb->segments[sb->segment_count].data = malloc(to_add_size);
        sb->segments[sb->segment_count].count = 0;
        sb->segments[sb->segment_count].capacity = to_add_size;
        sb->segment_count += 1;
    }
}

local void SB_strcpy(char *dest, const char *src, s64 n) {
    for (s64 i = 0; i < n; i++) dest[n] = src[n];
}




SV SB_to_SV(String_Builder *sb) {
    s64 total_size = 0;
    for (s64 i = 0; i < sb->segment_count; i++) {
        Segment *segment = &sb->segments[i];
        total_size += segment->count;
    }

    SV result = {
        .data = malloc(total_size+1), // +1 for nul byte, (for c_string compatiblity)
        .size = total_size,
    };

    s64 index = 0;
    for (s64 i = 0; i < sb->segment_count; i++) {
        Segment *segment = &sb->segments[i];

        SB_strcpy(result.data + index, segment->data, segment->count);
        index += segment->count;
    }

    assert(index == total_size);
    result.data[index] = '\0'; // Null byte for c string's

    return result;
}


void SB_reset(String_Builder *sb) {
    assert(False && "TODO");
}

void SB_free(String_Builder *sb) {
    for (s64 i = 0; i < sb->segment_count; i++) {
        Segment *segment = &sb->segments[i];
        free(segment->data);
        segment->count = 0;
        segment->capacity = 0;
    }
    sb->segment_count = 0;
}





void SB_add_pointer_and_size(String_Builder *sb, char *ptr, s64 size) {
    if (size == 0) return;

    maybe_expand_to_fit(sb, size);

    // we know the last segment has enough room to fit
    Segment *last_segment = &sb->segments[sb->segment_count-1];

    SB_strcpy(last_segment->data + last_segment->count, ptr, size);
}

void SB_add_C_str(String_Builder *sb, const char *c_str) {
    // SV has a strlen, we'll just use it.
    SB_add_SV(sb, SV_from_C_Str(c_str));
}

void SB_add_SV(String_Builder *sb, SV sv) {
    SB_add_pointer_and_size(sb, sv.data, sv.size);
}



#endif // STRING_BUILDER_IMPLEMENTATION

