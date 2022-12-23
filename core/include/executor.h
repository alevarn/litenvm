#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdbool.h>

#include "instruction.h"
#include "evalstack.h"
#include "callstack.h"
#include "constantpool.h"

typedef struct
{
        ConstantPool constpool;
    InstructionStream stream;
    EvalStack evalstack;
    CallStack callstack;
} Executor;

Executor executor_new(ConstantPool constpool, InstructionStream inststream);

void executor_free(Executor *executor);

bool executor_step(Executor *executor);

void executor_step_all(Executor *executor);

void executor_call_method(Executor *executor, ConstantPoolEntryMethod *method);

void executor_exit_method(Executor *executor);

void executor_new_object(Executor *executor, ConstantPoolEntryClass *_class);

void executor_push_field(Executor *executor, ConstantPoolEntryField *field);

void executor_pop_field(Executor *executor, ConstantPoolEntryField *field);

#endif