
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

Arena arena = ZEROED;

int main(void) {

    Person_Array people = ZEROED;
    people.allocator = &arena;


    Array_Push(&people, ((Person){.name = S("Tim"),  .age = 53, .is_male = true}));
    Array_Push(&people, ((Person){.name = S("Chad"), .age = 21, .is_male = true}));
    Array_Push(&people, ((Person){.name = S("Mary"), .age = 43, .is_male = false}));
    Array_Push(&people, ((Person){.name = S("Ryan"), .age = 11, .is_male = true}));


    Array_Reserve(&people, 32);

    Array_Add_Clear(&people, 2); // add 2 empty people

    Array_Insert(&people, 1, ((Person){.name = S("Steve"), .age = 60, .is_male = true}));

    Array_Remove(&people, 4, 1); // remove Ryan

    Array_Add(&people, 1); // dangerous add.
    Array_Remove(&people, people.count-1, 1);

    Array_For_Each(Person, p, &people) {
        u64 i = p - people.items;
        printf("%ld: "S_Fmt", age: %.2f, %s\n", i, S_Arg(p->name), p->age, p->is_male ? "Male" : "Female");
    }

    // we set an allocator, so we dont have to do this.
    // Array_Free(&people);

    Arena_Free(&arena);
    return 0;
}

