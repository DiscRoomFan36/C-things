

#ifndef PROFILE_H_
#define PROFILE_H_

#ifdef PROFILE_CODE
    #define PROFILE_ZONE(title) profile_zone(title, __FILE__, __LINE__, __func__)
    #define PROFILE_ZONE_END()  profile_zone_end(__FILE__, __LINE__, __func__)

    #define PROFILE_PRINT() profile_print()
    #define PROFILE_RESET() profile_reset()
    #define PROFILE_FREE() profile_free()
#else
    #define PROFILE_ZONE(...)
    #define PROFILE_ZONE_END(...)

    #define PROFILE_PRINT(...)
    #define PROFILE_RESET(...)
    #define PROFILE_FREE(...)
#endif


#include <stdlib.h>
#include <time.h>

#ifndef PROFILE_ASSERT
# include <assert.h>
# define PROFILE_ASSERT assert
#endif


typedef clock_t time_unit;

typedef struct Profile_Data {
    const char *title;
    time_unit start_time;
    time_unit end_time;

    const char *file; // these could be useful
    int         line; // these could be useful
    const char *func; // these could be useful
} Profile_Data;

typedef struct Profile_Data_Array {
    Profile_Data *items;
    size_t count;
    size_t capacity;
} Profile_Data_Array;

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
double time_units_to_secs(time_unit x);


int profile_equal(Profile_Data a, Profile_Data b);

void profile_zone(const char *title, const char *__file__, int __line__, const char *__fun__);
void profile_zone_end(const char *__file__, int __line__, const char *__fun__);

void profile_print(void);
void profile_reset(void);
void profile_free(void);

Profile_Stats_Array collect_stats(void);


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


Profile_Data_Array __base_zones = {0};


time_unit get_time(void) {
    return clock();
}
double time_units_to_secs(time_unit x) {
    return (double) x / (double) CLOCKS_PER_SEC;
}

int profile_equal(Profile_Data a, Profile_Data b) {
    if (a.title != b.title) return 0;
    if (a.file != b.file) return 0;
    if (a.line != b.line) return 0;
    if (a.func != b.func) return 0;
    return 1;
}


// profile the things after this call
// will stop with profile_end_zone
void profile_zone(const char *title, const char *__file__, int __line__, const char *__fun__) {
    time_unit start = get_time();
    Profile_Data data = {
        .title = title,
        .start_time = start,
        .end_time = 0,

        .file = __file__,
        .line = __line__,
        .func = __fun__,
    };

    profile_da_append(&__base_zones, data);
}

void profile_zone_end(const char *__file__, int __line__, const char *__fun__) {
    (void) __file__;
    (void) __line__;
    (void) __fun__;

    time_unit end = get_time();

    for (int i = __base_zones.count-1; i >= 0; i--) {
        Profile_Data *it = &__base_zones.items[i];

        // if its not zero, this zone has already been completed...
        if (it->end_time != 0) continue;

        it->end_time = end;
        return;
    }

    PROFILE_ASSERT(0 && "Unreachable");
    // return;
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

void profile_print(void) {
    printf("Profiling Results:\n");

    int max_word_length = 0;
    for (size_t i = 0; i < __base_zones.count; i++) {
        Profile_Data *it = &__base_zones.items[i];
        int title_len = profile_strlen(it->title);
        if (max_word_length < title_len) max_word_length = title_len;
    }

    for (size_t i = 0; i < __base_zones.count; i++) {
        Profile_Data *it = &__base_zones.items[i];

        float secs = time_units_to_secs(it->end_time - it->start_time);

        printf("|   ");
        printf("%-*s", max_word_length, it->title);
        printf(" : ");
        printf("%*.*f", PAD_DIGITS, DIGITS_OF_PRECISION, secs);
        printf("\n");
    }
}


void profile_reset(void) {
    __base_zones.count = 0;
}
void profile_free(void) {
    profile_da_free(&__base_zones);
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

Profile_Stats_Array collect_stats(void) {
    Profile_Stats_Array result = {0};     // linked arrays
    Profile_Data_Array unique_data = {0}; // linked arrays

    for (size_t i = 0; i < __base_zones.count; i++) {
        Profile_Data it = __base_zones.items[i];

        // check weather this data is in the unique data array.
        int maybe_index = maybe_index_in_array(unique_data, it);

        if (maybe_index == -1) {
            // its not in the array.
            Profile_Stats new_stats = {
                .title = it.title,
                .file  = it.file,
                .line  = it.line,
                .func  = it.func,
            };

            profile_da_append(&unique_data, it);
            profile_da_append(&result, new_stats);
            maybe_index = unique_data.count-1;
        }

        Profile_Stats *stats = &result.items[maybe_index];
        time_unit time = it.end_time - it.start_time;
        profile_da_append(&stats->times, time);
    }

    profile_da_free(&unique_data);
    return result;
}


// TODO move this
typedef struct Numerical_Average_Bounds {
    double sample_mean;
    double standard_deviation;
    double standard_error;
    double confidence_interval_upper;
    double confidence_interval_lower;
} Numerical_Average_Bounds;

typedef struct Double_Array {
    double *items;
    size_t count;
    size_t capacity;
} Double_Array;


#include <math.h>

Numerical_Average_Bounds get_numerical_average(Double_Array numbers) {
    double sample_mean = 0;
    for (size_t i = 0; i < numbers.count; i++) {
        double secs = time_units_to_secs(numbers.items[i]);

        // TODO? use https://en.wikipedia.org/wiki/Kahan_summation_algorithm
        sample_mean += secs;
    }
    sample_mean /= numbers.count;

    double standard_deviation = 0;
    for (size_t i = 0; i < numbers.count; i++) {
        double secs = time_units_to_secs(numbers.items[i]);

        // TODO? use https://en.wikipedia.org/wiki/Kahan_summation_algorithm
        standard_deviation += (secs - sample_mean)*(secs - sample_mean);
    }
    standard_deviation /= numbers.count;

    return (Numerical_Average_Bounds){
        .sample_mean = sample_mean,
        .standard_deviation = standard_deviation,
        .standard_error = standard_deviation / numbers.count,
        .confidence_interval_upper = sample_mean + 0.95 * (standard_deviation / sqrt(numbers.count)),
        .confidence_interval_lower = sample_mean - 0.95 * (standard_deviation / sqrt(numbers.count)),
    };
}


#endif // PROFILE_IMPLEMENTATION_

#endif // PROFILE_IMPLEMENTATION
