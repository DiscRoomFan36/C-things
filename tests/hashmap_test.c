
#include "../Bested.h"


void foo(void) {
    Hash_Map(u32, u32) my_hash_map = { .default_value = 60 };
    assert(my_hash_map.count == 0);

    assert(Hash_Map_Get(&my_hash_map, 32) == NULL);
    u32 *value_40 = Hash_Map_Get_Or_Default(&my_hash_map, 40);
    assert(*value_40 == 60);
    assert(my_hash_map.count == 1);

    Hash_Map_Reserve(&my_hash_map, 100);

    assert(Hash_Map_Contains(&my_hash_map, 32) == false);
    assert(Hash_Map_Contains(&my_hash_map, 40) == true);

    u32 *value = Hash_Map_Get_Or_Default(&my_hash_map, 40);
    *Hash_Map_Put(&my_hash_map, 141) = 84;
    assert(my_hash_map.count == 2);

    assert(Hash_Map_Remove(&my_hash_map, 423) == false);
    assert(Hash_Map_Remove_By_Value(&my_hash_map, value) == true);

    Hash_Map_For_Each(it, &my_hash_map) {
        u32 *key = Hash_Map_Key_For(&my_hash_map, it);

        printf("%u => %u\n", *key, *it);

        // you can also modify the value here.
        *it += 1;
    }

    Hash_Map_Clear(&my_hash_map);
    assert(my_hash_map.count == 0);
}





int main(void) {

    foo();

    typedef struct {
        String name;
        u32 age;
        b32 is_male;
    } Person;

    typedef Hash_Map(String, Person) Name_To_Person_Map;

    Name_To_Person_Map people = {
        .hash_function = Hash_Map_Hash_String,
        .eq_function   = Hash_Map_Eq_String,
        .default_value = { .name = S("NULL"), .age = 32, .is_male = false },
    };

    Person jim = {.name = S("Jim"), .age = 43, .is_male = true};

    *Hash_Map_Get_Or_Default(&people, jim.name) = jim;

    // create this one with only defaults.
    Hash_Map_Get_Or_Default(&people, S("Guy"));

    Person *jim_again = Hash_Map_Get(&people, S("Jim"));
    ASSERT(jim_again);

    u64 item_number = 0;
    Hash_Map_For_Each(p, &people) {
        item_number += 1;
        printf("%lu: "S_Fmt", age: %u, %s\n", item_number, S_Arg(p->name), p->age, p->is_male ? "Male" : "Female");
    }

    Hash_Map_Free(&people);
    return 0;
}




#define BESTED_IMPLEMENTATION
#include "../Bested.h"
