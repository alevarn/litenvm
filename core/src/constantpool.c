#include "config.h"
#include "constantpool.h"

ConstantPool constantpool_new(uint32_t length)
{
    return (ConstantPool){.length = length, .entries = config._malloc(length * sizeof(ConstantPoolEntry))};
}

void constantpool_free(ConstantPool *constpool)
{
    config._free(constpool->entries);
    constpool->entries = NULL;
}

void constantpool_add(ConstantPool *constpool, uint32_t index, ConstantPoolEntry entry)
{
    constpool->entries[index] = entry;
}

ConstantPoolEntry *constantpool_get(ConstantPool *constpool, uint32_t index)
{
    return &constpool->entries[index];
}