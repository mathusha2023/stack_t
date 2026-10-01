#include "stack.h"
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "config.h"
#include "log.h"
#include "hash.h"

static StackError grow_stack(Stack *stk);
static StackError reduce_stack(Stack *stk);
static int need_stack_reduce(Stack *stk);
static int is_equal(double a, double b);
static const char *get_str_canary_status(stack_el_t canary);
static const char *get_str_struct_canary_status(size_t canary);
static size_t get_stack_hash(Stack *stk);

void dump_stack(Stack *stk)
{
#ifndef NSTKDEBUG
    assert(stk);

    flog("");
    flog("ERROR!!!");
    flog("Stack<%s> '%s' [%p] created at %s:%d, in function %s",
         STACK_EL_TYPE_STR, stk->name, stk, stk->file, stk->line, stk->function);
    flog("{\n");
    flog("    canary_left = %x, %s", stk->canary_left, get_str_struct_canary_status(stk->canary_left));
    flog("    canary_right = %x, %s", stk->canary_right, get_str_struct_canary_status(stk->canary_right));
    flog("");
    flog("    capacity = %lu", stk->capacity);
    flog("    size = %lu", stk->size);
    flog("    stack_hash = %x", stk->stack_hash);
    flog("    data<%s> [%p]", STACK_EL_TYPE_STR, stk->data);
    flog("    {\n");

    if (stk->data)
    {
        flog("         %s [%lu] = %x (CANARYYYY)", get_str_canary_status(stk->data[0]), 0, (size_t)stk->data[0]);

        size_t i = 1;
        for (i = 1; i < stk->size + 1; i++)
        {
            if (i > stk->capacity)
                break;
            flog("           * [%lu] = " STACK_EL_SPECIFICATOR, i, stk->data[i]);
        }

        for (; i < stk->capacity + 1; i++)
        {
            flog("             [%lu] = 1488 (POIZON!!!!)", i);
        }

        flog("         %s [%lu] = %x (CANARYYYY)", get_str_canary_status(stk->data[stk->right_data_canary_index]), i, (size_t)stk->data[stk->right_data_canary_index]);
    }

    flog("    }");
    flog("}\n");

#endif
}

// такой формат переменных - НЕ БАГ, а необходимость для дебага
StackError __init_stack(Stack *stk,
                        size_t capacity
                            ON_DEBUG(,
                                     const char *name,
                                     const char *file,
                                     const char *function,
                                     int line))
{
    assert(stk);
    assert(capacity);

    // только при дебаге устанавливаем соответствующие поля
#ifndef NSTKDEBUG
    assert(name);
    assert(file);
    assert(function);

    stk->name = name;
    stk->file = file;
    stk->function = function;
    stk->line = line;
#endif

    flog("Beginning stack [%p] initialization...", stk);

    StackError error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not empty!", stk);
        dump_stack(stk);
        return error;
    }

    stk->capacity = capacity;

    stk->canary_left = stk->canary_right = STRUCT_CANARY_CONST;

    stack_el_t *p = (stack_el_t *)calloc(capacity + 2, sizeof(stack_el_t));
    if (!p)
    {
        flog("ERROR: can not allocate memory for stack data!");
        dump_stack(stk);
        return STACK_MEMORY_ERROR;
    }

    stk->data = p;
    stk->data[0] = CANARY_CONST;
    stk->data[capacity + 1] = CANARY_CONST;
    stk->right_data_canary_index = capacity + 1;
    stk->stack_hash = get_stack_hash(stk);

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after initialization!", stk);
        dump_stack(stk);
        return error;
    }

    flog("Stack [%p] initialization successful!", stk);

    return STACK_OK;
}

StackError push_stack(Stack *stk, stack_el_t value)
{
    assert(stk);

    flog("Beginning stack [%p] pushing value " STACK_EL_SPECIFICATOR "...", stk, value);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK before pushing!", stk);
        dump_stack(stk);
        return error;
    }

    if (stk->size == stk->capacity)
    {
        flog("Stack [%p] size equals capacity, beginning growing...", stk);
        error = grow_stack(stk);
        if (error != STACK_OK)
        {
            flog("Error while growing stack [%p]: %s", stk, get_stack_error(error));
            dump_stack(stk);
            return error;
        }
        flog("Stack [%p] growing successful, time for pushing", stk);
    }

    stk->data[stk->size++ + 1] = value;

    stk->stack_hash = get_stack_hash(stk);

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after pushing!", stk);
        dump_stack(stk);
        return error;
    }

    flog("Stack [%p] pushing successfull. New element: [%lu] = " STACK_EL_SPECIFICATOR, stk, stk->size, value);
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
        dump_stack(stk);
        return error;
    }

    if (stk->size == 0)
    {
        flog("Stack [%p] size is 0, cant pop elem", stk);
        dump_stack(stk);
        return STACK_EMPTY;
    }

    if (need_stack_reduce(stk))
    {
        flog("Stack [%p] can be reduced. Beginning...", stk);
        error = reduce_stack(stk);
        if (error != STACK_OK)
        {
            flog("Error while reducing stack [%p]: %s", stk, get_stack_error(error));
            dump_stack(stk);
            return error;
        }
        flog("Stack [%p] reducing successful, time for popping", stk);
    }

    // stk->size > 0
    stk->size--;
    *buffer = stk->data[stk->size + 1];
    stk->data[stk->size + 1] = 0;

    stk->stack_hash = get_stack_hash(stk);

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after popping!", stk);
        dump_stack(stk);
        return error;
    }

    flog("Stack [%p] popping successfull! Popped element: " STACK_EL_SPECIFICATOR, stk, *buffer);
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
        dump_stack(stk);
        return error;
    }

    free_ptr(stk->data);
    stk->capacity = 0;
    stk->size = 0;

    stk->canary_left = stk->canary_right = 0;
    stk->right_data_canary_index = 0;
    stk->stack_hash = 0;

    error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not empty after destroing!", stk);
        dump_stack(stk);
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
        dump_stack(stk);
        return STACK_NOT_NULL_DATA;
    }
    if (stk->capacity > 0)
    {
        flog("Stack [%p] capacity not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_CAPACITY;
    }
    if (stk->size > 0)
    {
        flog("Stack [%p] size not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_SIZE;
    }
    if (stk->canary_left > 0)
    {
        flog("Stack [%p] left canary not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_CANARY;
    }
    if (stk->canary_right > 0)
    {
        flog("Stack [%p] right canary not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_CANARY;
    }
    if (stk->right_data_canary_index > 0)
    {
        flog("Stack [%p] right data canary index not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_RIGHT_DATA_CANARY_INDEX;
    }
    if (stk->stack_hash > 0)
    {
        flog("Stack [%p] stack_hash not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_HASH;
    }
    return STACK_OK;
}

StackError is_stack_ok(Stack *stk)
{
    assert(stk);

    if (!stk->data)
    {
        flog("Stack [%p] data is null", stk);
        dump_stack(stk);
        return STACK_NULL_DATA;
    }
    if (!stk->capacity)
    {
        flog("Stack [%p] capacity is null", stk);
        dump_stack(stk);
        return STACK_NULL_CAPACITY;
    }
    if (stk->size > stk->capacity)
    {
        flog("Stack [%p] size more than capacity", stk);
        dump_stack(stk);
        return STACK_SIZE_GREATER_THAN_CAPACITY;
    }
    if (!stk->right_data_canary_index)
    {
        flog("Stack [%p] right_data_canary_index is null", stk);
        dump_stack(stk);
        return STACK_NULL_RIGHT_DATA_CANARY_INDEX;
    }
    if (!stk->stack_hash)
    {
        flog("Stack [%p] stack_hash is null", stk);
        dump_stack(stk);
        return STACK_NULL_HASH;
    }

    check_canary_alive(stk);

    size_t new_hash = get_stack_hash(stk);
    if (stk->stack_hash != new_hash)
    {
        flog("Stack [%p] stack_hash is invalid: %x != %x", stk, stk->stack_hash, new_hash);
        dump_stack(stk);
        return STACK_INVALID_HASH;
    }
    return STACK_OK;
}

void check_canary_alive(Stack *stk)
{
    assert(stk);
    assert(stk->data);

    if (stk->canary_left != STRUCT_CANARY_CONST)
    {
        log("FATAL ERROR: left stack canary is died!!");
        dump_stack(stk);
        abort();
    }

    if (stk->canary_right != STRUCT_CANARY_CONST)
    {
        log("FATAL ERROR: right stack canary is died!!");
        dump_stack(stk);
        abort();
    }

    if (!is_equal((double)stk->data[0], (double)CANARY_CONST))
    {
        log("FATAL ERROR: left data canary is died!!");
        dump_stack(stk);
        abort();
    }

    if (!is_equal((double)stk->data[stk->right_data_canary_index], (double)CANARY_CONST))
    {
        log("FATAL ERROR: right data canary is died!!");
        dump_stack(stk);
        abort();
    }
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

static StackError grow_stack(Stack *stk)
{
    assert(stk);

    flog("Beginning stack [%p] growing... Old capacity: %lu", stk, stk->capacity);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK before growing!", stk);
        dump_stack(stk);
        return error;
    }

    if (stk->capacity != stk->size)
    {
        flog("Stack [%p] growing not need: stk->capacity != stk->size", stk);
        dump_stack(stk);
        return STACK_GROWING_NOT_NEED;
    }

    stack_el_t *temp = (stack_el_t *)realloc(stk->data, 2 * stk->capacity * sizeof(stack_el_t) + 2 * sizeof(stack_el_t));
    if (!temp)
    {
        flog("ERROR while reallocating memory for stack [%p]", stk);
        dump_stack(stk);
        return STACK_MEMORY_ERROR;
    }

    temp[0] = CANARY_CONST;
    temp[stk->capacity + 1] = 0;
    temp[2 * stk->capacity + 1] = CANARY_CONST;
    stk->right_data_canary_index = 2 * stk->capacity + 1;

    stk->data = temp;
    stk->capacity *= 2;

    stk->stack_hash = get_stack_hash(stk);

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after reallocating!", stk);
        dump_stack(stk);
        return error;
    }

    flog("Stack [%p] growing successful! New capacity: %lu", stk, stk->capacity);
    return STACK_OK;
}

// уменьшаем размер стека в 2 раза, только если
static StackError reduce_stack(Stack *stk)
{
    assert(stk);

    flog("Beginning stack [%p] reducing... Old capacity: %lu", stk, stk->capacity);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK before reducing!", stk);
        dump_stack(stk);
        return error;
    }

    if (stk->capacity <= MIN_STACK_CAPACITY_TO_REDUCE)
    {
        flog("Stack [%p] capacity = %lu, "
             "MIN_STACK_CAPACITY_TO_REDUCE = %lu, "
             "stack reducing not need",
             stk, stk->capacity, MIN_STACK_CAPACITY_TO_REDUCE);
        dump_stack(stk);
        return STACK_REDUCING_NOT_NEED;
    }

    if (stk->size * 2 >= stk->capacity)
    {
        flog("Stack [%p] capacity = %lu, "
             "size = %lu, "
             "stack reducing not allowed becouse size less than capacity in less than 2 times",
             stk, stk->capacity, stk->size);
        dump_stack(stk);
        return STACK_REDUCING_NOT_ALLOWED;
    }

    stack_el_t *temp = (stack_el_t *)realloc(stk->data, ++stk->capacity / 2 * sizeof(stack_el_t));
    if (!temp)
    {
        flog("ERROR while reallocating memory for stack [%p]", stk);
        dump_stack(stk);
        return STACK_MEMORY_ERROR;
    }

    stk->data[stk->capacity + 1] = 0;

    stk->data = temp;
    stk->capacity = ++stk->capacity / 2;
    stk->data[0] = CANARY_CONST;
    stk->data[2 * stk->capacity + 1] = CANARY_CONST;
    stk->right_data_canary_index = 2 * stk->capacity + 1;

    stk->stack_hash = get_stack_hash(stk);

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        flog("Stack [%p] not OK after reallocating!", stk);
        dump_stack(stk);
        return error;
    }

    flog("Stack [%p] reducing successful! New capacity: %lu", stk, stk->capacity);
    return STACK_OK;
}

static int need_stack_reduce(Stack *stk)
{
    assert(stk);

    return stk->capacity > MIN_STACK_CAPACITY_TO_REDUCE && stk->size * 2 < stk->capacity;
}

static int is_equal(double a, double b)
{
    return fabs(a - b) < EPSILON;
}

static const char *get_str_canary_status(stack_el_t canary)
{
    return is_equal((double)canary, (double)CANARY_CONST) ? " OK" : "BUG";
}

static const char *get_str_struct_canary_status(size_t canary)
{
    return canary == STRUCT_CANARY_CONST ? " OK" : "BUG or STACK_EMPTY";
}

// возвращает хэшированный стэк
// сам старый хэш не учитывается в хэшировании для корректной работы сравнений
static size_t get_stack_hash(Stack *stk)
{
    assert(stk);

    size_t temp_hash = stk->stack_hash;
    stk->stack_hash = 1488;

    size_t h = hash((char *)stk, sizeof(Stack));

    stk->stack_hash = temp_hash;

    return h;
}