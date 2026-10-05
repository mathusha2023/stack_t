#include <string.h>
#include <time.h>
#include "stack.h"
#include "log.h"

static void print_status(const char *label, Stack *stk);
static StackError case1(void);
static StackError case2(void);
static StackError case3(void);

int main(void)
{
    restart_log();

    StackError error = case1();
    if (error != STACK_OK)
    {
        log("ERROR: %s", get_stack_error(error));
        return error;
    }

    return 0;
}

static void print_status(const char *label, Stack *stk)
{
    StackError error = is_stack_ok(stk);
    log("%-35s : %s", label, get_stack_error(error));
}

// нечаянно залезли в память пренадлежащую стеку
static StackError case1(void)
{
    StackError error = STACK_OK;

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

    return error;
}

// нечаянно поменяли значение size стэка
static StackError case2(void)
{
    StackError error = STACK_OK;

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

    return error;
}

static StackError case3(void)
{
    StackError error = STACK_OK;
    const size_t COUNT = 100000;

    log("Start timer...");
    clock_t start_time = clock();

    Stack stk = {};
    error = init_stack(&stk, 1);
    if (error != STACK_OK)
    {
        log("init_stack failed: %s", get_stack_error(error));
        return error;
    }

    for (size_t i = 0; i < COUNT; i++)
    {
        error = push_stack(&stk, i * 0.25);
        if (error != STACK_OK)
        {
            log("push_stack failed: %s", get_stack_error(error));
            return error;
        }
    }

    error = destroy_stack(&stk);
    if (error != STACK_OK)
    {
        log("destroy_stack stk1 failed: %s", get_stack_error(error));
        return error;
    }

    clock_t end_time = clock();
    log("End timer");

    double seconds = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    log("Time spent: %lg seconds", seconds);

    return error;
}