#ifndef EVALSTACK_H
#define EVALSTACK_H

#include "stackitem.h"

#define EVALBLOCK_NUMBER_OF_ITEMS 64

#define EVALBLOCK_ITEMS_BYTES (sizeof(StackItem) * EVALBLOCK_NUMBER_OF_ITEMS)

typedef struct EvalBlock
{
    StackItem *items;
    struct EvalBlock *next;
} EvalBlock;

EvalBlock *evalstack_new();

EvalBlock *evalstack_grow(EvalBlock *evalstack);

EvalBlock *evalstack_shrink(EvalBlock *evalstack);

#endif