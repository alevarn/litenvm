#include <math.h>
#include <string.h>

#include "config.h"
#include "vtable.h"

VTable *vtable_new(ConstantPoolEntryClass _class)
{
    uint32_t table_len = _class.methods * 2;
    VTable *vtable = (VTable *)config._malloc(sizeof(VTable));
    vtable->length = table_len;
    vtable->table = (VTableEntry *)config._calloc(table_len, sizeof(VTableEntry));
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

uint32_t vtable_hash(VTable *vtable, const char *str)
{
    uint32_t hash = 0;
    uint32_t n = strlen(str);

    // Algorithm from: Java's String hashcode().
    for (char *c = str; *c != '\0'; c++)
    {
        hash += *c * powl(31, n - 1);
        n--;
    }

    return hash % vtable->length;
}
