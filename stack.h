#ifndef STACK_H
#define STACK_H

#include <stddef.h>

typedef int stack_el_t;

typedef enum StackError
{
    STACK_OK,
    STACK_MEMORY_ERROR,
    STACK_NULL_DATA,
    STACK_NOT_NULL_DATA,
    STACK_NULL_CAPACITY,
    STACK_NOT_NULL_CAPACITY,
    STACK_NOT_NULL_SIZE,
    STACK_SIZE_GREATER_THAN_CAPACITY
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
};

typedef struct Stack
{
    stack_el_t *data;
    size_t size;
    size_t capacity;

} Stack;

StackError init_stack(Stack *stk, size_t capacity);
StackError destroy_stack(Stack *stk);
StackError is_stack_empty(Stack *stk);
StackError is_stack_ok(Stack *stk);
const char *get_stack_error(StackError error);

#endif // STACK_H
