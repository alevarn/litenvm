#include "callstack.h"

CallStack callstack_new()
{
    return stack_new(sizeof(CallStackFrame));
}

void callstack_free(CallStack *callstack)
{
    stack_free(callstack);
}

void callstack_push(CallStack *callstack, CallStackFrame frame)
{
    stack_push(callstack, &frame);
}

CallStackFrame callstack_top(CallStack *callstack)
{
    return *(CallStackFrame *)stack_top(callstack);
}

void callstack_pop(CallStack *callstack)
{
    stack_pop(callstack);
}