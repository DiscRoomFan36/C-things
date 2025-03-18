

#ifndef PROFILE_H_
#define PROFILE_H_

#ifdef PROFILE_CODE
    #define PROFILE_SECTION(title) profile_section(title, __FILE__, __LINE__, __func__)
    #define PROFILE_SECTION_END()  profile_section_end(__FILE__, __LINE__, __func__)

    #define PROFILE_ZONE(title) profile_zone(title, __FILE__, __LINE__, __func__)
    #define PROFILE_ZONE_END()  profile_zone_end(__FILE__, __LINE__, __func__)

    #define PROFILE_PRINT() profile_print()
    #define PROFILE_RESET() profile_reset()
#else
    #define PROFILE_SECTION(...)
    #define PROFILE_SECTION_END(...)

    #define PROFILE_ZONE(...)
    #define PROFILE_ZONE_END(...)

    #define PROFILE_PRINT(...)
    #define PROFILE_RESET(...)
#endif


#include <stdlib.h>
#include <time.h>

#ifndef PROFILE_ASSERT
# include <assert.h>
# define PROFILE_ASSERT assert
#endif


typedef clock_t time_unit;

typedef struct Profile_Data Profile_Data;

typedef struct Profile_Data_Array {
    Profile_Data *items;
    size_t count;
    size_t capacity;
} Profile_Data_Array;

struct Profile_Data {
    const char *title;
    time_unit start_time;
    time_unit end_time;

    const char *file; // these could be useful
    int         line; // these could be useful
    const char *func; // these could be useful

    // boolean value
    int is_section;
    Profile_Data *prev_section;
    Profile_Data_Array sub_sections;
};

typedef struct Time_Unit_Array {
    time_unit *items;
    size_t count;
    size_t capacity;
} Time_Unit_Array;

// collects like Profile data, useing file line and func
typedef struct Profile_Stats {
    const char *title;

    const char *file;
    int         line;
    const char *func;

    Time_Unit_Array times;
} Profile_Stats;

typedef struct Profile_Stats_Array {
    Profile_Stats *items;
    size_t count;
    size_t capacity;
} Profile_Stats_Array;


time_unit get_time(void);
float time_units_to_secs(time_unit x);


int profile_equal(Profile_Data a, Profile_Data b);

void profile_section(const char *title, const char *__file__, int __line__, const char *__fun__);
void profile_section_end(const char *__file__, int __line__, const char *__fun__);

void profile_zone(const char *title, const char *__file__, int __line__, const char *__fun__);
void profile_zone_end(const char *__file__, int __line__, const char *__fun__);

void profile_print(void);
void profile_reset(void);

Profile_Stats_Array collect_stats();


#define profile_da_append(da, item)                                                                        \
    do {                                                                                                   \
        if ((da)->count >= (da)->capacity) {                                                               \
            (da)->capacity = (da)->capacity == 0 ? 32 : (da)->capacity*2;                                  \
            (da)->items = (typeof((da)->items)) realloc((da)->items, (da)->capacity*sizeof(*(da)->items)); \
            assert((da)->items != NULL && "Buy More RAM lol");                                             \
        }                                                                                                  \
                                                                                                           \
        (da)->items[(da)->count++] = (item);                                                               \
    } while (0)

#define profile_da_free(da)                 \
    do {                                    \
        if ((da)->items) free((da)->items); \
        (da)->items    = 0;                 \
        (da)->count    = 0;                 \
        (da)->capacity = 0;                 \
    } while (0)



#endif // PROFILE_H_


#ifdef PROFILE_IMPLEMENTATION

#ifndef PROFILE_IMPLEMENTATION_
#define PROFILE_IMPLEMENTATION_


Profile_Data __base_section = {.is_section = 1, .sub_sections = {}};
Profile_Data *curent_section = &__base_section;


time_unit get_time(void) {
    return clock();
}
float time_units_to_secs(time_unit x) {
    return (float) x / (float) CLOCKS_PER_SEC;
}

int profile_equal(Profile_Data a, Profile_Data b) {
    if (a.title != b.title) return 0;
    if (a.file != b.file) return 0;
    if (a.line != b.line) return 0;
    if (a.func != b.func) return 0;
    return 1;
}


// this starts a new section, all the other stuff after this is indented
void profile_section(const char *title, const char *__file__, int __line__, const char *__fun__) {
    // zone's cant contain sections...
    profile_zone_end(__file__, __line__, __fun__);

    time_unit start = get_time();

    Profile_Data data = {
        .title = title,
        .start_time = start,
        .end_time = 0,

        .file = __file__,
        .line = __line__,
        .func = __fun__,

        .is_section = 1,
        .prev_section = curent_section,
        .sub_sections = {},
    };

    Profile_Data_Array *current_profiles = &curent_section->sub_sections;

    da_append_profile_array(current_profiles, data);
    Profile_Data *last = &current_profiles->items[current_profiles->count-1];
    curent_section = last;
}

void profile_section_end(const char *__file__, int __line__, const char *__fun__) {
    profile_zone_end(__file__, __line__, __fun__);

    if (curent_section->prev_section == NULL) return;

    time_unit end = get_time();
    curent_section->end_time = end;

    curent_section = curent_section->prev_section;
}


// profile the things after this call
// will stop with next profile.h function call
void profile_zone(const char *title, const char *__file__, int __line__, const char *__fun__) {
    // end last zone
    profile_zone_end(__file__, __line__, __fun__);

    time_unit start = get_time();
    Profile_Data data = {
        .title = title,
        .start_time = start,
        .end_time = 0,

        .file = __file__,
        .line = __line__,
        .func = __fun__,

        .is_section = 0,
        .prev_section = NULL,
        .sub_sections = {},
    };

    da_append_profile_array(&curent_section->sub_sections, data);
}

void profile_zone_end(const char *__file__, int __line__, const char *__fun__) {
    (void) __file__;
    (void) __line__;
    (void) __fun__;

    Profile_Data_Array *curent_profiles = &curent_section->sub_sections;

    // there could be no zone to end. lookout!
    // there is no need if there was no prev
    if (curent_profiles->count == 0) return;

    Profile_Data *last = &curent_profiles->items[curent_profiles->count-1];
    // dont update if the end time is already there
    if (last->end_time != 0) return;

    time_unit end = get_time();
    last->end_time = end;
}


#define DIGITS_OF_PRECISION 3
// how many tens digits before the '.'
#define NUM_TENS_DIGITS 3
// +1 for the dot
#define PAD_DIGITS (DIGITS_OF_PRECISION + NUM_TENS_DIGITS + 1)

size_t profile_strlen(const char *str) {
    size_t n = 0;
    while (*str++) n++;
    return n;
}

void profile_print_helper(Profile_Data_Array *array, int depth) {
    int max_word_length = 0;
    for (size_t i = 0; i < array->count; i++) {
        Profile_Data *data = &array->items[i];
        int title_len = profile_strlen(data->title);
        if (max_word_length < title_len) max_word_length = title_len;
    }

    for (size_t i = 0; i < array->count; i++) {
        Profile_Data *data = &array->items[i];

        float secs = time_units_to_secs(data->end_time - data->start_time);

        for (int i = 0; i < depth; i++) printf("|   ");

        printf("%-*s", max_word_length, data->title);
        printf(" : ");
        printf("%*.*f", PAD_DIGITS, DIGITS_OF_PRECISION, secs);
        printf("\n");

        if (data->is_section) {
            profile_print_helper(&data->sub_sections, depth+1);
        }
    }
}

void profile_print(void) {
    // profile_zone_end(); // hmmm? skill issue?

    printf("Profiling Results:\n");

    // TODO dont assert here...?
    PROFILE_ASSERT(curent_section == &__base_section);

    profile_print_helper(&curent_section->sub_sections, 1);
}


void profile_reset_helper(Profile_Data_Array *array) {
    for (size_t i = 0; i < array->count; i++) {
        Profile_Data *data = &array->items[i];

        if (data->is_section) {
            profile_reset_helper(&data->sub_sections);
        }
    }

    if (array->items) free(array->items);
    array->items = 0;
    array->count = 0;
    array->capacity = 0;
}

void profile_reset(void) {
    profile_reset_helper(&curent_section->sub_sections);
}


int maybe_index_in_array(Profile_Data_Array array, Profile_Data checking) {
    for (size_t i = 0; i < array.count; i++) {
        Profile_Data item = array.items[i];
        if (profile_equal(item, checking)) {
            return i;
        }
    }
    return -1;
}

Profile_Stats_Array collect_stats() {
    Profile_Stats_Array result = {};     // linked arrays
    Profile_Data_Array unique_data = {}; // linked arrays

    Profile_Data_Array sections_to_check = {};
    profile_da_append(&sections_to_check, *curent_section);

    while (sections_to_check.count > 0) {
        Profile_Data cur_section = sections_to_check.items[sections_to_check.count-1];
        sections_to_check.count -= 1;

        for (size_t i = 0; i < cur_section.sub_sections.count; i++) {
            Profile_Data checking = cur_section.sub_sections.items[i];

            // check weather this data is in the unique data array.
            int maybe_index = maybe_index_in_array(unique_data, checking);

            if (maybe_index == -1) {
                // its not in the array.
                da_append(&unique_data, checking);
                Profile_Stats new_stats = {
                    .title = checking.title,
                    .file = checking.file,
                    .line = checking.line,
                    .func = checking.func,
                };
                da_append(&result, new_stats);
                maybe_index = unique_data.count-1;
            }

            Profile_Stats *stats = &result.items[maybe_index];
            time_unit time = checking.end_time - checking.start_time;
            profile_da_append(&stats->times, time);

            if (checking.is_section) da_append_profile_array(&sections_to_check, checking);
        }
    }

    profile_da_free(&unique_data);
    profile_da_free(&sections_to_check);

    return result;
}


#endif // PROFILE_IMPLEMENTATION_

#endif // PROFILE_IMPLEMENTATION
