#include <string.h>

#include "config.h"
#include "executor.h"

Executor executor_new(Instruction *instructions, size_t length)
{
    return (Executor){
        .stream = (InstructionStream){
            .length = length,
            .current = 0,
            .instructions = instructions},
        .evalstack = evalstack_new(),
        .callstack = callstack_new()};
}

void executor_free(Executor *executor)
{
    evalstack_free(&executor->evalstack);
}

static EvalStackElement add(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer + right.integer};
}

static EvalStackElement sub(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer - right.integer};
}

static EvalStackElement mul(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer * right.integer};
}

static EvalStackElement div(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer / right.integer};
}

static EvalStackElement eq(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = memcmp(&left, &right, sizeof(EvalStackElement)) == 0};
}

static EvalStackElement ne(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = memcmp(&left, &right, sizeof(EvalStackElement)) != 0};
}

static EvalStackElement lt(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer < right.integer};
}

static EvalStackElement le(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer <= right.integer};
}

static EvalStackElement gt(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer > right.integer};
}

static EvalStackElement ge(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer >= right.integer};
}

static void binary_function(EvalStack *evalstack, EvalStackElement (*op)(EvalStackElement left, EvalStackElement right))
{
    EvalStackElement right = evalstack_top(evalstack);
    evalstack_pop(evalstack);
    EvalStackElement left = evalstack_top(evalstack);
    evalstack_pop(evalstack);
    evalstack_push(evalstack, op(left, right));
}

void executor_call_method(Executor *executor, ConstantPoolEntryMethod *method)
{
    uint32_t vars_count = method->args + method->locals;
    CallStackFrame frame = {.return_address = executor->stream.current + 1,
                            .vars_count = vars_count,
                            .vars = config._malloc(vars_count * sizeof(EvalStackElement))};

    // Load arguments into frame.
    for (int i = method->args - 1; i >= 0; i--)
    {
        frame.vars[i] = evalstack_top(&executor->evalstack);
        evalstack_pop(&executor->evalstack);
    }

    // Update program counter.
    executor->stream.current = method->address;

    callstack_push(&executor->callstack, frame);
}

void executor_exit_method(Executor *executor)
{
    CallStackFrame frame = callstack_top(&executor->callstack);

    executor->stream.current = frame.return_address;

    config._free(executor->stream.instructions);

    callstack_pop(&executor->callstack);
}

void executor_new_object(Executor *executor, ConstantPoolEntryClass *_class)
{
    EvalStack *evalstack = &executor->evalstack;
    void *object = config._malloc(_class->fields * sizeof(EvalStackElement));
    evalstack_push(evalstack, (EvalStackElement){.pointer = object});
}

static EvalStackElement *get_field(void *object, ConstantPoolEntryField *field)
{
    return (EvalStackElement *)((char *)object + field->index * sizeof(EvalStackElement));
}

void executor_push_field(Executor *executor, ConstantPoolEntryField *field)
{
    EvalStack *evalstack = &executor->evalstack;
    void *object = evalstack_top(evalstack).pointer;
    evalstack_pop(evalstack);
    evalstack_push(evalstack, *get_field(object, field));
}

void executor_pop_field(Executor *executor, ConstantPoolEntryField *field)
{
    EvalStack *evalstack = &executor->evalstack;
    EvalStackElement value = evalstack_top(evalstack);
    evalstack_pop(evalstack);
    void *object = evalstack_top(evalstack).pointer;
    evalstack_pop(evalstack);
    *get_field(object, field) = value;
}

bool executor_step(Executor *executor)
{
    size_t current = executor->stream.current;
    Instruction inst = executor->stream.instructions[current];
    EvalStack *evalstack = &executor->evalstack;
    CallStack *callstack = &executor->callstack;
    ConstantPool *constpool = &executor->constpool;

    switch (inst.opcode)
    {
    case PUSH:
        evalstack_push(evalstack, (EvalStackElement){.integer = inst.operand});
        break;
    case PUSH_CONST:
        // Implement
        break;
    case PUSH_VAR:
        evalstack_push(evalstack, callstack_top(callstack).vars[inst.operand]);
        break;
    case PUSH_FIELD:
        executor_push_field(executor, &constantpool_get(constpool, inst.operand)->data.field);
        break;
    case POP:
        evalstack_pop(evalstack);
        break;
    case POP_VAR:
        callstack_top(callstack).vars[inst.operand].integer = evalstack_top(evalstack).integer;
        evalstack_pop(evalstack);
        break;
    case POP_FIELD:
        executor_pop_field(executor, &constantpool_get(constpool, inst.operand)->data.field);
        break;
    case ADD:
        binary_function(evalstack, add);
        break;
    case SUB:
        binary_function(evalstack, sub);
        break;
    case MUL:
        binary_function(evalstack, mul);
        break;
    case DIV:
        binary_function(evalstack, div);
        break;
    case JUMP:
        evalstack_push(evalstack, (EvalStackElement){.integer = 1});
        break;
    case JUMP_EQ:
        binary_function(evalstack, eq);
        break;
    case JUMP_NE:
        binary_function(evalstack, ne);
        break;
    case JUMP_LT:
        binary_function(evalstack, lt);
        break;
    case JUMP_LE:
        binary_function(evalstack, le);
        break;
    case JUMP_GT:
        binary_function(evalstack, gt);
        break;
    case JUMP_GE:
        binary_function(evalstack, ge);
        break;
    case CALL:
        executor_call_method(executor, &constantpool_get(constpool, inst.operand)->data.method);
        break;
    case RETURN:
        executor_exit_method(executor);

        // The program has finished running.
        if (callstack->length == 0)
        {
            return false;
        }
        break;
    case NEW:
        executor_new_object(executor, &constantpool_get(constpool, inst.operand)->data._class);
        break;
    case DUP:
        evalstack_push(evalstack, evalstack_top(evalstack));
        break;
    }

    if (inst.opcode & JUMP_BIT && evalstack_top(evalstack).integer)
    {
        evalstack_pop(evalstack);
        executor->stream.current = inst.operand;
    }
    else if (inst.opcode != CALL && inst.opcode != RETURN)
    {
        executor->stream.current++;
    }

    return true;
}

void executor_step_all(Executor *executor)
{
    while (executor_step(executor))
        ;
}