#include <string.h>

#include "config.h"
#include "binary_format.h"

// We want to use htonl/ntohl to ensure big-endian in the binary format.
#ifdef __unix__
#include <arpa/inet.h>
#elif defined(_WIN32) || defined(WIN32)
#include <winsock.h>
#endif

// Write big-endian.
static void write_uint32_big_endian(FILE *file, uint32_t value)
{
    uint32_t big_endian = htonl(value);
    fwrite(&big_endian, sizeof(uint32_t), 1, file);
}

// Read big-endian and convert back to the host-endianness.
static void read_uint32_big_endian(FILE *file, uint32_t *value)
{
    uint32_t big_endian;
    fread(&big_endian, sizeof(uint32_t), 1, file);
    *value = ntohl(big_endian);
}

void binform_write_constantpool(FILE *file, ConstantPool *constpool)
{
    write_uint32_big_endian(file, constpool->length);
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
            write_uint32_big_endian(file, name_len);
            fwrite(_class.name, name_len, 1, file);
            write_uint32_big_endian(file, _class.parent);
            write_uint32_big_endian(file, _class.fields);
            write_uint32_big_endian(file, _class.methods);
        }
        break;
        case TYPE_FIELD:
        {
            ConstantPoolEntryField field = constpool->entries[i - 1].data.field;
            uint32_t name_len = strlen(field.name) + 1;
            write_uint32_big_endian(file, name_len);
            fwrite(field.name, name_len, 1, file);
            write_uint32_big_endian(file, field._class);
            write_uint32_big_endian(file, field.index);
        }
        break;
        case TYPE_METHOD:
        {
            ConstantPoolEntryMethod method = constpool->entries[i - 1].data.method;
            uint32_t name_len = strlen(method.name) + 1;
            write_uint32_big_endian(file, name_len);
            fwrite(method.name, name_len, 1, file);
            write_uint32_big_endian(file, method._class);
            write_uint32_big_endian(file, method.address);
            write_uint32_big_endian(file, method.args);
            write_uint32_big_endian(file, method.locals);
        }
        break;
        case TYPE_STRING:
        {
            ConstantPoolEntryString string = constpool->entries[i - 1].data.string;
            uint32_t value_len = strlen(string.value) + 1;
            write_uint32_big_endian(file, value_len);
            fwrite(string.value, value_len, 1, file);
        }
        break;
        }
    }
}

void binform_write_instructions(FILE *file, InstructionStream *inststream)
{
    write_uint32_big_endian(file, inststream->length);
    for (uint32_t i = 0; i < inststream->length; i++)
    {
        fwrite(&inststream->instructions[i].opcode, sizeof(uint8_t), 1, file);
        write_uint32_big_endian(file, inststream->instructions[i].operand);
    }
}

ConstantPool *binform_read_constantpool(FILE *file)
{
    uint32_t length;
    read_uint32_big_endian(file, &length);
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
            read_uint32_big_endian(file, &name_len);
            _class.name = config._malloc(name_len);
            fread(_class.name, name_len, 1, file);
            read_uint32_big_endian(file, &_class.parent);
            read_uint32_big_endian(file, &_class.fields);
            read_uint32_big_endian(file, &_class.methods);
            _class.vtable = NULL;
            constantpool_add(constpool, i, (ConstantPoolEntry){.type = type, .data._class = _class});
        }
        break;
        case TYPE_FIELD:
        {
            ConstantPoolEntryField field;
            uint32_t name_len;
            read_uint32_big_endian(file, &name_len);
            field.name = config._malloc(name_len);
            fread(field.name, name_len, 1, file);
            read_uint32_big_endian(file, &field._class);
            read_uint32_big_endian(file, &field.index);
            constantpool_add(constpool, i, (ConstantPoolEntry){.type = type, .data.field = field});
        }
        break;
        case TYPE_METHOD:
        {
            ConstantPoolEntryMethod method;
            uint32_t name_len;
            read_uint32_big_endian(file, &name_len);
            method.name = config._malloc(name_len);
            fread(method.name, name_len, 1, file);
            read_uint32_big_endian(file, &method._class);
            read_uint32_big_endian(file, &method.address);
            read_uint32_big_endian(file, &method.args);
            read_uint32_big_endian(file, &method.locals);
            constantpool_add(constpool, i, (ConstantPoolEntry){.type = type, .data.method = method});
        }
        break;
        case TYPE_STRING:
        {
            ConstantPoolEntryString string;
            uint32_t value_len;
            read_uint32_big_endian(file, &value_len);
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
    read_uint32_big_endian(file, &length);
    InstructionStream *inststream = inststream_new(length);

    for (uint32_t i = 0; i < inststream->length; i++)
    {
        fread(&inststream->instructions[i].opcode, sizeof(uint8_t), 1, file);
        read_uint32_big_endian(file, &inststream->instructions[i].operand);
    }

    return inststream;
}

void binform_print(ConstantPool *constpool, InstructionStream *inststream)
{
    if (constpool)
    {
        printf("Constant pool:\n");
        for (uint32_t i = 1; i <= constpool->length; i++)
        {
            ConstantPoolEntry *entry = constantpool_get(constpool, i);

            switch (entry->type)
            {
            case TYPE_CLASS:
                printf("#%d\t\tCLASS\t\t%s (parent=%d, fields=%d, methods=%d)\n", i, entry->data._class.name, entry->data._class.parent, entry->data._class.fields, entry->data._class.methods);
                break;
            case TYPE_FIELD:
                printf("#%d\t\tFIELD\t\t%s (class=%d, index=%d)\n", i, entry->data.field.name, entry->data.field._class, entry->data.field.index);
                break;
            case TYPE_METHOD:
                printf("#%d\t\tMETHOD\t\t%s (class=%d, address=%d, args=%d, locals=%d)\n", i, entry->data.method.name, entry->data.method._class, entry->data.method.address, entry->data.method.args, entry->data.method.locals);
                break;
            case TYPE_STRING:
                printf("#%d\t\tSTRING\t\t\"%s\"\n", i, entry->data.string.value);
                break;
            }
        }
        if (inststream)
        {
            printf("\n");
        }
    }

    if (inststream)
    {
        printf("Instruction stream:\n");
        for (uint32_t i = 0; i < inststream->length; i++)
        {
            uint8_t opcode = inststream->instructions[i].opcode;
            uint32_t operand = inststream->instructions[i].operand;

            switch (opcode)
            {
            case PUSH:
                printf("%d\t\t%s\t\t%d\n", i, "PUSH", operand);
                break;
            case PUSH_STRING:
                printf("%d\t\t%s\t%d\n", i, "PUSH_STRING", operand);
                break;
            case PUSH_VAR:
                printf("%d\t\t%s\t%d\n", i, "PUSH_VAR", operand);
                break;
            case PUSH_FIELD:
                printf("%d\t\t%s\t%d\n", i, "PUSH_FIELD", operand);
                break;
            case POP:
                printf("%d\t\t%s\t\t%d\n", i, "POP", operand);
                break;
            case POP_VAR:
                printf("%d\t\t%s\t%d\n", i, "POP_VAR", operand);
                break;
            case POP_FIELD:
                printf("%d\t\t%s\t%d\n", i, "POP_FIELD", operand);
                break;
            case ADD:
                printf("%d\t\t%s\t\t%d\n", i, "ADD", operand);
                break;
            case SUB:
                printf("%d\t\t%s\t\t%d\n", i, "SUB", operand);
                break;
            case MUL:
                printf("%d\t\t%s\t\t%d\n", i, "MUL", operand);
                break;
            case DIV:
                printf("%d\t\t%s\t\t%d\n", i, "DIV", operand);
                break;
            case CALL:
                printf("%d\t\t%s\t\t%d\n", i, "CALL", operand);
                break;
            case RETURN:
                printf("%d\t\t%s\t\t%d\n", i, "RETURN", operand);
                break;
            case NEW:
                printf("%d\t\t%s\t\t%d\n", i, "NEW", operand);
                break;
            case DUP:
                printf("%d\t\t%s\t\t%d\n", i, "DUP", operand);
                break;
            case JUMP:
                printf("%d\t\t%s\t\t%d\n", i, "JUMP", operand);
                break;
            case JUMP_EQ:
                printf("%d\t\t%s\t\t%d\n", i, "JUMP_EQ", operand);
                break;
            case JUMP_NE:
                printf("%d\t\t%s\t\t%d\n", i, "JUMP_NE", operand);
                break;
            case JUMP_LT:
                printf("%d\t\t%s\t\t%d\n", i, "JUMP_LT", operand);
                break;
            case JUMP_LE:
                printf("%d\t\t%s\t\t%d\n", i, "JUMP_LE", operand);
                break;
            case JUMP_GT:
                printf("%d\t\t%s\t\t%d\n", i, "JUMP_GT", operand);
                break;
            case JUMP_GE:
                printf("%d\t\t%s\t\t%d\n", i, "JUMP_GE", operand);
                break;
            }
        }
    }
}