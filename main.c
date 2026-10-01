#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"
#include "log.h"

int main(void)
{
    restart_log();

    StackError error = STACK_OK;
    Stack stack = {};
    // stack.canary_left = 1;

    error = init_stack(&stack, 90);
    if (error != STACK_OK)
    {
        log("Error while initializing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    error = push_stack(&stack, 99999.99999);
    if (error != STACK_OK)
    {
        log("Error while pushing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    stack_el_t a = 0;

    for (int i = 0; i < 100; i++)
    {
        error = push_stack(&stack, 10 * i + 2.8);
        if (error != STACK_OK)
        {
            log("Error while pushing stack [%p]: %s", &stack, get_stack_error(error));
            return error;
        }
    }

    stack.size = stack.capacity + 10;
    memset(&stack, 67, 2);

    error = pop_stack(&stack, &a);
    if (error != STACK_OK)
    {
        log("Error while popping stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }
    printf(STACK_EL_SPECIFICATOR "\n", a);

    error = destroy_stack(&stack);
    if (error != STACK_OK)
    {
        log("Error while destroing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    return 0;
}