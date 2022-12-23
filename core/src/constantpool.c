#include "config.h"
#include "constantpool.h"

ConstantPool constantpool_new(uint32_t length)
{
    return (ConstantPool){.length = length, .entries = config._malloc(length * sizeof(ConstantPoolEntry))};
}

void constantpool_free(ConstantPool *pool)
{
    config._free(pool->entries);
    pool->entries = NULL;
}

void constantpool_add(ConstantPool *pool, uint32_t index, ConstantPoolEntry entry)
{
    pool->entries[index] = entry;
}

ConstantPoolEntry *constantpool_get(ConstantPool *pool, uint32_t index)
{
    return &pool->entries[index];
}