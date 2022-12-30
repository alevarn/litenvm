#include <string.h>

#include "config.h"
#include "binary_format.h"

void binform_write_constantpool(FILE *file, ConstantPool *constpool)
{
    fwrite(&constpool->length, sizeof(uint32_t), 1, file);
    for (uint32_t i = 1; i <= constpool->length; i++)
    {
        uint8_t entry_type = constpool->entries[i - 1].type;
        fwrite(&entry_type, sizeof(uint8_t), 1, file);

        switch (entry_type)
        {
        case TYPE_CLASS:
        {
            ConstantPoolEntryClass _class = constpool->entries[i - 1].data._class;
            uint32_t name_len = strlen(_class.name) + 1;
            fwrite(&name_len, sizeof(uint32_t), 1, file);
            fwrite(_class.name, name_len, 1, file);
            fwrite(&_class.parent, sizeof(uint32_t), 1, file);
            fwrite(&_class.fields, sizeof(uint32_t), 1, file);
            fwrite(&_class.methods, sizeof(uint32_t), 1, file);
        }
        break;
        case TYPE_FIELD:
        {
            ConstantPoolEntryField field = constpool->entries[i - 1].data.field;
            uint32_t name_len = strlen(field.name) + 1;
            fwrite(&name_len, sizeof(uint32_t), 1, file);
            fwrite(field.name, name_len, 1, file);
            fwrite(&field._class, sizeof(uint32_t), 1, file);
            fwrite(&field.index, sizeof(uint32_t), 1, file);
        }
        break;
        case TYPE_METHOD:
        {
            ConstantPoolEntryMethod method = constpool->entries[i - 1].data.method;
            uint32_t name_len = strlen(method.name) + 1;
            fwrite(&name_len, sizeof(uint32_t), 1, file);
            fwrite(method.name, name_len, 1, file);
            fwrite(&method._class, sizeof(uint32_t), 1, file);
            fwrite(&method.address, sizeof(uint32_t), 1, file);
            fwrite(&method.args, sizeof(uint32_t), 1, file);
            fwrite(&method.locals, sizeof(uint32_t), 1, file);
        }
        break;
        case TYPE_STRING:
        {
            ConstantPoolEntryString string = constpool->entries[i - 1].data.string;
            uint32_t value_len = strlen(string.value) + 1;
            fwrite(&value_len, sizeof(uint32_t), 1, file);
            fwrite(string.value, value_len, 1, file);
        }
        break;
        }
    }
}

void binform_write_instructions(FILE *file, InstructionStream *inststream)
{
    fwrite(&inststream->length, sizeof(uint32_t), 1, file);
    for (uint32_t i = 0; i < inststream->length; i++)
    {
        fwrite(&inststream->instructions[i].opcode, sizeof(uint8_t), 1, file);
        fwrite(&inststream->instructions[i].operand, sizeof(uint32_t), 1, file);
    }
}

ConstantPool *binform_read_constantpool(FILE *file)
{
    uint32_t length;
    fread(&length, sizeof(uint32_t), 1, file);
    ConstantPool *constpool = constantpool_new(length);

    for (uint32_t i = 1; i <= length; i++)
    {
        uint8_t type;

        fread(&type, sizeof(uint8_t), 1, file);

        switch (type)
        {
        case TYPE_CLASS:
        {
            ConstantPoolEntryClass _class;
            uint32_t name_len;
            fread(&name_len, sizeof(uint32_t), 1, file);
            _class.name = config._malloc(name_len);
            fread(_class.name, name_len, 1, file);
            fread(&_class.parent, sizeof(uint32_t), 1, file);
            fread(&_class.fields, sizeof(uint32_t), 1, file);
            fread(&_class.methods, sizeof(uint32_t), 1, file);
            _class.vtable = NULL;
            constantpool_add(constpool, i, (ConstantPoolEntry){.type = type, .data._class = _class});
        }
        break;
        case TYPE_FIELD:
        {
            ConstantPoolEntryField field;
            uint32_t name_len;
            fread(&name_len, sizeof(uint32_t), 1, file);
            field.name = config._malloc(name_len);
            fread(field.name, name_len, 1, file);
            fread(&field._class, sizeof(uint32_t), 1, file);
            fread(&field.index, sizeof(uint32_t), 1, file);
            constantpool_add(constpool, i, (ConstantPoolEntry){.type = type, .data.field = field});
        }
        break;
        case TYPE_METHOD:
        {
            ConstantPoolEntryMethod method;
            uint32_t name_len;
            fread(&name_len, sizeof(uint32_t), 1, file);
            method.name = config._malloc(name_len);
            fread(method.name, name_len, 1, file);
            fread(&method._class, sizeof(uint32_t), 1, file);
            fread(&method.address, sizeof(uint32_t), 1, file);
            fread(&method.args, sizeof(uint32_t), 1, file);
            fread(&method.locals, sizeof(uint32_t), 1, file);
            constantpool_add(constpool, i, (ConstantPoolEntry){.type = type, .data.method = method});
        }
        break;
        case TYPE_STRING:
        {
            ConstantPoolEntryString string;
            uint32_t value_len;
            fread(&value_len, sizeof(uint32_t), 1, file);
            string.value = config._malloc(value_len);
            fread(string.value, value_len, 1, file);
            constantpool_add(constpool, i, (ConstantPoolEntry){.type = type, .data.string = string});
        }
        break;
        }
    }

    return constpool;
}

InstructionStream *binform_read_instructions(FILE *file)
{
    uint32_t length;
    fread(&length, sizeof(uint32_t), 1, file);
    InstructionStream *inststream = inststream_new(length);

    for (uint32_t i = 0; i < inststream->length; i++)
    {
        fread(&inststream->instructions[i].opcode, sizeof(uint8_t), 1, file);
        fread(&inststream->instructions[i].operand, sizeof(uint32_t), 1, file);
    }

    return inststream;
}