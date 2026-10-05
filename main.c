#include <string.h>
#include "stack.h"
#include "log.h"

static void print_status(const char *label, Stack *stk);

int main(void)
{
    restart_log();

    StackError error = STACK_OK;

    // CASE 1. Переполнение соседнего буфера

    // Структура чтобы все лежало подряд
    struct
    {
        char input[16];
        Stack stk;
    } case1 = {};

    error = init_stack(&case1.stk, 4);
    if (error != STACK_OK)
    {
        log("init_stack failed: %s", get_stack_error(error));
        return error;
    }

    for (int x = 1; x <= 20.; x++)
    {
        error = push_stack(&case1.stk, x);
        if (error != STACK_OK)
        {
            log("push failed: %s", get_stack_error(error));
            return error;
        }
    }

    print_status("before overflow", &case1.stk); // STACK_OK

    // Вводим строку длиннее буфера и задеваем канарейку стэка
    strcpy(case1.input, "AAAAAAAAAAAAAAAAAAAAAAAA");

    error = push_stack(&case1.stk, 1000 - 7);
    if (error != STACK_OK)
    {
        log("push after overflow                 : %s", get_stack_error(error));
        return error;
    }

    // CASE 2. Нечаянная порча поля size
    Stack stk2 = {};
    error = init_stack(&stk2, 4);
    if (error != STACK_OK)
    {
        log("init_stack failed: %s", get_stack_error(error));
        return error;
    }

    push_stack(&stk2, 10);
    push_stack(&stk2, 20);
    push_stack(&stk2, 30);

    print_status("before size corruption", &stk2); // STACK_OK

    // Меняем значение size
    stk2.size = 0;

    print_status("after size corruption", &stk2); // STACK_INVALID_HASH

    stack_el_t buf = 0;
    error = pop_stack(&stk2, &buf);
    if (error != STACK_OK)
    {
        log("pop after size corruption           : %s", get_stack_error(error));
        return error;
    }

    error = destroy_stack(&stk2);
    if (error != STACK_OK)
    {
        log("destroy corrupted stk2              : %s", get_stack_error(error));
        return error;
    }

    return 0;
}

static void print_status(const char *label, Stack *stk)
{
    StackError error = is_stack_ok(stk);
    log("%-35s : %s", label, get_stack_error(error));
}
