#include "config.h"
#include "evalstack.h"

static EvalBlock *alloc_block()
{
    EvalBlock *block = (EvalBlock *)config.allocator(sizeof(EvalBlock));
    block->items = (StackItem *)config.allocator(sizeof(StackItem) * config.evalblock_length);
    block->next = NULL;
    return block;
}

static void dealloc_block(EvalBlock *evalstack)
{
    config.deallocator(evalstack->items);
    config.deallocator(evalstack);
}

EvalBlock *evalstack_new()
{
    return alloc_block();
}

EvalBlock *evalstack_grow(EvalBlock *evalstack)
{
    EvalBlock *new_head = alloc_block();
    new_head->next = evalstack;
    return new_head;
}

EvalBlock *evalstack_shrink(EvalBlock *evalstack)
{
    EvalBlock *new_head = evalstack->next;
    dealloc_block(evalstack);
    return new_head;
}
