#include "stack.h"
#include <assert.h>
#include <stdlib.h>
#include "config.h"
#include "log.h"
#include "hash.h"

static StackError grow_stack(Stack *stk);
static StackError reduce_stack(Stack *stk);
static int need_stack_reduce(Stack *stk);

// функции используемые только при работе в режиме отладки
#ifndef NSTKDEBUG

static double my_fabs(double x);
static int is_equal(double a, double b);
void check_canary_alive(Stack *stk);
static const char *get_str_canary_status(stack_el_t canary);
static const char *get_str_struct_canary_status(size_t canary);
static size_t get_stack_hash(Stack *stk);

#endif

void dump_stack(Stack *stk)
{
    assert(stk);

    stklog("");
    stklog("ERROR!!!");
    stklog("Stack<%s> '%s' [%p] created at %s:%d, in function %s",
           STACK_EL_TYPE_STR, stk->name, stk, stk->file, stk->line, stk->function);
    stklog("{\n");
    stklog("    canary_left = %x, %s", stk->canary_left, get_str_struct_canary_status(stk->canary_left));
    stklog("    canary_right = %x, %s", stk->canary_right, get_str_struct_canary_status(stk->canary_right));
    stklog("");
    stklog("    capacity = %lu", stk->capacity);
    stklog("    size = %lu", stk->size);
    stklog("    stack_hash = %x", stk->stack_hash);
    stklog("    data<%s> [%p]", STACK_EL_TYPE_STR, stk->data);
    stklog("    {\n");

    if (stk->data)
    {
        stklog("         %s [%lu] = %x (CANARYYYY)", get_str_canary_status(stk->data[0]), 0, (size_t)stk->data[0]);

        size_t i = 1;
        for (i = 1; i < stk->size + 1; i++)
        {
            if (i > stk->capacity)
                break;
            stklog("           * [%lu] = " STACK_EL_SPECIFICATOR, i, stk->data[i]);
        }

        for (; i < stk->capacity + 1; i++)
        {
            stklog("             [%lu] = 1488 (POIZON!!!!)", i);
        }

        stklog("         %s [%lu] = %x (CANARYYYY)", get_str_canary_status(stk->data[stk->right_data_canary_index]), i, (size_t)stk->data[stk->right_data_canary_index]);
    }

    stklog("    }");
    stklog("}\n");
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

    stklog("Beginning stack [%p] initialization...", stk);

    StackError error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not empty!", stk);
        dump_stack(stk);
        return error;
    }

    stk->capacity = capacity;

#ifndef NSTKDEBUG

    stk->canary_left = stk->canary_right = STRUCT_CANARY_CONST;

    stack_el_t *p = (stack_el_t *)calloc(capacity + 2, sizeof(stack_el_t));
#else
    stack_el_t *p = (stack_el_t *)calloc(capacity, sizeof(stack_el_t));
#endif

    if (!p)
    {
        stklog("ERROR: can not allocate memory for stack data!");
        dump_stack(stk);
        return STACK_MEMORY_ERROR;
    }

    stk->data = p;

#ifndef NSTKDEBUG

    stk->data[0] = CANARY_CONST;
    stk->data[capacity + 1] = CANARY_CONST;
    stk->right_data_canary_index = capacity + 1;

    stk->stack_hash = get_stack_hash(stk);

#endif

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK after initialization!", stk);
        dump_stack(stk);
        return error;
    }

    stklog("Stack [%p] initialization successful!", stk);

    return STACK_OK;
}

StackError push_stack(Stack *stk, stack_el_t value)
{
    assert(stk);

    stklog("Beginning stack [%p] pushing value " STACK_EL_SPECIFICATOR "...", stk, value);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK before pushing!", stk);
        dump_stack(stk);
        return error;
    }

    if (stk->size == stk->capacity)
    {
        stklog("Stack [%p] size equals capacity, beginning growing...", stk);
        error = grow_stack(stk);
        if (error != STACK_OK)
        {
            stklog("Error while growing stack [%p]: %s", stk, get_stack_error(error));
            dump_stack(stk);
            return error;
        }
        stklog("Stack [%p] growing successful, time for pushing", stk);
    }

#ifndef NSTKDEBUG

    stk->data[stk->size++ + 1] = value;
    stk->stack_hash = get_stack_hash(stk);

#else

    stk->data[stk->size++] = value;

#endif

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK after pushing!", stk);
        dump_stack(stk);
        return error;
    }

    stklog("Stack [%p] pushing successfull. New element: [%lu] = " STACK_EL_SPECIFICATOR, stk, stk->size, value);
    return STACK_OK;
}

StackError pop_stack(Stack *stk, stack_el_t *buffer)
{
    assert(stk);

    stklog("Beginning stack [%p] popping...", stk);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK before popping!", stk);
        dump_stack(stk);
        return error;
    }

    if (stk->size == 0)
    {
        stklog("Stack [%p] size is 0, cant pop elem", stk);
        dump_stack(stk);
        return STACK_EMPTY;
    }

    if (need_stack_reduce(stk))
    {
        stklog("Stack [%p] can be reduced. Beginning...", stk);
        error = reduce_stack(stk);
        if (error != STACK_OK)
        {
            stklog("Error while reducing stack [%p]: %s", stk, get_stack_error(error));
            dump_stack(stk);
            return error;
        }
        stklog("Stack [%p] reducing successful, time for popping", stk);
    }

    // stk->size > 0
    stk->size--;

#ifndef NSTKDEBUG

    *buffer = stk->data[stk->size + 1];
    stk->data[stk->size + 1] = 0;

    stk->stack_hash = get_stack_hash(stk);

#else

    *buffer = stk->data[stk->size];
    stk->data[stk->size] = 0;

#endif

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK after popping!", stk);
        dump_stack(stk);
        return error;
    }

    stklog("Stack [%p] popping successfull! Popped element: " STACK_EL_SPECIFICATOR, stk, *buffer);
    return STACK_OK;
}

StackError destroy_stack(Stack *stk)
{
    assert(stk);

    stklog("Beginning stack [%p] destroing...", stk);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK before destroing!", stk);
        dump_stack(stk);
        return error;
    }

    free_ptr(stk->data);
    stk->capacity = 0;
    stk->size = 0;

#ifndef NSTKDEBUG

    stk->canary_left = stk->canary_right = 0;
    stk->right_data_canary_index = 0;
    stk->stack_hash = 0;

#endif

    error = is_stack_empty(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not empty after destroing!", stk);
        dump_stack(stk);
        return error;
    }

    stklog("Stack [%p] destroing successful!", stk);
    return STACK_OK;
}

StackError is_stack_empty(Stack *stk)
{
    assert(stk);

    if (stk->data != NULL)
    {
        stklog("Stack [%p] data not null!", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_DATA;
    }
    if (stk->capacity > 0)
    {
        stklog("Stack [%p] capacity not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_CAPACITY;
    }
    if (stk->size > 0)
    {
        stklog("Stack [%p] size not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_SIZE;
    }

#ifndef NSTKDEBUG

    if (stk->canary_left > 0)
    {
        stklog("Stack [%p] left canary not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_CANARY;
    }
    if (stk->canary_right > 0)
    {
        stklog("Stack [%p] right canary not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_CANARY;
    }
    if (stk->right_data_canary_index > 0)
    {
        stklog("Stack [%p] right data canary index not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_RIGHT_DATA_CANARY_INDEX;
    }
    if (stk->stack_hash > 0)
    {
        stklog("Stack [%p] stack_hash not null", stk);
        dump_stack(stk);
        return STACK_NOT_NULL_HASH;
    }

#endif

    return STACK_OK;
}

StackError is_stack_ok(Stack *stk)
{
    assert(stk);

    if (!stk->data)
    {
        stklog("Stack [%p] data is null", stk);
        dump_stack(stk);
        return STACK_NULL_DATA;
    }
    if (!stk->capacity)
    {
        stklog("Stack [%p] capacity is null", stk);
        dump_stack(stk);
        return STACK_NULL_CAPACITY;
    }
    if (stk->size > stk->capacity)
    {
        stklog("Stack [%p] size more than capacity", stk);
        dump_stack(stk);
        return STACK_SIZE_GREATER_THAN_CAPACITY;
    }

#ifndef NSTKDEBUG

    if (!stk->right_data_canary_index)
    {
        stklog("Stack [%p] right_data_canary_index is null", stk);
        dump_stack(stk);
        return STACK_NULL_RIGHT_DATA_CANARY_INDEX;
    }
    if (!stk->stack_hash)
    {
        stklog("Stack [%p] stack_hash is null", stk);
        dump_stack(stk);
        return STACK_NULL_HASH;
    }

    check_canary_alive(stk);

    size_t new_hash = get_stack_hash(stk);
    if (stk->stack_hash != new_hash)
    {
        stklog("Stack [%p] stack_hash is invalid: %x != %x", stk, stk->stack_hash, new_hash);
        dump_stack(stk);
        return STACK_INVALID_HASH;
    }

#endif

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

static StackError grow_stack(Stack *stk)
{
    assert(stk);

    stklog("Beginning stack [%p] growing... Old capacity: %lu", stk, stk->capacity);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK before growing!", stk);
        dump_stack(stk);
        return error;
    }

    if (stk->capacity != stk->size)
    {
        stklog("Stack [%p] growing not need: stk->capacity != stk->size", stk);
        dump_stack(stk);
        return STACK_GROWING_NOT_NEED;
    }

#ifndef NSTKDEBUG
    stack_el_t *temp = (stack_el_t *)realloc(stk->data, 2 * stk->capacity * sizeof(stack_el_t) + 2 * sizeof(stack_el_t));
#else
    stack_el_t *temp = (stack_el_t *)realloc(stk->data, 2 * stk->capacity * sizeof(stack_el_t));
#endif

    if (!temp)
    {
        stklog("ERROR while reallocating memory for stack [%p]", stk);
        dump_stack(stk);
        return STACK_MEMORY_ERROR;
    }

#ifndef NSTKDEBUG

    temp[0] = CANARY_CONST;
    temp[stk->capacity + 1] = 0;
    temp[2 * stk->capacity + 1] = CANARY_CONST;
    stk->right_data_canary_index = 2 * stk->capacity + 1;

#endif

    stk->data = temp;
    stk->capacity *= 2;

#ifndef NSTKDEBUG
    stk->stack_hash = get_stack_hash(stk);
#endif

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK after reallocating!", stk);
        dump_stack(stk);
        return error;
    }

    stklog("Stack [%p] growing successful! New capacity: %lu", stk, stk->capacity);
    return STACK_OK;
}

static StackError reduce_stack(Stack *stk)
{
    assert(stk);

    stklog("Beginning stack [%p] reducing... Old capacity: %lu", stk, stk->capacity);

    StackError error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK before reducing!", stk);
        dump_stack(stk);
        return error;
    }

    if (stk->capacity <= MIN_STACK_CAPACITY_TO_REDUCE)
    {
        stklog("Stack [%p] capacity = %lu, "
               "MIN_STACK_CAPACITY_TO_REDUCE = %lu, "
               "stack reducing not need",
               stk, stk->capacity, MIN_STACK_CAPACITY_TO_REDUCE);
        dump_stack(stk);
        return STACK_REDUCING_NOT_NEED;
    }

    if (stk->size * 2 >= stk->capacity)
    {
        stklog("Stack [%p] capacity = %lu, "
               "size = %lu, "
               "stack reducing not allowed becouse size less than capacity in less than 2 times",
               stk, stk->capacity, stk->size);
        dump_stack(stk);
        return STACK_REDUCING_NOT_ALLOWED;
    }

#ifndef NSTKDEBUG

    stk->data[stk->right_data_canary_index] = 0;
    stack_el_t *temp = (stack_el_t *)realloc(stk->data, (stk->capacity + 1) / 2 * sizeof(stack_el_t) + 2 * sizeof(stack_el_t));

#else
    stack_el_t *temp = (stack_el_t *)realloc(stk->data, (stk->capacity + 1) / 2 * sizeof(stack_el_t));
#endif

    if (!temp)
    {

#ifndef NSTKDEBUG
        stk->data[stk->right_data_canary_index] = CANARY_CONST;
#endif

        stklog("ERROR while reallocating memory for stack [%p]", stk);
        dump_stack(stk);
        return STACK_MEMORY_ERROR;
    }

    stk->data = temp;
    stk->capacity = (stk->capacity + 1) / 2;

#ifndef NSTKDEBUG

    stk->data[0] = CANARY_CONST;
    stk->right_data_canary_index = stk->capacity + 1;
    stk->data[stk->right_data_canary_index] = CANARY_CONST;

    stk->stack_hash = get_stack_hash(stk);
#endif

    error = is_stack_ok(stk);
    if (error != STACK_OK)
    {
        stklog("Stack [%p] not OK after reallocating!", stk);
        dump_stack(stk);
        return error;
    }

    stklog("Stack [%p] reducing successful! New capacity: %lu", stk, stk->capacity);
    return STACK_OK;
}

static int need_stack_reduce(Stack *stk)
{
    assert(stk);

    return stk->capacity > MIN_STACK_CAPACITY_TO_REDUCE && stk->size * 2 < stk->capacity;
}

// функции только для дебаг режима
#ifndef NSTKDEBUG

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

static const char *get_str_canary_status(stack_el_t canary)
{
    return is_equal((double)canary, (double)CANARY_CONST) ? " OK" : "BUG";
}

static const char *get_str_struct_canary_status(size_t canary)
{
    return canary == STRUCT_CANARY_CONST ? " OK" : "BUG or STACK_EMPTY";
}

// возвращает хэш стэка
// сам старый хэш не учитывается в хэшировании для корректной работы сравнений
static size_t get_stack_hash(Stack *stk)
{
    assert(stk);

    size_t temp_hash = stk->stack_hash;
    stk->stack_hash = 0x3142DA;

    size_t h = hash((char *)stk, sizeof(Stack));

    stk->stack_hash = temp_hash;

    return h;
}

static double my_fabs(double x)
{
    return x > 0 ? x : -x;
}

static int is_equal(double a, double b)
{
    return my_fabs(a - b) < EPSILON;
}

#endif // NSTKDEBUG
