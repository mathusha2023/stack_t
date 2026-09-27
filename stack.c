#include "stack.h"
#include <assert.h>
#include <stdlib.h>
#include "config.h"
#include "log.h"

static StackError grow_stack(Stack *stk)
{
    assert(stk);

    flog("Beginning stack [%p] growing... Old size: %lu", stk, stk->capacity);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK before growing!", stk);
        return error;
    }

    if (stk->capacity != stk->size)
    {
        flog("Stack [%p] growing not need: stk->capacity != stk->size", stk);
        return STACK_GROWING_NOT_NEED;
    }

    stack_el_t *temp = (stack_el_t *)realloc(stk->data, 2 * stk->capacity * sizeof(stack_el_t));
    if (!temp)
    {
        flog("ERROR while reallocating memory for stack [%p]", stk);
        return STACK_MEMORY_ERROR;
    }

    stk->data = temp;
    stk->capacity *= 2;

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after reallocating!", stk);
        return error;
    }

    flog("Stack [%p] growing successful! New size: %lu", stk, stk->capacity);
    return STACK_OK;
}

StackError init_stack(Stack *stk, size_t capacity)
{
    assert(stk);
    assert(capacity);

    flog("Beginning stack [%p] initialization...", stk);

    StackError error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not empty!", stk);
        return error;
    }

    stk->capacity = capacity;

    stack_el_t *p = (stack_el_t *)calloc(capacity, sizeof(stack_el_t));
    if (!p)
    {
        flog("ERROR: can not allocate memory for stack data!");
        return STACK_MEMORY_ERROR;
    }

    stk->data = p;

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after initialization!", stk);
        return error;
    }

    flog("Stack [%p] initialization successful!", stk);
    return STACK_OK;
}

StackError push_stack(Stack *stk, stack_el_t value)
{
    assert(stk);

    flog("Beginning stack [%p] pushing...", stk);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK before pushing!", stk);
        return error;
    }

    if (stk->size == stk->capacity)
    {
        flog("Stack [%p] size equals capacity, beginning growing...", stk);
        error = grow_stack(stk);
        if (error != STACK_OK)
        {
            flog("Error while growing stack [%p]: %s", stk, get_stack_error(error));
            return error;
        }
        flog("Stack [%p] growing successful, time for pushing", stk);
    }

    stk->data[stk->size++] = value;

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after pushing!", stk);
        return error;
    }

    flog("Stack [%p] pushing successfull!", stk);
    return STACK_OK;
}

StackError pop_stack(Stack *stk, stack_el_t *buffer)
{
    assert(stk);

    flog("Beginning stack [%p] popping...", stk);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK before popping!", stk);
        return error;
    }

    if (stk->size == 0)
    {
        flog("Stack [%p] size is 0, cant pop elem", stk);
        return STACK_EMPTY;
    }

    // stk->size > 0
    stk->size--;
    *buffer = stk->data[stk->size];
    stk->data[stk->size] = 0;

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after popping!", stk);
        return error;
    }

    flog("Stack [%p] popping successfull!", stk);
    return STACK_OK;
}

StackError destroy_stack(Stack *stk)
{
    assert(stk);

    flog("Beginning stack [%p] destroing...", stk);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK before destroing!", stk);
        return error;
    }

    free_ptr(stk->data);
    stk->capacity = 0;
    stk->size = 0;

    error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not empty after destroing!", stk);
        return error;
    }

    flog("Stack [%p] destroing successful!", stk);
    return STACK_OK;
}

StackError is_stack_empty(Stack *stk)
{
    assert(stk);

    if (stk->data != NULL)
    {
        flog("Stack [%p] data not null!", stk);
        return STACK_NOT_NULL_DATA;
    }
    if (stk->capacity > 0)
    {
        flog("Stack [%p] capacity not null", stk);
        return STACK_NOT_NULL_CAPACITY;
    }
    if (stk->size > 0)
    {
        flog("Stack [%p] size not null", stk);
        return STACK_NOT_NULL_SIZE;
    }
    return STACK_OK;
}

StackError is_stack_ok(Stack *stk)
{
    assert(stk);

    if (!stk->data)
    {
        flog("Stack [%p] data is null", stk);
        return STACK_NULL_DATA;
    }
    if (!stk->capacity)
    {
        flog("Stack [%p] capacity is null", stk);
        return STACK_NULL_CAPACITY;
    }
    if (stk->size > stk->capacity)
    {
        flog("Stack [%p] size more than capacity", stk);
        return STACK_SIZE_GREATER_THAN_CAPACITY;
    }
    return STACK_OK;
}

const char *get_stack_error(StackError error)
{
    return STR_STACK_ERRORS[error];
}

size_t get_capacity(Stack *stk)
{
    assert(stk);
    return stk->capacity;
}

size_t get_size(Stack *stk)
{
    assert(stk);
    return stk->size;
}
