//
// hashmap.h - A simple hashmap
//
// Fletcher M - 06/02/2025
//

// TODO add u64   -> void* hashmap
// TODO add char* -> void* hashmap
// TODO add void* -> void* hashmap

#ifndef HASHMAP_H_
#define HASHMAP_H_


#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ints.h"


typedef u64 HASH_INT;

typedef struct Entry_Str_Int {
    // the hash of the key, for faster lookup
    HASH_INT hash;
    bool32 alive;
    bool32 in_use;

    // KV Pair
    char *key;
    s64 value;
} Entry_Str_Int;


typedef struct HashMap_Str_Int {
    u64 array_size;

    u64 num_curr_elements;
    u64 num_used_slots;

    Entry_Str_Int *entrys;
} HashMap_Str_Int;

// bleh, C
void resize_hashmap(HashMap_Str_Int *hm, u64 new_size);

void add(HashMap_Str_Int *hm, char *key, s64 value);
void add_with_hash(HashMap_Str_Int *hm, char *key, s64 value, HASH_INT hash);

s64 get(HashMap_Str_Int *hm, char *key);
void delete(HashMap_Str_Int *hm, char *key);

// either returns the index of the key, the first open slot, or -1 on error
s64 maybe_get_index_of_key(HashMap_Str_Int *hm, char *key, HASH_INT hash);

// returns -1 if not there
s64 get_index_of_key(HashMap_Str_Int *hm, char *key, HASH_INT hash);

bool32 key_exists(HashMap_Str_Int *hm, char *key);


// -------------------------------------------------------
// -------------------------------------------------------


HASH_INT compute_string_hash(char *to_hash) {
    // to handle the null Key...
    if (to_hash == NULL) return 0;

    HASH_INT hash = 6251;
    while (*to_hash) {
        hash += 31*hash + *to_hash;
        to_hash++;
    }
    return hash;
}


s64 maybe_get_index_of_key(HashMap_Str_Int *hm, char *key, HASH_INT hash) {
    if (hm->array_size == 0) return -1;

    // this probe strategy covers all positions when its a power of 2
    // assert(hm->array_size is power of 2)
    u64 incr = 1;
    u64 pos = hash % hm->array_size;

    // while (hm->used_entrys[pos]) {
    while (hm->entrys[pos].in_use) {
        // check the hashes for speed, then check the actual keys
        Entry_Str_Int entry = hm->entrys[pos];
        if ((entry.alive) && (entry.hash == hash) && (strcmp(key, entry.key) == 0)) return pos;

        pos = (pos + incr) % hm->array_size;
        incr += 1;
        assert(incr < 500); // just error if this number gets to big
    }

    return pos;
}

s64 get_index_of_key(HashMap_Str_Int *hm, char *key, HASH_INT hash) {
    s64 index = maybe_get_index_of_key(hm, key, hash);

    if (index == -1)                 return -1;
    if (!hm->entrys[index].in_use)   return -1;

    return index;
}

bool32 key_exists(HashMap_Str_Int *hm, char *key) {
    return get_index_of_key(hm, key, compute_string_hash(key)) != -1;
}


void resize_hashmap(HashMap_Str_Int *hm, u64 new_size) {
    // only make the hashmap bigger. maybe fix later?
    assert(new_size > hm->num_curr_elements && "For now we can only make it bigger, you shouldn't be touching this anyway.");

    // we'll stomp the old one with this.
    HashMap_Str_Int new_hm = {
        .array_size = new_size,

        .num_curr_elements = 0,
        .num_used_slots    = 0,

        .entrys = malloc(new_size * sizeof(Entry_Str_Int)),
    };

    // set these all to false, all other arrays can stay un-initalized for speed.
    for (u64 i = 0; i < new_hm.array_size; i++) {
        // we dont have to clean this whole thing. just the in_use flag...
        // new_hm.entrys[i] = {0};
        // however, memset might be faster...
        new_hm.entrys[i].in_use = False;
    }

    for (u64 i = 0; i < hm->array_size; i++) {
        Entry_Str_Int entry = hm->entrys[i];
        if (entry.in_use && entry.alive) {
            add_with_hash(&new_hm, entry.key, entry.value, entry.hash);
        }
    }

    free(hm->entrys);
    *hm = new_hm;
}



void add(HashMap_Str_Int *hm, char *key, s64 value) {
    HASH_INT hash = compute_string_hash(key);
    add_with_hash(hm, key, value, hash);
}

void add_with_hash(HashMap_Str_Int *hm, char *key, s64 value, HASH_INT hash) {
    s64 possible_index = get_index_of_key(hm, key, hash);
    if (possible_index != -1) {
        // update the value and return
        hm->entrys[possible_index].value = value;
        return;
    }

    // resize array when it is filled more than 50%
    if (hm->array_size <= (hm->num_used_slots + 1) * 2) {
        u64 new_size = hm->array_size == 0 ? 32 : hm->array_size * 2;
        resize_hashmap(hm, new_size);
    }

    // this must be an empty slot
    s64 empty_slot = maybe_get_index_of_key(hm, key, hash);
    assert(empty_slot != -1); // we have resized already
    assert(hm->entrys[empty_slot].in_use == False);

    hm->entrys[empty_slot].key      = key;
    hm->entrys[empty_slot].value    = value;
    hm->entrys[empty_slot].hash     = hash;
    hm->entrys[empty_slot].alive    = True;
    hm->entrys[empty_slot].in_use   = True;

    hm->num_curr_elements += 1;
    hm->num_used_slots    += 1;
}


s64 get(HashMap_Str_Int *hm, char *key) {
    HASH_INT hash = compute_string_hash(key);

    s64 index = get_index_of_key(hm, key, hash);
    if (index == -1) assert(False && "The key was not in the array.");

    return hm->entrys[index].value;
}

void delete(HashMap_Str_Int *hm, char *key) {
    HASH_INT hash = compute_string_hash(key);

    s64 index = get_index_of_key(hm, key, hash);
    if (index == -1) assert(False && "there is no key to delete.");

    hm->entrys[index].alive = False;
    hm->num_curr_elements  -= 1;
}


#endif // HASHMAP_H_
