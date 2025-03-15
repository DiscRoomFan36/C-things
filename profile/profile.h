

#ifndef PROFILE_H_
#define PROFILE_H_

#ifdef PROFILE_CODE
    #define PROFILE_SECTION(title) profile_section(title, __FILE__, __LINE__, __func__)
    #define PROFILE_SECTION_END()  profile_section_end(__FILE__, __LINE__, __func__)

    #define PROFILE_ZONE(title) profile_zone(title, __FILE__, __LINE__, __func__)
    #define PROFILE_ZONE_END()  profile_zone_end(__FILE__, __LINE__, __func__)

    #define PROFILE_PRINT() profile_print()
#else
    #define PROFILE_SECTION(...)
    #define PROFILE_SECTION_END(...)

    #define PROFILE_ZONE(...)
    #define PROFILE_ZONE_END(...)

    #define PROFILE_PRINT(...)
#endif


#include <stdlib.h>
#include <time.h>

#ifndef PROFILE_ASSERT
# include <assert.h>
# define PROFILE_ASSERT assert
#endif


typedef clock_t time_units;

typedef struct Profile_Data Profile_Data;

typedef struct Profile_Array {
    Profile_Data *items;
    size_t count;
    size_t capacity;
} Profile_Array;


struct Profile_Data {
    const char *title;
    time_units start_time;
    time_units end_time;

    const char *file; // these could be useful
    int         line; // these could be useful
    const char *func; // these could be useful

    // size_t section_depth;

    // // boolean value
    // int is_section;
    // Profile_Array sub_sections;
};


time_units get_time(void);
float time_units_to_secs(time_units x);

void da_append_profile_array(Profile_Array *da, Profile_Data item);


void profile_section(const char *title, const char *__file__, int __line__, const char *__fun__);
void profile_section_end(const char *__file__, int __line__, const char *__fun__);

void profile_zone(const char *title, const char *__file__, int __line__, const char *__fun__);
void profile_zone_end(const char *__file__, int __line__, const char *__fun__);

void profile_print(void);


#endif // PROFILE_H_


#ifdef PROFILE_IMPLEMENTATION

#ifndef PROFILE_IMPLEMENTATION_
#define PROFILE_IMPLEMENTATION_


Profile_Array profiles = {};


time_units get_time(void) {
    return clock();
}
float time_units_to_secs(time_units x) {
    return (float) x / (float) CLOCKS_PER_SEC;
}


void da_append_profile_array(Profile_Array *da, Profile_Data item) {
    if (da->count >= da->capacity) {
        da->capacity = da->capacity == 0 ? 32 : da->capacity*2;
        da->items = (Profile_Data*) realloc(da->items, da->capacity*sizeof(Profile_Data));
        PROFILE_ASSERT(da->items != NULL && "Buy More RAM lol");
    }

    da->items[da->count++] = item;
}


// this starts a new section, all the other stuff after this is indented
void profile_section(const char *title, const char *__file__, int __line__, const char *__fun__) {
    // zone's cant contain sections...
    profile_zone_end(__file__, __line__, __fun__);

    // TODO
}

void profile_section_end(const char *__file__, int __line__, const char *__fun__) {
    profile_zone_end(__file__, __line__, __fun__);

    // TODO
}


// profile the things after this call
// will stop with next profile.h function call
void profile_zone(const char *title, const char *__file__, int __line__, const char *__fun__) {
    time_units start = get_time();

    // end last zone
    profile_zone_end(__file__, __line__, __fun__);

    Profile_Data data = {
        .title = title,
        .start_time = start,
        .end_time = 0,

        .file = __file__,
        .line = __line__,
        .func = __fun__,
    };

    da_append_profile_array(&profiles, data);
}

void profile_zone_end(const char *__file__, int __line__, const char *__fun__) {
    (void) __file__;
    (void) __line__;
    (void) __fun__;

    // there could be no zone to end. lookout!
    // there is no need if there was no prev
    if (profiles.count == 0) return;

    Profile_Data *last = &profiles.items[profiles.count-1];
    // dont update if the end time is already there
    if (last->end_time != 0) return;

    time_units end = get_time();
    last->end_time = end;
}


void profile_print(void) {
    // profile_zone_end(); // hmmm? skill issue?

    printf("Profiling Results:\n");
    for (size_t i = 0; i < profiles.count; i++) {
        Profile_Data *data = &profiles.items[i];

        float secs = time_units_to_secs(data->end_time - data->start_time);

        int digits_of_precision = 5;
        int digits_of_pad = 3;
        // +1 for the dot
        int pad_digits = digits_of_precision + digits_of_pad + 1;

        printf("    %20s : ", data->title);
        printf("%*.*f", pad_digits, digits_of_precision, secs);
        printf("\n");
    }
}


#endif // PROFILE_IMPLEMENTATION_

#endif // PROFILE_IMPLEMENTATION
