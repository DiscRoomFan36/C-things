
#include "Bested.h"

struct Foo {
    u32 bar;
};

function u32 get_foo_bar(void) {
    return get_a_foo(32).bar;
}

function Foo get_a_foo(u32 cool) {
    Foo result = { .bar = cool };
    return result;
}

typedef u32 Baz;

enum cool_things {
    CAX,
    COX,
    SPACX,
};

