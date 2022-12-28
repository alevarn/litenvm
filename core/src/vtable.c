#include <math.h>
#include <string.h>

#include "config.h"
#include "vtable.h"

VTable *vtable_new(uint32_t length)
{
    VTable *vtable = (VTable *)config._malloc(sizeof(VTable));
    vtable->length = length;
    vtable->table = (VTableEntry *)config._calloc(length, sizeof(VTableEntry));
    return vtable;
}

void vtable_free(VTable *vtable)
{
    config._free(vtable->table);
    vtable->table = NULL;
    config._free(vtable);
}

void vtable_put(VTable *vtable, VTableEntry entry)
{
    uint32_t index = vtable_hash(vtable, entry.method_name);

    while (true)
    {
        if (vtable->table[index].const_index == 0)
        {
            vtable->table[index] = entry;
            break;
        }
        else if (strcmp(vtable->table[index].method_name, entry.method_name) == 0)
        {
            vtable->table[index] = entry;
            break;
        }
        else
        {
            index = (index + 1) % vtable->length;
        }
    }
}

bool vtable_exists(VTable *vtable, const char *method_name)
{
    uint32_t index = vtable_hash(vtable, method_name);

    while (true)
    {
        if (vtable->table[index].const_index == 0)
        {
            return false;
        }
        else if (strcmp(vtable->table[index].method_name, method_name) == 0)
        {
            return true;
        }
        else
        {
            index = (index + 1) % vtable->length;
        }
    }
}

uint32_t vtable_get(VTable *vtable, const char *method_name)
{
    uint32_t index = vtable_hash(vtable, method_name);

    while (true)
    {
        if (vtable->table[index].const_index == 0)
        {
            return 0;
        }
        else if (strcmp(vtable->table[index].method_name, method_name) == 0)
        {
            return vtable->table[index].const_index;
        }
        else
        {
            index = (index + 1) % vtable->length;
        }
    }
}

uint32_t vtable_hash(VTable *vtable, const char *method_name)
{
    uint32_t hash = 0;
    uint32_t n = strlen(method_name);

    // Algorithm from: Java's String hashcode().
    while (*method_name != '\0')
    {
        hash += *method_name * powl(31, n - 1);
        n--;
        method_name++;
    }

    return hash % vtable->length;
}

void vtable_copy(VTable *dest, VTable *src)
{
    for (uint32_t i = 0; i < src->length; i++)
    {
        if (src->table[i].const_index != 0)
        {
            vtable_put(dest, src->table[i]);
        }
    }
}

uint32_t vtable_size(VTable *vtable)
{
    uint32_t size = 0;

    for (uint32_t i = 0; i < vtable->length; i++)
    {
        if (vtable->table[i].const_index != 0)
        {
            size++;
        }
    }

    return size;
}