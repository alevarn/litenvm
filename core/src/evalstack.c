#include "evalstack.h"

EvalStack evalstack_new()
{
    return stack_new(sizeof(EvalStackElement));
}

void evalstack_free(EvalStack *evalstack)
{
    stack_free(evalstack);
}

void evalstack_push(EvalStack *evalstack, EvalStackElement element)
{
    stack_push(evalstack, &element);
}

EvalStackElement evalstack_top(EvalStack *evalstack)
{
    return *(EvalStackElement *)stack_top(evalstack);
}

void evalstack_pop(EvalStack *evalstack)
{
    stack_pop(evalstack);
}