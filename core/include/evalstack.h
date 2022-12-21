#ifndef EVALSTACK_H
#define EVALSTACK_H

#include "stack.h"

typedef Stack EvalStack;

typedef union
{
    int integer;
    void *pointer;
} EvalStackElement;

EvalStack evalstack_new();

void evalstack_free(EvalStack *evalstack);

void evalstack_push(EvalStack *evalstack, EvalStackElement element);

EvalStackElement evalstack_top(EvalStack *evalstack);

void evalstack_pop(EvalStack *evalstack);

#endif