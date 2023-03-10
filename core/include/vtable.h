#ifndef VTABLE_H
#define VTABLE_H

#include <stdint.h>
#include <stdbool.h>

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

VTable *vtable_new(uint32_t length);

void vtable_put(VTable *vtable, VTableEntry entry);

bool vtable_exists(VTable *vtable, const char *method_name);

uint32_t vtable_get(VTable *vtable, const char *method_name);

void vtable_free(VTable *vtable);

uint32_t vtable_hash(VTable *vtable, const char *method_name);

void vtable_copy(VTable *dest, VTable *src);

uint32_t vtable_size(VTable *vtable);

#endif