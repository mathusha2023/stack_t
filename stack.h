#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include "config.h"
#include "log.h"

// раскомментировать для отключения режима отладки стэка
// #define NSTKDEBUG

// значения которые по сути и задают полное поведение стэка
#define STACK_EL_TYPE double
#define STACK_EL_SPECIFICATOR "%lg"

// комбинация 2 обертки + переменная для прокидывания именно
// значения макроса STACK_EL_TYPE как строки
#define __STACK_EL_TYPE_STR_WRAPPER2(type) #type
#define __STACK_EL_TYPE_STR_WRAPPER1(type) __STACK_EL_TYPE_STR_WRAPPER2(type)
#define STACK_EL_TYPE_STR __STACK_EL_TYPE_STR_WRAPPER1(STACK_EL_TYPE)

// макрос для дополнительных параметров, которые  нужны только в режиме дебага
#ifdef NSTKDEBUG
#define ON_DEBUG(...)
#else
#define ON_DEBUG(...) __VA_ARGS__
#endif // NSTKDEBUG

// логирование стэка
#ifndef NSTKDEBUG
#define stklog(message, ...) flog(message, ##__VA_ARGS__)
#else
#define stklog(message, ...)
#endif

typedef STACK_EL_TYPE stack_el_t;

// уменьшение размера стека будет работать только если его размер превышает данное значение
const size_t MIN_STACK_CAPACITY_TO_REDUCE = 100;

const stack_el_t CANARY_CONST = (stack_el_t)0xEDAEDA;
const size_t STRUCT_CANARY_CONST = 0xEB1DEDA;

typedef enum StackError
{
    STACK_OK,
    STACK_MEMORY_ERROR,
    STACK_NULL_DATA,
    STACK_NOT_NULL_DATA,
    STACK_NULL_CAPACITY,
    STACK_NOT_NULL_CAPACITY,
    STACK_NOT_NULL_SIZE,
    STACK_NOT_NULL_CANARY,
    STACK_NULL_HASH,
    STACK_NOT_NULL_HASH,
    STACK_NULL_RIGHT_DATA_CANARY_INDEX,
    STACK_NOT_NULL_RIGHT_DATA_CANARY_INDEX,
    STACK_SIZE_GREATER_THAN_CAPACITY,
    STACK_OVERFLOW,
    STACK_EMPTY,
    STACK_GROWING_NOT_NEED,
    STACK_REDUCING_NOT_NEED,
    STACK_REDUCING_NOT_ALLOWED,
    STACK_CANARY_DIED,
    STACK_INVALID_HASH,
} StackError;

static const char *STR_STACK_ERRORS[] = {
    "STACK_OK",
    "STACK_MEMORY_ERROR",
    "STACK_NULL_DATA",
    "STACK_NOT_NULL_DATA",
    "STACK_NULL_CAPACITY",
    "STACK_NOT_NULL_CAPACITY",
    "STACK_NOT_NULL_SIZE",
    "STACK_NOT_NULL_CANARY",
    "STACK_NULL_HASH",
    "STACK_NOT_NULL_HASH",
    "STACK_NULL_RIGHT_DATA_CANARY_INDEX",
    "STACK_NOT_NULL_RIGHT_DATA_CANARY_INDEX",
    "STACK_SIZE_GREATER_THAN_CAPACITY",
    "STACK_OVERFLOW",
    "STACK_EMPTY",
    "STACK_GROWING_NOT_NEED",
    "STACK_REDUCING_NOT_NEED",
    "STACK_REDUCING_NOT_ALLOWED",
    "STACK_CANARY_DIED",
    "STACK_INVALID_HASH",
};

typedef struct Stack
{
    ON_DEBUG(size_t canary_left;)

    ON_DEBUG(const char *name;
             const char *file;
             const char *function;
             int line;)

    stack_el_t *data;
    size_t size;
    size_t capacity;

    ON_DEBUG(size_t stack_hash;
             size_t right_data_canary_index;)

    ON_DEBUG(size_t canary_right;)

} Stack;

StackError __init_stack(Stack *stk, size_t capacity ON_DEBUG(,
                                                             const char *name,
                                                             const char *file,
                                                             const char *function,
                                                             int line));

#ifdef NSTKDEBUG
#define init_stack(stk, capacity) __init_stack(stk, capacity)
#else
#define init_stack(stk, capacity) __init_stack(stk, capacity, #stk, __FILE__, __func__, __LINE__)
#endif // NSTKDEBUG

void dump_stack(Stack *stk);
StackError push_stack(Stack *stk, stack_el_t value);
StackError pop_stack(Stack *stk, stack_el_t *buffer);
StackError destroy_stack(Stack *stk);
StackError is_stack_empty(Stack *stk);
StackError is_stack_ok(Stack *stk);
const char *get_stack_error(StackError error);
size_t get_capacity(Stack *stk);
size_t get_size(Stack *stk);

#endif // STACK_H
