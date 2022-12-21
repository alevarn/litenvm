#ifndef STACK_H
#define STACK_H

#include <stddef.h>

typedef struct
{
    size_t capacity;
    size_t length;
    size_t elemsize;
    void *elements;
} Stack;

Stack stack_new(size_t elemsize);

void stack_free(Stack *stack);

void stack_push(Stack *stack, void *element);

void *stack_top(Stack *stack);

void stack_pop(Stack *stack);

#endif