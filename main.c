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

    error = init_stack(&stack, 10);
    if (error != STACK_OK)
    {
        log("Error while initializing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    const size_t num = 10000;
    stack_el_t a = 0;

    for (size_t i = 0; i < num; i++)
    {
        error = push_stack(&stack, 10 * i + 2.8);
        if (error != STACK_OK)
        {
            log("Error while pushing stack [%p]: %s", &stack, get_stack_error(error));
            return error;
        }
    }

    for (size_t i = 0; i < num; i++)
    {
        error = pop_stack(&stack, &a);
        if (error != STACK_OK)
        {
            log("Error while popping stack [%p]: %s", &stack, get_stack_error(error));
            return error;
        }
        printf("Popped element [%lu] is " STACK_EL_SPECIFICATOR "\n", i, a);
    }

    error = destroy_stack(&stack);
    if (error != STACK_OK)
    {
        log("Error while destroing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    return 0;
}