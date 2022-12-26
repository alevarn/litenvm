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

#endif