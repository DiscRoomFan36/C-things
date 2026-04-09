
#define BESTED_IMPLEMENTATION
#include "../Bested.h"


typedef struct {
    String name;
    u32 age;
    b32 is_male;
} Person;

// key == person.age
typedef struct Person_KV_Pair { u32 key; Person value; } Person_KV_Pair;

typedef struct {
    _Map_Header_;
    Person_KV_Pair *items;
} Person_Map;

Arena arena = {0};

int main(void) {

    Person_Map people = {0};
    people.allocator = &arena;

    Person jim = {.name = S("Jim"), .age = 43, .is_male = true};

    Map_Put(&people, jim.age, jim);


    Person *jim_again = Map_Get(&people, Person, jim.age);

    ASSERT(jim_again);

    for (u64 i = 0; i < people.count; i++) {
        Person p = people.items[i].value;
        printf("%lu: "S_Fmt", age: %u, %s\n", i, S_Arg(p.name), p.age, p.is_male ? "Male" : "Female");
    }

    // dont have to do this because we provided an allocator.
    // Map_Free(&people);

    Arena_Free(&arena);
    return 0;
}

