#ifndef CONSTANTPOOL_H
#define CONSTANTPOOL_H

#include <stdint.h>

#include "vtable.h"

#define TYPE_CLASS 0
#define TYPE_FIELD 1
#define TYPE_METHOD 2

typedef struct
{
    const char *name;
    uint32_t parent;
    uint32_t fields;
    uint32_t methods;
    VTable *vtable;
} ConstantPoolEntryClass;

typedef struct
{
    const char *name;
    uint32_t _class;
    uint32_t index;
} ConstantPoolEntryField;

typedef struct
{
    bool virtual;
    const char *name;
    uint32_t _class;
    uint32_t address;
    uint32_t args;
    uint32_t locals;
} ConstantPoolEntryMethod;

typedef struct
{
    uint8_t type;
    union
    {
        ConstantPoolEntryClass _class;
        ConstantPoolEntryField field;
        ConstantPoolEntryMethod method;
    } data;
} ConstantPoolEntry;

typedef struct
{
    uint32_t length;
    ConstantPoolEntry *entries;
} ConstantPool;

ConstantPool *constantpool_new(uint32_t length);

void constantpool_free(ConstantPool *constpool);

void constantpool_add(ConstantPool *constpool, uint32_t index, ConstantPoolEntry entry);

ConstantPoolEntry *constantpool_get(ConstantPool *constpool, uint32_t index);

void constantpool_compute_vtables(ConstantPool *constpool);

#endif