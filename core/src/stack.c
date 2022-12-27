#include <string.h>

#include "config.h"
#include "stack.h"

static void resize(Stack *stack, size_t new_capacity)
{
    stack->capacity = new_capacity;
    stack->elements = config._realloc(stack->elements, new_capacity * stack->elemsize);
}

static void *offset(Stack *stack, size_t index)
{
    return (char *)stack->elements + index * stack->elemsize;
}

Stack stack_new(size_t elemsize)
{
    void *elements = config._malloc(config.min_stack_capacity * elemsize);

    return (Stack){.capacity = config.min_stack_capacity,
                   .length = 0,
                   .elemsize = elemsize,
                   .elements = elements};
}

void stack_free(Stack *stack)
{
    config._free(stack->elements);
    stack->elements = NULL;
}

void stack_push(Stack *stack, void *element)
{
    if (stack->length == stack->capacity)
    {
        resize(stack, stack->capacity * 2);
    }

    memcpy(offset(stack, stack->length), element, stack->elemsize);

    stack->length++;
}

void *stack_top(Stack *stack)
{
    return offset(stack, stack->length - 1);
}

void stack_pop(Stack *stack)
{
    stack->length--;

    if (stack->length > 0 && stack->length >= config.min_stack_capacity / 2 && stack->length == stack->capacity / 4)
    {
        resize(stack, stack->capacity / 2);
    }
}
