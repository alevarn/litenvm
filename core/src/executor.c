#include <string.h>
#include <stdio.h>

#include "object.h"
#include "string_class.h"
#include "config.h"
#include "executor.h"

Executor *executor_new(ConstantPool *constpool, InstructionStream *inststream)
{
    Executor *executor = (Executor *)config._malloc(sizeof(Executor));
    executor->constpool = constpool;
    executor->inststream = inststream;
    executor->evalstack = evalstack_new(),
    executor->callstack = callstack_new();
    return executor;
}

void executor_free(Executor *executor)
{
    evalstack_free(executor->evalstack);
    executor->evalstack = NULL;
    callstack_free(executor->callstack);
    executor->callstack = NULL;
    config._free(executor);
}

static EvalStackElement op_add(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer + right.integer};
}

static EvalStackElement op_sub(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer - right.integer};
}

static EvalStackElement op_mul(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer * right.integer};
}

static EvalStackElement op_div(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer / right.integer};
}

static EvalStackElement op_eq(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = memcmp(&left, &right, sizeof(EvalStackElement)) == 0};
}

static EvalStackElement op_ne(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = memcmp(&left, &right, sizeof(EvalStackElement)) != 0};
}

static EvalStackElement op_lt(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer < right.integer};
}

static EvalStackElement op_le(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer <= right.integer};
}

static EvalStackElement op_gt(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer > right.integer};
}

static EvalStackElement op_ge(EvalStackElement left, EvalStackElement right)
{
    return (EvalStackElement){.integer = left.integer >= right.integer};
}

static void apply_binary_function(EvalStack *evalstack, EvalStackElement (*op)(EvalStackElement left, EvalStackElement right))
{
    EvalStackElement right = evalstack_top(evalstack);
    evalstack_pop(evalstack);
    EvalStackElement left = evalstack_top(evalstack);
    evalstack_pop(evalstack);
    evalstack_push(evalstack, op(left, right));
}

static void call_method(Executor *executor, uint32_t constpool_method)
{
    ConstantPoolEntryMethod *method = &constantpool_get(executor->constpool, constpool_method)->data.method;

    // Get the runtime class of the object used to call the method in order to find the vtable.
    uint32_t constpool_class = object_get_class(((EvalStackElement *)executor->evalstack->elements + (executor->evalstack->length - method->args))->pointer);
    VTable *vtable = constantpool_get(executor->constpool, constpool_class)->data._class.vtable;

    // Find the correct method to call by looking in the vtable (we do this to achieve runtime polymorphism).
    constpool_method = vtable_get(vtable, method->name);
    method = &constantpool_get(executor->constpool, constpool_method)->data.method;

    uint32_t vars_count = method->args + method->locals;
    CallStackFrame frame = {.return_address = executor->inststream->current + 1,
                            .vars_count = vars_count,
                            .vars = config._malloc(vars_count * sizeof(EvalStackElement))};

    // Load arguments into frame.
    for (int i = method->args - 1; i >= 0; i--)
    {
        frame.vars[i] = evalstack_top(executor->evalstack);
        evalstack_pop(executor->evalstack);
    }

    // Update program counter.
    executor->inststream->current = method->address;

    callstack_push(executor->callstack, frame);
}

static void exit_method(Executor *executor)
{
    CallStackFrame frame = callstack_top(executor->callstack);

    executor->inststream->current = frame.return_address;

    config._free(frame.vars);

    callstack_pop(executor->callstack);
}

static void new_object(Executor *executor, uint32_t constpool_class)
{
    ConstantPoolEntryClass *_class = &constantpool_get(executor->constpool, constpool_class)->data._class;
    evalstack_push(executor->evalstack, (EvalStackElement){.pointer = object_new(constpool_class, _class->fields)});
}

static EvalStackElement *get_field(Executor *executor, void *object, uint32_t constpool_field)
{
    ConstantPoolEntryField *field = &constantpool_get(executor->constpool, constpool_field)->data.field;
    return object_get_field(object, field->index);
}

static void push_field(Executor *executor, uint32_t constpool_field)
{
    EvalStack *evalstack = executor->evalstack;
    void *object = evalstack_top(evalstack).pointer;
    evalstack_pop(evalstack);
    evalstack_push(evalstack, *get_field(executor, object, constpool_field));
}

static void pop_field(Executor *executor, uint32_t constpool_field)
{
    EvalStack *evalstack = executor->evalstack;
    EvalStackElement value = evalstack_top(evalstack);
    evalstack_pop(evalstack);
    void *object = evalstack_top(evalstack).pointer;
    evalstack_pop(evalstack);
    *get_field(executor, object, constpool_field) = value;
}

static void push_string(Executor *executor, uint32_t constpool_string)
{
    const char *value = constantpool_get(executor->constpool, constpool_string)->data.string.value;
    void *string_object = string_new(value);
    evalstack_push(executor->evalstack, (EvalStackElement){.pointer = string_object});
}

bool executor_step(Executor *executor)
{
    size_t current = executor->inststream->current;
    Instruction inst = executor->inststream->instructions[current];
    EvalStack *evalstack = executor->evalstack;
    CallStack *callstack = executor->callstack;
    ConstantPool *constpool = executor->constpool;

    switch (inst.opcode)
    {
    case PUSH:
        evalstack_push(evalstack, (EvalStackElement){.integer = inst.operand});
        break;
    case PUSH_STRING:
        push_string(executor, inst.operand);
        break;
    case PUSH_VAR:
        evalstack_push(evalstack, callstack_top(callstack).vars[inst.operand]);
        break;
    case PUSH_FIELD:
        push_field(executor, inst.operand);
        break;
    case POP:
        evalstack_pop(evalstack);
        break;
    case POP_VAR:
        callstack_top(callstack).vars[inst.operand].integer = evalstack_top(evalstack).integer;
        evalstack_pop(evalstack);
        break;
    case POP_FIELD:
        pop_field(executor, inst.operand);
        break;
    case ADD:
        apply_binary_function(evalstack, op_add);
        break;
    case SUB:
        apply_binary_function(evalstack, op_sub);
        break;
    case MUL:
        apply_binary_function(evalstack, op_mul);
        break;
    case DIV:
        apply_binary_function(evalstack, op_div);
        break;
    case JUMP:
        evalstack_push(evalstack, (EvalStackElement){.integer = 1});
        break;
    case JUMP_EQ:
        apply_binary_function(evalstack, op_eq);
        break;
    case JUMP_NE:
        apply_binary_function(evalstack, op_ne);
        break;
    case JUMP_LT:
        apply_binary_function(evalstack, op_lt);
        break;
    case JUMP_LE:
        apply_binary_function(evalstack, op_le);
        break;
    case JUMP_GT:
        apply_binary_function(evalstack, op_gt);
        break;
    case JUMP_GE:
        apply_binary_function(evalstack, op_ge);
        break;
    case CALL:
        call_method(executor, inst.operand);
        break;
    case RETURN:
        exit_method(executor);

        // The program has finished running.
        if (callstack->length == 0)
        {
            return false;
        }
        break;
    case NEW:
        new_object(executor, inst.operand);
        break;
    case DUP:
        evalstack_push(evalstack, evalstack_top(evalstack));
        break;
    }

    if (inst.opcode & JUMP_BIT)
    {
        int32_t jump = evalstack_top(evalstack).integer;
        evalstack_pop(evalstack);

        if (jump)
        {
            executor->inststream->current = inst.operand;
        }
        else
        {
            executor->inststream->current++;
        }
    }
    else if (inst.opcode != CALL && inst.opcode != RETURN)
    {
        executor->inststream->current++;
    }

    return true;
}

void executor_step_all(Executor *executor)
{
    while (executor_step(executor))
        ;
}