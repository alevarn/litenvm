#ifndef VTABLE_H
#define VTABLE_H

#include <stdbool.h>

#include "constantpool.h"

typedef struct
{
    const char *method_name;
    uint32_t const_index;
} VTableEntry;

typedef struct
{
    VTableEntry *table;
    uint32_t length;
} VTable;

VTable *vtable_new(ConstantPoolEntryClass _class);

void vtable_put(VTable *vtable, VTableEntry entry);

bool vtable_exists(VTable *vtable, const char *method_name);

uint32_t vtable_get(VTable *vtable, const char *method_name);

void vtable_free(VTable *vtable);

uint32_t vtable_hash(const char *str);

#endif