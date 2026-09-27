#include "stack.h"
#include <assert.h>
#include <stdlib.h>
#include "config.h"
#include "log.h"

StackError init_stack(Stack *stk, size_t capacity)
{
    assert(stk);
    assert(capacity);

    log("Beginning stack [%p] initialization...", stk);

    StackError error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        log("Stack [%p] not empty!", stk);
        return error;
    }

    stk->capacity = capacity;

    stack_el_t *p = (stack_el_t *)calloc(capacity, sizeof(stack_el_t));
    if (!p)
    {
        log("ERROR: can not allocate memory for stack data!");
        return STACK_MEMORY_ERROR;
    }

    stk->data = p;

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        log("Stack [%p] not OK after initialization!", stk);
        return error;
    }

    log("Stack [%p] initialization successful!", stk);
    return STACK_OK;
}

StackError destroy_stack(Stack *stk)
{
    assert(stk);

    log("Beginning stack [%p] destroing...", stk);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        log("Stack [%p] not OK before destroing!", stk);
        return error;
    }

    free_ptr(stk->data);
    stk->capacity = 0;
    stk->size = 0;

    error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        log("Stack [%p] not empty after destroing!", stk);
        return error;
    }

    log("Stack [%p] destroing successful!", stk);
    return STACK_OK;
}

StackError is_stack_empty(Stack *stk)
{
    assert(stk);

    if (stk->data != NULL)
    {
        log("Stack [%p] data not null!", stk);
        return STACK_NOT_NULL_DATA;
    }
    if (stk->capacity > 0)
    {
        log("Stack [%p] capacity not null", stk);
        return STACK_NOT_NULL_CAPACITY;
    }
    if (stk->size > 0)
    {
        log("Stack [%p] size not null", stk);
        return STACK_NOT_NULL_SIZE;
    }
    return STACK_OK;
}

StackError is_stack_ok(Stack *stk)
{
    assert(stk);

    if (!stk->data)
    {
        log("Stack [%p] data is null", stk);
        return STACK_NULL_DATA;
    }
    if (!stk->capacity)
    {
        log("Stack [%p] capacity is null", stk);
        return STACK_NULL_CAPACITY;
    }
    if (stk->size > stk->capacity)
    {
        log("Stack [%p] size more than capacity", stk);
        return STACK_SIZE_GREATER_THAN_CAPACITY;
    }
    return STACK_OK;
}

const char *get_stack_error(StackError error)
{
    return STR_STACK_ERRORS[error];
}