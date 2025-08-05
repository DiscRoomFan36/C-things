
#define ARRAY_INITAL_CAPCITY 1

#define BESTED_IMPLEMENTATION
#include "../Bested.h"


typedef struct {
    String name;
    f32 age;
    b32 is_male;
} Person;

typedef struct {
    _Array_Header_;
    Person *items;
} Person_Array;

Arena arena = {0};

int main(void) {

    Person_Array people = {0};

    Array_Push(&arena, &people, ((Person){.name = S("Tim"),  .age = 53, .is_male = true}));
    Array_Push(&arena, &people, ((Person){.name = S("Chad"), .age = 21, .is_male = true}));
    Array_Push(&arena, &people, ((Person){.name = S("Mary"), .age = 43, .is_male = false}));
    Array_Push(&arena, &people, ((Person){.name = S("Ryan"), .age = 11, .is_male = true}));


    Array_For_Each(Person, p, &people) {
        u64 i = p - people.items;
        printf("%ld: "S_Fmt", age: %.2f, %s\n", i, S_Arg(p->name), p->age, p->is_male ? "Male" : "Female");
    }

    Arena_Free(&arena);
    return 0;
}

