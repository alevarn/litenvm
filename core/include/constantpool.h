#ifndef CONSTANTPOOL_H
#define CONSTANTPOOL_H

#include <stdint.h>

#include "vtable.h"

#define BUILTIN_CONSTPOOL_ENTRIES 8
#define CONSTPOOL_CLASS_STRING (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 1))
#define CONSTPOOL_CLASS_CONSOLE (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 2))
#define CONSTPOOL_METHOD_CONSOLE_PRINTLN (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 3))
#define CONSTPOOL_CLASS_STRING_BUILDER (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 4))
#define CONSTPOOL_METHOD_STRING_BUILDER_APPEND_STRING (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 5))
#define CONSTPOOL_METHOD_STRING_BUILDER_APPEND_INT (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 6))
#define CONSTPOOL_METHOD_STRING_BUILDER_APPEND_BOOL (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 7))
#define CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING (UINT32_MAX - (BUILTIN_CONSTPOOL_ENTRIES - 8))

#define TYPE_CLASS 0
#define TYPE_FIELD 1
#define TYPE_METHOD 2
#define TYPE_STRING 3

typedef struct
{
    char *name;
    uint32_t parent;
    uint32_t fields;
    uint32_t methods;
    VTable *vtable;
} ConstantPoolEntryClass;

typedef struct
{
    char *name;
    uint32_t _class;
    uint32_t index;
} ConstantPoolEntryField;

typedef struct
{
    char *name;
    uint32_t _class;
    uint32_t address;
    uint32_t args;
    uint32_t locals;
} ConstantPoolEntryMethod;

typedef struct
{
    char *value;
} ConstantPoolEntryString;

typedef struct
{
    uint8_t type;
    union
    {
        ConstantPoolEntryClass _class;
        ConstantPoolEntryField field;
        ConstantPoolEntryMethod method;
        ConstantPoolEntryString string;
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