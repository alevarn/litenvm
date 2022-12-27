#ifndef CALLSTACK_H
#define CALLSTACK_H

#include "evalstack.h"

typedef Stack CallStack;

typedef struct
{
    size_t vars_count;
    EvalStackElement *vars;
    uint32_t return_address;
} CallStackFrame;

CallStack *callstack_new();

void callstack_free(CallStack *callstack);

void callstack_push(CallStack *callstack, CallStackFrame frame);

CallStackFrame callstack_top(CallStack *callstack);

void callstack_pop(CallStack *callstack);

#endif