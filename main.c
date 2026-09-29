#include <stdio.h>

// TODO - fix it
// определение типа элементов нашего стэка
// если убрать define, будет выбран тип по умолчанию - double
// #define STACK_EL_TYPE int

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

    for (size_t i = 0; i < 24; i++)
    {
        push_stack(&stack, i * 10 + 1);
    }

    error = destroy_stack(&stack);
    if (error != STACK_OK)
    {
        log("Error while destroing stack [%p]: %s", &stack, get_stack_error(error));
        return error;
    }

    return 0;
}