#ifndef STACK_H
#define STACK_H

#include <stddef.h>

typedef int stack_el_t;

// уменьшение размера стека будет работать только если его размер превышает данное значение
const size_t MIN_STACK_CAPACITY_TO_REDUCE = 100;

typedef enum StackError
{
    STACK_OK,
    STACK_MEMORY_ERROR,
    STACK_NULL_DATA,
    STACK_NOT_NULL_DATA,
    STACK_NULL_CAPACITY,
    STACK_NOT_NULL_CAPACITY,
    STACK_NOT_NULL_SIZE,
    STACK_SIZE_GREATER_THAN_CAPACITY,
    STACK_OVERFLOW,
    STACK_EMPTY,
    STACK_GROWING_NOT_NEED,
    STACK_REDUCING_NOT_NEED,
    STACK_REDUCING_NOT_ALLOWED,
} StackError;

static const char *STR_STACK_ERRORS[] = {
    "STACK_OK",
    "STACK_MEMORY_ERROR",
    "STACK_NULL_DATA",
    "STACK_NOT_NULL_DATA",
    "STACK_NULL_CAPACITY",
    "STACK_NOT_NULL_CAPACITY",
    "STACK_NOT_NULL_SIZE",
    "STACK_SIZE_GREATER_THAN_CAPACITY",
    "STACK_OVERFLOW",
    "STACK_EMPTY",
    "STACK_GROWING_NOT_NEED",
    "STACK_REDUCING_NOT_NEED",
    "STACK_REDUCING_NOT_ALLOWED",
};

typedef struct Stack
{
    stack_el_t *data;
    size_t size;
    size_t capacity;

} Stack;

StackError init_stack(Stack *stk, size_t capacity);
StackError push_stack(Stack *stk, stack_el_t value);
StackError pop_stack(Stack *stk, stack_el_t *buffer);
StackError destroy_stack(Stack *stk);
StackError is_stack_empty(Stack *stk);
StackError is_stack_ok(Stack *stk);
const char *get_stack_error(StackError error);
size_t get_capacity(Stack *stk);
size_t get_size(Stack *stk);

#endif // STACK_H
