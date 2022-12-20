#ifndef EVALSTACK_H
#define EVALSTACK_H

#include "stackitem.h"

typedef struct EvalBlock
{
    StackItem *items;
    struct EvalBlock *next;
} EvalBlock;

EvalBlock *evalstack_new();

EvalBlock *evalstack_grow(EvalBlock *evalstack);

EvalBlock *evalstack_shrink(EvalBlock *evalstack);

#endif