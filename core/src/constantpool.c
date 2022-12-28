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
    // Free all vtables.
    for (uint32_t i = 1; i <= constpool->length; i++)
    {
        ConstantPoolEntry *entry = constantpool_get(constpool, i);
        if (entry->type == TYPE_CLASS)
        {
            VTable *vtable = entry->data._class.vtable;
            if (vtable)
            {
                vtable_free(vtable);
            }
        }
    }
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

void constantpool_compute_vtables(ConstantPool *constpool)
{
    // The vtable of the super class must be fully defined in the constant pool before we can construct the vtable of the subclass.
    // Classes must be defined before their methods in the constant pool.
    for (uint32_t i = 1; i <= constpool->length; i++)
    {
        ConstantPoolEntry *entry = constantpool_get(constpool, i);

        if (entry->type == TYPE_CLASS)
        {
            // Create a new vtable for the class.
            ConstantPoolEntryClass *_class = &entry->data._class;
            _class->vtable = vtable_new(_class->methods * 2);

            // Copy the vtable of the parent.
            if (_class->parent != 0)
            {
                ConstantPoolEntryClass *parent_class = &constantpool_get(constpool, _class->parent)->data._class;
                vtable_copy(_class->vtable, parent_class->vtable);
            }
        }
        else if (entry->type == TYPE_METHOD)
        {
            // Add the method to the vtable (may override a previous method definition with the same name).
            ConstantPoolEntryMethod *method = &entry->data.method;
            ConstantPoolEntryClass *_class = &constantpool_get(constpool, method->_class)->data._class;
            vtable_put(_class->vtable, (VTableEntry){.method_name = method->name, .const_index = i});
        }
    }
}