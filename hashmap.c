
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <assert.h>


typedef unsigned int HASH_INT;

typedef struct Entry {
    char *key;
    int value;
    // the hash of the key, for faster lookup
    HASH_INT hash;
    bool alive;
    bool in_use;
} Entry;


typedef struct HashMap {
    size_t array_size;

    size_t num_curr_elements;
    size_t num_used_slots;

    Entry *entrys;
} HashMap;

// bleh, C
void resize_hashmap(HashMap *hm, size_t new_size);

void add(HashMap *hm, char *key, int value);
void add_with_hash(HashMap *hm, char *key, int value, HASH_INT hash);

int get(HashMap *hm, char *key);
void delete(HashMap *hm, char *key);

// either returns the index of the key, the first open slot, or -1 on error
int maybe_get_index_of_key(HashMap *hm, char *key, HASH_INT hash);

// returns -1 if not there
int get_index_of_key(HashMap *hm, char *key, HASH_INT hash);

bool key_exists(HashMap *hm, char *key);


// -------------------------------------------------------
// -------------------------------------------------------


int compute_hash(char *to_hash) {
    HASH_INT hash = 6251;
    while (*to_hash != '\0') {
        hash += 31*hash + to_hash[0];
        to_hash++;
    }
    return hash;
}


int maybe_get_index_of_key(HashMap *hm, char *key, HASH_INT hash) {
    if (hm->array_size == 0) return -1;

    // this probe strategy covers all positions when its a power of 2
    // assert(hm->array_size is power of 2)
    size_t incr = 1;
    size_t pos = hash % hm->array_size;

    // while (hm->used_entrys[pos]) {
    while (hm->entrys[pos].in_use) {
        // check the hashes for speed, then check the actual keys
        Entry entry = hm->entrys[pos];
        if ((entry.alive) && (entry.hash == hash) && (strcmp(key, entry.key) == 0)) return pos;

        pos = (pos + incr) % hm->array_size;
        incr += 1;
        assert(incr < 500); // just error if this number gets to big
    }

    return pos;
}

int get_index_of_key(HashMap *hm, char *key, HASH_INT hash) {
    int index = maybe_get_index_of_key(hm, key, hash);

    if (index == -1)                 return -1;
    if (!hm->entrys[index].in_use)   return -1;

    return index;
}

bool key_exists(HashMap *hm, char *key) {
    assert(key != NULL);
    return get_index_of_key(hm, key, compute_hash(key)) != -1;
}


void resize_hashmap(HashMap *hm, size_t new_size) {
    // only make the hashmap bigger
    assert(new_size > hm->num_curr_elements);

    // we'll stomp the old one with this.
    HashMap new_hm = {
        .array_size = new_size,

        .num_curr_elements = 0,
        .num_used_slots    = 0,

        // get some new arrays
        .entrys = malloc(new_size * sizeof(Entry)),
    };

    // set these all to false, all other arrays can stay un-init
    for (size_t i = 0; i < new_hm.array_size; i++) {
        // we dont have to clean this whole thing. just the in_use flag...
        // new_hm.entrys[i] = {0};
        // however, memset might be faster...
        new_hm.entrys[i].in_use = false;
    }

    for (size_t i = 0; i < hm->array_size; i++) {
        Entry entry = hm->entrys[i];

        if (entry.in_use && entry.alive) {
            add_with_hash(&new_hm, entry.key, entry.value, entry.hash);
        }
    }


    // free old array
    free(hm->entrys);

    // stamp the new hashmap
    *hm = new_hm;
}



void add(HashMap *hm, char *key, int value) {
    assert(key != NULL);
    HASH_INT hash = compute_hash(key);
    add_with_hash(hm, key, value, hash);
}

void add_with_hash(HashMap *hm, char *key, int value, HASH_INT hash) {
    assert(key != NULL);

    int possible_index = get_index_of_key(hm, key, hash);
    if (possible_index != -1) {
        // update the value and return
        hm->entrys[possible_index].value = value;
        return;
    }

    // resize array when it is filled more than 50%
    if (hm->array_size <= (hm->num_used_slots + 1) * 2) {
        size_t new_size = hm->array_size == 0 ? 32 : hm->array_size * 2;
        resize_hashmap(hm, new_size);
    }

    // this must be an empty slot
    int empty_slot = maybe_get_index_of_key(hm, key, hash);
    assert(empty_slot != -1); // we have resized already
    assert(hm->entrys[empty_slot].in_use == false);

    hm->entrys[empty_slot].key      = key;
    hm->entrys[empty_slot].value    = value;
    hm->entrys[empty_slot].hash     = hash;
    hm->entrys[empty_slot].alive    = true;
    hm->entrys[empty_slot].in_use   = true;

    hm->num_curr_elements += 1;
    hm->num_used_slots    += 1;
}


int get(HashMap *hm, char *key) {
    assert(hm->num_curr_elements > 0);
    assert(key != NULL);

    HASH_INT hash = compute_hash(key);

    int index = get_index_of_key(hm, key, hash);
    if (index == -1) assert(false && "The key was not in the array.");

    return hm->entrys[index].value;
}

void delete(HashMap *hm, char *key) {
    assert(hm->num_curr_elements > 0);
    assert(key != NULL);

    HASH_INT hash = compute_hash(key);

    int index = get_index_of_key(hm, key, hash);
    if (index == -1) assert(false && "there is no key to delete.");

    hm->entrys[index].alive = false;
    hm->num_curr_elements  -= 1;
}


