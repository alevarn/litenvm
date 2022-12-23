#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <stdbool.h>

#include "instruction.h"
#include "evalstack.h"
#include "callstack.h"
#include "constantpool.h"

typedef struct
{
    InstructionStream stream;
    EvalStack evalstack;
    CallStack callstack;
    ConstantPool constpool;
} Executor;

Executor executor_new(Instruction *instructions, size_t length);

void executor_free(Executor *executor);

bool executor_step(Executor *executor);

void executor_step_all(Executor *executor);

void executor_call_method(Executor *executor, uint32_t method_address, uint32_t args_count, uint32_t locals_count);

void executor_exit_method(Executor *executor);

#endif