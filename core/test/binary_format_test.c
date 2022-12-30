#include "unit_testing.h"

#include "config.h"
#include "binary_format.h"

#define STACK_INITIAL_CAPACITY 8

#define FILE_NAME "test.lvm"

static int setup()
{
    ConstantPool *constpool = constantpool_new(4);
    constantpool_add(constpool, 1, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Animal", .fields = 2, .methods = 3, .parent = 1, .vtable = NULL}});
    constantpool_add(constpool, 2, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "sound", ._class = 1, .address = 2, .args = 3, .locals = 4}});
    constantpool_add(constpool, 3, (ConstantPoolEntry){.type = TYPE_FIELD, .data.field = {.name = "age", ._class = 1, .index = 0}});
    constantpool_add(constpool, 4, (ConstantPoolEntry){.type = TYPE_STRING, .data.string = {.value = "This is a long string"}});
    InstructionStream *inststream = inststream_new(4);
    inststream->instructions[0] = (Instruction){.opcode = PUSH, .operand = 1};
    inststream->instructions[1] = (Instruction){.opcode = POP, .operand = 0};
    inststream->instructions[2] = (Instruction){.opcode = NEW, .operand = 120};
    inststream->instructions[3] = (Instruction){.opcode = JUMP_EQ, .operand = 50};
    FILE *file = fopen(FILE_NAME, "wb");
    binform_write_constantpool(file, constpool);
    binform_write_instructions(file, inststream);
    fclose(file);
    constantpool_free(constpool);
    inststream_free(inststream);
    return 0;
}

static int teardown()
{
    return remove(FILE_NAME);
}

void binary_format_read_test(void **state)
{
    FILE *file = fopen(FILE_NAME, "rb");
    ConstantPool *constpool = binform_read_constantpool(file);

    assert_int_equal(4, constpool->length);

    ConstantPoolEntry *entry = constantpool_get(constpool, 1);
    assert_int_equal(TYPE_CLASS, entry->type);
    assert_string_equal("Animal", entry->data._class.name);
    assert_int_equal(1, entry->data._class.parent);
    assert_int_equal(2, entry->data._class.fields);
    assert_int_equal(3, entry->data._class.methods);
    config._free(entry->data._class.name);

    entry = constantpool_get(constpool, 2);
    assert_int_equal(TYPE_METHOD, entry->type);
    assert_string_equal("sound", entry->data.method.name);
    assert_int_equal(1, entry->data.method._class);
    assert_int_equal(2, entry->data.method.address);
    assert_int_equal(3, entry->data.method.args);
    assert_int_equal(4, entry->data.method.locals);
    config._free(entry->data.method.name);

    entry = constantpool_get(constpool, 3);
    assert_int_equal(TYPE_FIELD, entry->type);
    assert_string_equal("age", entry->data.field.name);
    assert_int_equal(1, entry->data.field._class);
    assert_int_equal(0, entry->data.field.index);
    config._free(entry->data.field.name);

    entry = constantpool_get(constpool, 4);
    assert_int_equal(TYPE_STRING, entry->type);
    assert_string_equal("This is a long string", entry->data.string.value);
    config._free(entry->data.string.value);

    InstructionStream *inststream = binform_read_instructions(file);

    assert_int_equal(4, inststream->length);

    assert_int_equal(PUSH, inststream->instructions[0].opcode);
    assert_int_equal(1, inststream->instructions[0].operand);
    assert_int_equal(POP, inststream->instructions[1].opcode);
    assert_int_equal(0, inststream->instructions[1].operand);
    assert_int_equal(NEW, inststream->instructions[2].opcode);
    assert_int_equal(120, inststream->instructions[2].operand);
    assert_int_equal(JUMP_EQ, inststream->instructions[3].opcode);
    assert_int_equal(50, inststream->instructions[3].operand);

    fclose(file);
    constantpool_free(constpool);
    inststream_free(inststream);
}

int main()
{
    set_config(test_malloc_func, test_calloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(binary_format_read_test),
        };

    return cmocka_run_group_tests(tests, setup, teardown);
}