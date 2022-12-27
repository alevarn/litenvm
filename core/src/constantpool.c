#include "config.h"
#include "constantpool.h"

ConstantPool *constantpool_new(uint32_t length)
{
    ConstantPool *constpool = (ConstantPool *)config._malloc(sizeof(ConstantPool));
    constpool->length = length;
    constpool->entries = (ConstantPoolEntry *)config._malloc(length * sizeof(ConstantPoolEntry));
    return constpool;
}

void constantpool_free(ConstantPool *constpool)
{
    config._free(constpool->entries);
    constpool->entries = NULL;
    config._free(constpool);
}

void constantpool_add(ConstantPool *constpool, uint32_t index, ConstantPoolEntry entry)
{
    constpool->entries[index - 1] = entry;
}

ConstantPoolEntry *constantpool_get(ConstantPool *constpool, uint32_t index)
{
    return &constpool->entries[index - 1];
}