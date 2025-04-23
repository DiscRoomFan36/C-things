//
// context.h
//
// DiscRoomFan
// created - 03/03/2025
//
// before you import this file. make sure to typedef a Context struct.
// that contains all the things you want in a context.
//
// this file just contains some helper tools to manage the context better,
// mainly 'PUSH_CONTEXT' and 'PUSH_CONTEXT_PARTLY'.
// these macros just make it easier to manage the scope of context switches
//
// Make sure to '#define CONTEXT_IMPLEMENTATION' only
// once when useing it in you own projects
//
// Other Optional Defines:
//
//  USE_EMPTY_CONTEXT
//      typedef's an empty context, not useful
//
//  ASSERT_IN_GET_OR_SET_CONTEXT
//      checks for null pointer in 'get_context()'
//      or 'set_context()' (for the new_context ptr)
//      (NOTE: You can '#define CONTEXT_ASSERT' to change
//      to a different assert, the default is
//      <assert.h>'s assert)
//
//  NO_GLOBAL_CONTEXT_BASE
//      stops a variable called 'context_base' from being
//      put into global memory, setting the context
//      pointer to NULL at the start of the program,
//      you will have to set it in your main function,
//      if you do not define this, the value at the
//      context pointer will be zero initalized when
//      the program starts. (as you might have done anyway)
//

#ifndef CONTEXT_H_
#define CONTEXT_H_


// You are suppost to define your own context when useing this header.
#ifdef USE_EMPTY_CONTEXT
typedef struct Context {} Context;
#endif // USE_EMPTY_CONTEXT


// Use this function to get the current context,
// Dont pass the pointer to the context around, use this.
Context *get_context();

// Use this function to set the context.
// All future calls to 'get_context()' will use the supplied context.
// Returns the old context
Context *set_context(Context *new_context);


// Push a new context for the duration of a scope block
//
// WARNING: Has some hidden pitfalls, Do not break out of this scope,
// or return from a function within it, it will not set the correct context
// after leaving in these ways, if you need to escape the scope, use 'continue'
#define PUSH_CONTEXT(new)                          \
    for (Context *tmp = set_context(new), *i = 0;  \
        (__intptr_t)i != 1;                        \
        set_context(tmp), i = (__typeof__(i))1)    \


// A "polymorphic" version, that can handle all possible push's.
//
// This is used to set only one variable on the context to something different,
// as you might often want to do, instead of making a whole new copy of the context.
//
// This macro requires a function with the name "set_context_{to_set}" to exist,
// and return the old value of the variable, kind of like the 'set_context()' dose
#define PUSH_CONTEXT_PARTLY(to_set, new)                           \
    for (__typeof__(new) tmp = set_context_##to_set(new), *i = 0;  \
        (__intptr_t)i != 1;                                        \
        set_context_##to_set(tmp), i = (__typeof__(new)*)1)        \


#endif // CONTEXT_H_


#ifdef CONTEXT_IMPLEMENTATION

#ifndef CONTEXT_IMPLEMENTATION_GUARD_
#define CONTEXT_IMPLEMENTATION_GUARD_

// I've had some thought about where the context_base should go.
// the way I see it, there are 3 ways to store something:
//   - Globally, where everyone can see it
//   - On the Stack, where it is local to a function
//   - On the Heap, where you have a pointer to it.
//
//
// In our case a Heap variable is strictly worse than a Global, as when it is
// malloced, and the pointer is given to 'set_context()', it just acts the
// same as a global, the only difference is that it can be leaked.
//
// Using a global variable for the context is fine, except
// if the user has access to the context_base, they might
// accidentally use it, and defeat the whole purpose of having a context.
// So a global should be in this file, under an '#ifdef IMPL' block.
// (for C header reasons)
//
// The Stack however is different, if we leave it up the the user of this library,
// they will create a 'Context context = {0};' in the main function,
// and set the context to it. This is a totally valid  (and correct) way to do it.
//
//
// All this is to say: Neither option is massively better than the other,
// so I give ***YOU*** the reader, the choice.
//
// If you set NO_GLOBAL_CONTEXT_BASE, no context_base is made in this file and
// you have to set the context to something in the begining of your main function
// or it will be NULL. (I also recommend ASSERT_IN_GET_OR_SET_CONTEXT
// if you use this).
//
// If you dont, it will be globally allocated in this file, under the IMPL ifdef,
// where it is out of reach for you, but you dont have to worry about a
// false call to get_context(). and you know it will always return a valid context.
// Unless you pass something bad into set_context()
//


#ifdef NO_GLOBAL_CONTEXT_BASE

// pointer to the current context
// Starts NULL, Please set
Context *__context = 0;

#else // NO_GLOBAL_CONTEXT_BASE

// context_base is in global memory, but is unaccesstable
// without useing the get_context() function
Context context_base = {0};

// pointer to the current context
Context *__context = &context_base;

#endif // NO_GLOBAL_CONTEXT_BASE


// this just define the CONTEXT_ASSERT() function if you dont
// have it for ASSERT_IN_GET_OR_SET_CONTEXT
#ifdef ASSERT_IN_GET_OR_SET_CONTEXT

# ifndef CONTEXT_ASSERT
#  include <assert.h>
#  define CONTEXT_ASSERT assert
# endif // CONTEXT_ASSERT

#endif // ASSERT_IN_GET_OR_SET_CONTEXT

Context *get_context() {
#ifdef ASSERT_IN_GET_OR_SET_CONTEXT
    CONTEXT_ASSERT(__context && "context must not be NULL");
#endif

    return __context;
}

Context *set_context(Context *new_context) {
#ifdef ASSERT_IN_GET_OR_SET_CONTEXT
    CONTEXT_ASSERT(new_context && "set_context was passed a NULL pointer");
#endif

    Context *old = __context;
    __context = new_context;
    return old;
}

#endif // CONTEXT_IMPLEMENTATION_GUARD_

#endif // CONTEXT_IMPLEMENTATION
