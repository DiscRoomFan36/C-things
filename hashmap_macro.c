
// this is like the regular hashmap, but it works for all the types

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <assert.h>

typedef unsigned int HASH_INT;


typedef struct Entry_Base_ {
    HASH_INT hash;
    bool alive;
    bool in_use;
    struct KV_Pair {} pair;
} Entry_Base_;

typedef bool (*equality_function_type)(void *a, void *b);
typedef HASH_INT (*hash_function_type)(void *to_hash);

typedef struct HashMap_Base_ {
    size_t array_size;
    size_t num_curr_elements;
    size_t num_used_slots;
    equality_function_type equality_function;
    hash_function_type hash_function;
    Entry_Base_ *entrys;
} HashMap_Base_;


    // struct Entry_##key_type##_##value_type {    
#define ENTRY(key_type, value_type)             \
    struct {                                    \
        HASH_INT hash;                          \
        bool alive;                             \
        bool in_use;                            \
        struct {                                \
            key_type key;                       \
            value_type value;                   \
        } pair;                                 \
    }


    // struct HashMap_##key_type##_##value_type {
#define HASHMAP(key_type, value_type)               \
    struct {                                        \
        size_t array_size;                          \
        size_t num_curr_elements;                   \
        size_t num_used_slots;                      \
                                                    \
        equality_function_type equality_function;   \
        hash_function_type hash_function;           \
                                                    \
        ENTRY(key_type, value_type) *entrys;        \
    }


#define ENTRY_SIZE(pair_size) (sizeof(Entry_Base_) + pair_size)

int maybe_get_index_of_key_(HashMap_Base_ *hm, size_t pair_size, void *key, HASH_INT hash) {
    if (hm->array_size == 0) return -1;

    // this probe strategy covers all positions when its a power of 2
    // assert(hm->array_size is power of 2)
    size_t incr = 1;
    size_t pos = hash % hm->array_size;

    #define ARRAY_GET(arr, index, element_size)  ((arr) + ((index) * (element_size)))

    // ARRAY_GET(hm->entrys, pos, ENTRY_SIZE(pair_size))->in_use
    // hm->entrys + (pos * (sizeof(hm->entrys[0]) + pair_size));

    // while (hm->used_entrys[pos]) {
    while (ARRAY_GET(hm->entrys, pos, ENTRY_SIZE(pair_size))->in_use) {
        // check the hashes for speed, then check the actual keys
        Entry_Base_ entry = *ARRAY_GET(hm->entrys, pos, ENTRY_SIZE(pair_size));
        if ((entry.alive)
         && (entry.hash == hash)
         && (hm->equality_function(key, &entry.pair))) {
            return pos;
         }
        //  && (strcmp(key, entry.key) == 0)) return pos;

        pos = (pos + incr) % hm->array_size;
        incr += 1;
        assert(incr < 500); // just error if this number gets to big
    }

    return pos;
}

#define maybe_get_index_of_key(hm, key, hash) maybe_get_index_of_key_((hm), sizeof((hm)->entrys[0].pair), key, hash)

#define resize_hashmap(hm, new_size)                                        \
    do {                                                                    \
        assert((new_size) > (hm)->num_curr_elements);\
        \
        typeof(*hm) new_hm = {\
            .array_size = new_size,\
            .num_curr_elements = 0,\
            .num_used_slots    = 0,\
            .hash_function     = (hm)->hash_function,\
            .equality_function = (hm)->equality_function,\
            .entrys = calloc(new_size, sizeof((hm)->entrys[0])),\
        };\
        \
        for (size_t i = 0; i < (hm)->array_size; i++) {\
            typeof((hm)->entrys[0]) entry = (hm)->entrys[i];\
            \
            if (!(entry.in_use && entry.alive)) continue;\
            \
            int index = maybe_get_index_of_key(&new_hm, entry.pair.key, entry.hash);\
            \
            new_hm.entrys[index].pair.key      = entry.pair.key;                                 \
            new_hm.entrys[index].pair.value    = entry.pair.value;                             \
            new_hm.entrys[index].hash          = entry.hash;                                         \
            new_hm.entrys[index].alive         = entry.alive;                                         \
            new_hm.entrys[index].in_use        = entry.in_use;                                         \
                                                                                              \
            new_hm.num_curr_elements += 1;                                                     \
            new_hm.num_used_slots    += 1;                                                     \
        }\
        \
        free((hm)->entrys);\
        \
        *(hm) = new_hm;\
    } while (0)
    

#define add(hm, key, value)                                                                             \
    do {                                                                                                \
        HASH_INT hash = (hm)->hash_function(key);                                                       \
                                                                                                        \
        int possible_index = maybe_get_index_of_key((hm), (key), hash);   \
        if (possible_index != -1) {                                                                     \
            (hm)->entrys[possible_index].pair.val ## ue = (value);                                      \
        } else {                                                                                        \
                                                                                                        \
            if ((hm)->array_size <= ((hm)->num_used_slots + 1) * 2) {                                   \
                size_t new_size = (hm)->array_size == 0 ? 32 : (hm)->array_size * 2;                    \
                resize_hashmap((hm), new_size);                                                         \
            }                                                                                           \
                                                                                                        \
            int empty_slot = maybe_get_index_of_key((hm), (key), hash);   \
            assert(empty_slot != -1);                                                                   \
            assert((hm)->entrys[empty_slot].in_use == false);                                           \
                                                                                                        \
            (hm)->entrys[empty_slot].pair.ke ## y      = key;                                           \
            (hm)->entrys[empty_slot].pair.val ## ue    = (value);                                       \
            (hm)->entrys[empty_slot].hash     = hash;                                                   \
            (hm)->entrys[empty_slot].alive    = true;                                                   \
            (hm)->entrys[empty_slot].in_use   = true;                                                   \
                                                                                                        \
            (hm)->num_curr_elements += 1;                                                               \
            (hm)->num_used_slots    += 1;                                                               \
        }                                                                                               \
    } while (0)


// void *get_(HashMap_Base_ *hm, size_t pair_size, void *key) {
//     HASH_INT hash = hm->hash_function(key);

//     int index = maybe_get_index_of_key_(hm, pair_size, key, hash);
//     assert(index != -1);
//     Entry_Base_ *entry = (hm->entrys + (index * ENTRY_SIZE(pair_size)));
//     // assert(entry->in_use == true);

//     return &entry->pair;
// }

// #define get(hm, key)                                                                                \
//     ({                                                                                              \
//         typeof((hm)->entrys[0].pair) *p = get_((hm), sizeof((hm)->entrys[0].pair), (key));          \
//         p->value;\
//     })

#define get(hm, key)                                            \
    ({                                                          \
        HASH_INT hash = (hm)->hash_function(key);               \
                                                                \
        int index = maybe_get_index_of_key((hm), (key), hash);  \
        assert(index != -1);                                    \
        assert((hm)->entrys[index].in_use == true);             \
                                                                \
        (hm)->entrys[index].pair.value;                         \
    })
    


// void resize_hashmap(HashMap_Base_ *hm, size_t pair_size, size_t new_size);
// void add_with_hash (HashMap_Base_ *hm, size_t pair_size, void *key, void *value);


// void resize_hashmap(HashMap_Base_ *hm, size_t pair_size, size_t new_size) {
//     // only make the hashmap bigger
//     assert(new_size > hm->num_curr_elements);

//     // we'll stomp the old one with this.
//     HashMap_Base_ new_hm = {
//         .array_size = new_size,

//         .num_curr_elements = 0,
//         .num_used_slots    = 0,

//         // i dont want to set just the used flag, its a pain
//         .entrys = calloc(new_size, ENTRY_SIZE(pair_size)),
//     };


//     for (size_t i = 0; i < hm->array_size; i++) {
//         Entry_Base_ entry = *(hm->entrys + (i * ENTRY_SIZE(pair_size)));

//         if (entry.in_use && entry.alive) {
//             add_with_hash(&new_hm, entry.key, entry.value, entry.hash);
//         }
//     }


//     // free old array
//     free(hm->entrys);

//     // stamp the new hashmap
//     *hm = new_hm;
// }

