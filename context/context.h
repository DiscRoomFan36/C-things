//
// DiscRoomFan
// created - 03/03/2025
//

#ifndef CONTEXT_H_
#define CONTEXT_H_


// You are suppost to define your own context when useing this header.
#ifdef USE_EMPTY_CONTEXT
typedef struct Context {} Context;
#endif // USE_EMPTY_CONTEXT

// use this function to get the current context, dont pass it around.
Context *get_context();

// use this function to set the context.
// all future calls to 'get_context()' will use the supplied context
// returns the old context
Context *set_context(Context *new_context);


// Push a new context for the duration of a scope block
#define PUSH_CONTEXT(new)                          \
    for (Context *tmp = set_context(new), *i = 0;  \
        (__intptr_t)i != 1;                        \
        set_context(tmp), i = (typeof(i))1)        \


// a "polymorphic" version, that can handle all possible push's
// this requires a function with the name "set_context_{to_set}" to exist,
// and return the old thing
#define PUSH_CONTEXT_PARTLY(to_set, new)                       \
    for (typeof(new) tmp = set_context_##to_set(new), *i = 0;  \
        (__intptr_t)i != 1;                                    \
        set_context_##to_set(tmp), i = (typeof(new)*)1)        \


#endif // CONTEXT_H_


#ifdef CONTEXT_IMPLEMENTATION

// i dont think this sort of thing works in multiple files.
// maybe make a get_context() function...
Context context_base = {0};
Context *__context = &context_base;


Context *get_context() {
    return __context;
}

Context *set_context(Context *new_context) {
    Context *old = __context;
    __context = new_context;
    return old;
}

#endif // CONTEXT_IMPLEMENTATION
