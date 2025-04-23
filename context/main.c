//
// Demo for context.h
//
// DiscRoomFan
//
// created - 04/03/2025
//
// build and run:
// $ make && ./main
//


#include <stdio.h>
#include <assert.h>

// Remember to define your context up front.
// probably in a separate my_context.h file.
typedef struct Context {
    int a;
} Context;

#define CONTEXT_IMPLEMENTATION
#include "context.h"


// ------------------------ Helper Functions ------------------------

// this shouldn't be set in the "context.h" so we can define it somewhere else
// but it must be defined so we can use 'PUSH_CONTEXT_PARTLY(a, ...)'
int set_context_a(int new_a) {
    Context *context = get_context();

    int old = context->a;
    context->a = new_a;
    return old;
}

// for the demo
void print_context(void) {
    Context *context = get_context();

    printf("a = %d\n", context->a);
}


// ------------------------ Demo Functions ------------------------

void a_bad_way_to_handle_the_context() {
    // The mission statement is that we are trying
    // to replace the context for just one block

    printf("--------------------------------\n");
    printf("           A Bad Way\n");
    printf("--------------------------------\n");

    printf("the original context -> ");
    print_context();

    {
        Context new_context = *get_context();
        new_context.a = 69;

        // replace the context.
        Context *tmp = set_context(&new_context);

        printf("using the new context -> ");
        print_context();

        set_context(tmp);

        // this way is bad, because:
        // a) you could forget to set the context back
        // b) its hard to tell when the new context is active from a glance
        //    unless you put it in a block scope, and if your going to put it
        //    in a block, why not go all the way?
    }

    printf("context is reset after block -> ");
    print_context();
}


void a_better_way_to_handle_the_context() {
    // the better way. (With macro's)
    printf("--------------------------------\n");
    printf("         A Better Way\n");
    printf("--------------------------------\n");

    printf("the original context -> ");
    print_context();

    Context new_context = *get_context();
    new_context.a = 1239;

    PUSH_CONTEXT(&new_context) {
        printf("using the new context with the macro -> ");
        print_context();

        Context newer_context = *get_context();
        newer_context.a -= 10000;

        PUSH_CONTEXT(&newer_context) {
            printf("double scoped context also works -> ");
            print_context();
        }

        printf("back to prev context -> ");
        print_context();
    }

    printf("after the entire macro block -> ");
    print_context();
}


void a_more_specific_situation_aka_replace_only_one_element() {
    // now make a more specific one for 'a'
    printf("--------------------------------\n");
    printf("   A More Specific Situation\n");
    printf("--------------------------------\n");

    // unfortunately, we had to make a function above 'set_context_a()'
    // for this macro to work.
    int new_a = 1337;
    PUSH_CONTEXT_PARTLY(a, new_a) {
        printf("using the new a -> ");
        print_context();
    }

    printf("after using new a -> ");
    print_context();
}


void a_dangerous_pitfall() {
    printf("--------------------------------\n");
    printf("     A Dangerous Pitfall\n");
    printf("--------------------------------\n");

    // There is one serious problem with this macro system,
    // while it looks magical, its actually a for loop behind the scenes.
    //
    // And for this trick to work, the for loop must complete *naturally*.
    // What this means is that if you leave the scope via 'break' or 'return',
    // the context will not be set back to what it once was.


    // normally this is totally fine to do, and you should to it. For easer
    // accesss to the context in a function. And will be totally fine if you
    // don't fall into a pitfall.
    Context *context = get_context();

    printf("         -----------\n");
    printf("The Wrong Way To Exit Scope\n");
    printf("         -----------\n");
    {
        printf("the original context -> ");
        print_context();

        Context new_context = *get_context();
        new_context.a = 101010101;

        PUSH_CONTEXT(&new_context) {
            printf("just a normal push context -> ");
            print_context();

            if (0 < 1) {
                // oh no!
                // this could also be a 'return' or even a 'goto'
                break;
            }
        }

        printf("hopefully this is the correct context! -> ");
        print_context();

        // now the 'context' variable is different from the
        // 'get_context()' function return value.
        if (context == get_context()) {
            printf("The context is the same as get_context! Everything is fine!\n");
        } else {
            printf("The context is NOT the same as get_context! Everything is NOT fine! PANIC!!!!!!!\n");
        }

        // setting it back, for other examples
        set_context(context);
    }


    printf("         -----------\n");
    printf("The Correct Way To Exit Scope\n");
    printf("         -----------\n");
    {
        printf("the original context -> ");
        print_context();

        Context new_context = *get_context();
        new_context.a = 202020202;

        PUSH_CONTEXT(&new_context) {
            printf("just a normal push context -> ");
            print_context();

            if (0 < 1) {
                // oh yes! make sure to use continue
                continue;
            }
        }

        printf("hopefully this is the correct context! -> ");
        print_context();

        if (context == get_context()) {
            printf("The context is the same as get_context! Everything is fine!\n");
        } else {
            printf("The context is NOT the same as get_context! Everything is NOT fine! PANIC!!!!!!!\n");
        }
    }

    // this same problem is also in 'PUSH_CONTEXT_PARTLY'
}


// ------------------------ Just the Main Function ------------------------

int main(void) {

    // setting up the context
    //
    // NOTE: It should be fine for you to hold onto this variable,
    //       because unless your doing something stupid, this shouldn't differ
    //       from the output of 'get_context()' at the end of the program.
    Context *context = get_context();
    context->a = 420;


    a_bad_way_to_handle_the_context();
    printf("\n");

    a_better_way_to_handle_the_context();
    printf("\n");

    a_more_specific_situation_aka_replace_only_one_element();
    printf("\n");

    a_dangerous_pitfall();
    printf("\n");


    // unless you have an error in you code,
    // this should hold true
    assert(context == get_context());

    return 0;
}

