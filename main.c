#include <stdio.h>
#include <stdlib.h>
#include "stack.h"
#include "log.h"

int main(void)
{
    restart_log();

    StackError error = STACK_OK;
    Stack stack = {};

    error = init_stack(&stack, 90);
    if (error != STACK_OK)
    {
        log("Error while initializing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    push_stack(&stack, 10);
    stack_el_t a = 0;

    for (int i = 0; i < 100000; i++)
    {
        push_stack(&stack, i * 14.88);
    }

    pop_stack(&stack, &a);
    printf(STACK_EL_SPECIFICATOR "\n", a);

    error = destroy_stack(&stack);
    if (error != STACK_OK)
    {
        log("Error while destroing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    return 0;
}