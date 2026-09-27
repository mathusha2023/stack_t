#include <stdio.h>
#include "stack.h"
#include "log.h"

int main(void)
{
    restart_log();

    StackError error = STACK_OK;
    Stack stack = {};

    error = init_stack(&stack, 5);
    if (error != STACK_OK)
    {
        log("Error while initializing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    for (size_t i = 0; i < get_capacity(&stack); i++)
    {
        error = push_stack(&stack, 10 * i + 10);
        if (error != STACK_OK)
        {
            log("Error while pushing stack [%p]: %s", &stack, get_stack_error(error));
            return error;
        }
    }

    error = push_stack(&stack, 93848);
    if (error != STACK_OK)
    {
        log("Error while pushing stack [%p]: %s", &stack, get_stack_error(error));
    }

    for (size_t i = 0; i < get_capacity(&stack); i++)
    {
        stack_el_t el = 0;
        error = pop_stack(&stack, &el);
        if (error != STACK_OK)
        {
            log("Error while popping stack [%p]: %s", &stack, get_stack_error(error));
            return error;
        }
        printf("Get stack el: %d\n", el);
    }

    stack_el_t el = 0;
    error = pop_stack(&stack, &el);
    if (error != STACK_OK)
    {
        log("Error while popping stack [%p]: %s", &stack, get_stack_error(error));
    }

    el = 0;
    error = pop_stack(&stack, &el);
    if (error != STACK_OK)
    {
        log("Error while popping stack [%p]: %s", &stack, get_stack_error(error));
    }

    error = push_stack(&stack, 93848);
    if (error != STACK_OK)
    {
        log("Error while pushing stack [%p]: %s", &stack, get_stack_error(error));
    }

    el = 0;
    error = pop_stack(&stack, &el);
    if (error != STACK_OK)
    {
        log("Error while popping stack [%p]: %s", &stack, get_stack_error(error));
    }
    printf("Get stack el: %d\n", el);

    printf("Stack size is: %lu\n", get_size(&stack));

    error = destroy_stack(&stack);
    if (error != STACK_OK)
    {
        log("Error while destroing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    return 0;
}