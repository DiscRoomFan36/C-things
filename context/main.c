
#include <stdio.h>

typedef struct Context {
    int a;
} Context;

#define CONTEXT_IMPLEMENTATION
#include "context.h"

// this shouldn't be set in the "context.h" so we can define it somewhere else
int set_context_a(int new_a) {
    Context *context = get_context();

    int old = context->a;
    context->a = new_a;
    return old;
}

void print_context(void) {
    Context *context = get_context();

    printf("a = %d\n", context->a);
    printf("\n");
}


int main(void) {
    // we are trying to replace the context for just one block

    {
        Context *context = get_context();
        context->a = 420;
    }

    printf("the original context\n");
    print_context();

    printf("the bad way to do it\n");
    {
        Context new_context = *get_context();
        new_context.a = 69;

        // replace the context.
        Context *tmp = set_context(&new_context);

        printf("using the new context\n");
        print_context();

        set_context(tmp);

        // this way is bad, because:
        // a) you could forget to set the context back
        // b) its hard to tell when the new context is active from a glance
        //    unless you put it in a block scope, and if your going to put it
        //    in a block, why not go all the way?
    }

    printf("context is reset after block\n");
    print_context();



    // the better way. (With macro's)
    printf("----------------------------------\n");
    printf("Now a better way\n");
    printf("----------------------------------\n");


    Context new_context = *get_context();
    new_context.a = 1239;

    PUSH_CONTEXT(&new_context) {
        printf("using the new context with the macro\n");
        print_context();

        Context newer_context = *get_context();
        newer_context.a -= 10000;

        PUSH_CONTEXT(&newer_context) {
            printf("double scoped\n");
            print_context();
        }

        printf("back to prev\n");
        print_context();
    }

    printf("after the macro block.\n");
    print_context();


    // now make a more specific one for 'a'
    printf("-------------------------------\n");
    printf("Now were going the use a more specific one\n");
    printf("-------------------------------\n");

    // unfortunately, we had to make a function above 'set_context_a()'
    // for this macro to work.
    int new_a = 1337;
    PUSH_CONTEXT_PARTLY(a, new_a) {
        printf("using the new a\n");
        print_context();
    }

    printf("after using new a.\n");
    print_context();

    return 0;
}

