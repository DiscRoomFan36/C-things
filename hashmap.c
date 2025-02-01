
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include <assert.h>


typedef unsigned int HASH_INT;

typedef struct HashMap {
    size_t array_size;

    size_t num_curr_elements;
    size_t num_used_slots;

    char **keys;
    int *values;

    // an array of the hashs for the keys
    HASH_INT *hashs;
    bool *alive_entrys;
    bool *used_entrys;
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

    // helps keep runs of collisions down
    // assert(hm->array_size is power of 2) // this probe strategy works best when its a power of 2
    size_t incr = 1;
    size_t pos = hash % hm->array_size;

    while (hm->used_entrys[pos]) {
        // check the hashes for speed, then check the actual keys
        if ((hm->alive_entrys[pos]) && (hm->hashs[pos] == hash) && (strcmp(key, hm->keys[pos]) == 0)) return pos;

        pos = (pos + incr) % hm->array_size;
        incr += 1;
        assert(incr < 500); // just error if this number gets to big
    }

    return pos;
}

int get_index_of_key(HashMap *hm, char *key, HASH_INT hash) {
    int index = maybe_get_index_of_key(hm, key, hash);

    if (index == -1)                       return -1;
    if (hm->used_entrys[index] == false)   return -1;

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
        .num_used_slots = 0,

        // get some new arrays
        .keys          = malloc(new_size * sizeof(char *)),
        .values        = malloc(new_size * sizeof(int)),
        .hashs         = malloc(new_size * sizeof(HASH_INT)),
        .alive_entrys  = malloc(new_size * sizeof(bool)),
        .used_entrys   = malloc(new_size * sizeof(bool)),
    };

    // set these all to false, all other arrays can stay un-init
    memset(new_hm.alive_entrys, 0, new_size * sizeof(bool)); // this might be able to be un-init
    memset(new_hm.used_entrys,  0, new_size * sizeof(bool));

    for (size_t i = 0; i < hm->array_size; i++) {
        char *key     = hm->keys[i];
        int value     = hm->values[i];
        HASH_INT hash = hm->hashs[i];
        bool is_valid = hm->alive_entrys[i];
        bool is_used  = hm->used_entrys[i];

        if (is_used && is_valid)   add_with_hash(&new_hm, key, value, hash);
    }

    // free old arrays
    free(hm->keys);
    free(hm->values);
    free(hm->hashs);
    free(hm->alive_entrys);
    free(hm->used_entrys);

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
        hm->values[possible_index] = value;
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
    assert(hm->used_entrys[empty_slot] == false);

    hm->keys         [empty_slot] = key;
    hm->values       [empty_slot] = value;
    hm->hashs        [empty_slot] = hash;
    hm->alive_entrys [empty_slot] = true;
    hm->used_entrys  [empty_slot] = true;

    hm->num_curr_elements += 1;
    hm->num_used_slots    += 1;
}


int get(HashMap *hm, char *key) {
    assert(hm->num_curr_elements > 0);
    assert(key != NULL);

    HASH_INT hash = compute_hash(key);

    int index = get_index_of_key(hm, key, hash);
    if (index == -1) assert(false && "The key was not in the array.");

    return hm->values[index];
}

void delete(HashMap *hm, char *key) {
    assert(hm->num_curr_elements > 0);
    assert(key != NULL);

    HASH_INT hash = compute_hash(key);

    int index = get_index_of_key(hm, key, hash);
    if (index == -1) assert(false && "there is no key to delete.");

    hm->alive_entrys[index]  = false;
    hm->num_curr_elements   -= 1;
}


