#include <string.h>

#include "config.h"
#include "executor.h"

Executor executor_new(ConstantPool constpool, InstructionStream inststream)
{
    return (Executor){
        .constpool = constpool,
        .stream = inststream,
        .evalstack = evalstack_new(),
        .callstack = callstack_new()};
}

void executor_free(Executor *executor)
{
    evalstack_free(&executor->evalstack);
    callstack_free(&executor->callstack);
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

static void call_method(Executor *executor, ConstantPoolEntryMethod *method)
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

static void exit_method(Executor *executor)
{
    CallStackFrame frame = callstack_top(&executor->callstack);

    executor->stream.current = frame.return_address;

    config._free(frame.vars);

    callstack_pop(&executor->callstack);
}

static void new_object(Executor *executor, uint32_t pool_index, ConstantPoolEntryClass *_class)
{
    EvalStack *evalstack = &executor->evalstack;
    void *object = config._malloc(sizeof(uint32_t) + _class->fields * sizeof(EvalStackElement));
    *(uint32_t *)object = pool_index;
    evalstack_push(evalstack, (EvalStackElement){.pointer = object});
}

static EvalStackElement *get_field(void *object, ConstantPoolEntryField *field)
{
    return (EvalStackElement *)((char *)object + sizeof(uint32_t) + field->index * sizeof(EvalStackElement));
}

static void push_field(Executor *executor, ConstantPoolEntryField *field)
{
    EvalStack *evalstack = &executor->evalstack;
    void *object = evalstack_top(evalstack).pointer;
    evalstack_pop(evalstack);
    evalstack_push(evalstack, *get_field(object, field));
}

static void pop_field(Executor *executor, ConstantPoolEntryField *field)
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
        push_field(executor, &constantpool_get(constpool, inst.operand)->data.field);
        break;
    case POP:
        evalstack_pop(evalstack);
        break;
    case POP_VAR:
        callstack_top(callstack).vars[inst.operand].integer = evalstack_top(evalstack).integer;
        evalstack_pop(evalstack);
        break;
    case POP_FIELD:
        pop_field(executor, &constantpool_get(constpool, inst.operand)->data.field);
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
        call_method(executor, &constantpool_get(constpool, inst.operand)->data.method);
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
        new_object(executor, inst.operand, &constantpool_get(constpool, inst.operand)->data._class);
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
            executor->stream.current = inst.operand;
        }
        else
        {
            executor->stream.current++;
        }
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