#include "unit_testing.h"

#include "config.h"
#include "object.h"
#include "string_class.h"
#include "executor.h"

#define STACK_INITIAL_CAPACITY 8

typedef struct
{
    ConstantPool *constpool;
    InstructionStream *inststream;
} CMockaState;

void executor_new_test(void **state)
{
    ConstantPool *constpool = constantpool_new(0);
    InstructionStream *inststream = inststream_new(0);
    Executor *executor = executor_new(constpool, inststream);
    constantpool_free(constpool);
    inststream_free(inststream);
    executor_free(executor);
}

static int executor_with_main_method_setup(void **state)
{
    ConstantPool *constpool = constantpool_new(19);
    constantpool_add(constpool, 1, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "<Main>", .fields = 0, .methods = 1, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 2, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "<main>", ._class = 1, .address = 2, .args = 1, .locals = 1}});
    constantpool_add(constpool, 3, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "MyClass", .fields = 3, .methods = 0, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 4, (ConstantPoolEntry){.type = TYPE_FIELD, .data.field = {.name = "x", ._class = 3, .index = 0}});
    constantpool_add(constpool, 5, (ConstantPoolEntry){.type = TYPE_FIELD, .data.field = {.name = "y", ._class = 3, .index = 1}});
    constantpool_add(constpool, 6, (ConstantPoolEntry){.type = TYPE_FIELD, .data.field = {.name = "x", ._class = 3, .index = 2}});
    constantpool_add(constpool, 7, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Math", .fields = 0, .methods = 1, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 8, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "max", ._class = 7, .address = 30, .args = 3, .locals = 0}});
    constantpool_add(constpool, 9, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Factorial", .fields = 0, .methods = 1, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 10, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "fac", ._class = 9, .address = 30, .args = 2, .locals = 0}});

    constantpool_add(constpool, 11, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Animal", .fields = 0, .methods = 2, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 12, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "sound", ._class = 11, .address = 30, .args = 1, .locals = 0}});
    constantpool_add(constpool, 13, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "jump", ._class = 11, .address = 40, .args = 1, .locals = 0}});
    constantpool_add(constpool, 14, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Dog", .fields = 0, .methods = 3, .parent = 11, .vtable = NULL}});
    constantpool_add(constpool, 15, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "sound", ._class = 14, .address = 50, .args = 1, .locals = 0}});
    constantpool_add(constpool, 16, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Cat", .fields = 0, .methods = 3, .parent = 11, .vtable = NULL}});
    constantpool_add(constpool, 17, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "jump", ._class = 16, .address = 60, .args = 1, .locals = 0}});

    constantpool_add(constpool, 18, (ConstantPoolEntry){.type = TYPE_STRING, .data.string = {.value = "Hello!"}});
    constantpool_add(constpool, 19, (ConstantPoolEntry){.type = TYPE_STRING, .data.string = {.value = "Bye bye!"}});

    constantpool_compute_vtables(constpool);
    // Should be enough instruction space to perform all the tests we want.
    InstructionStream *inststream = inststream_new(100);
    inststream->instructions[0] = (Instruction){.opcode = NEW, .operand = 1};
    inststream->instructions[1] = (Instruction){.opcode = CALL, .operand = 2};
    CMockaState *cmocka_state = config._malloc(sizeof(CMockaState));
    cmocka_state->constpool = constpool;
    cmocka_state->inststream = inststream;
    *state = cmocka_state;
    return 0;
}

static int executor_with_main_method_teardown(void **state)
{
    CMockaState *cmocka_state = *state;
    constantpool_free(cmocka_state->constpool);
    inststream_free(cmocka_state->inststream);
    config._free(cmocka_state);
    return 0;
}

void executor_main_method_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_int_equal(0, executor->callstack->length);
    assert_int_equal(0, executor->evalstack->length);
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_int_equal(1, executor->callstack->length);
    assert_int_equal(0, executor->evalstack->length);
    assert_false(executor_step(executor)); // RETURN
    assert_int_equal(0, executor->callstack->length);
    assert_int_equal(0, executor->evalstack->length);
    object_free(main_obj);
    executor_free(executor);
}

void executor_push_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[3] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_push_and_pop_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[3] = (Instruction){.opcode = POP, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP
    assert_int_equal(0, executor->evalstack->length);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_arithmetic_test(void **state, int32_t first, int32_t second, uint8_t arithemtic_type, int32_t result)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH, .operand = first};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = second};
    instructions[4] = (Instruction){.opcode = arithemtic_type, .operand = 0};
    instructions[5] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH first
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(first, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH second
    assert_int_equal(2, executor->evalstack->length);
    assert_int_equal(second, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // arithemtic_type (ADD, SUB, MUL or DIV)
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(result, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_add_test(void **state)
{
    executor_arithmetic_test(state, 50, 25, ADD, 75);
}

void executor_sub_test(void **state)
{
    executor_arithmetic_test(state, 50, 25, SUB, 25);
}

void executor_sub_negative_result_test(void **state)
{
    executor_arithmetic_test(state, 50, 75, SUB, -25);
}

void executor_mul_test(void **state)
{
    executor_arithmetic_test(state, 50, 25, MUL, 1250);
}

void executor_div_test(void **state)
{
    executor_arithmetic_test(state, 50, 25, DIV, 2);
}

void executor_jump_skip_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = JUMP, .operand = 4};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[4] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[5] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // JUMP 4
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(25, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_jump_no_effect_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = JUMP, .operand = 3};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[4] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[5] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // JUMP 3
    assert_true(executor_step(executor)); // PUSH 50
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(50, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(2, executor->evalstack->length);
    assert_int_equal(25, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_cond_jump_true_test(void **state, int32_t first, int32_t second, uint8_t jump_type)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH, .operand = first};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = second};
    instructions[4] = (Instruction){.opcode = jump_type, .operand = 7};
    instructions[5] = (Instruction){.opcode = PUSH, .operand = 0};
    instructions[6] = (Instruction){.opcode = JUMP, .operand = 8};
    instructions[7] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[8] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor));  // CALL <main>
    assert_true(executor_step(executor));  // PUSH first
    assert_true(executor_step(executor));  // PUSH second
    assert_true(executor_step(executor));  // JUMP_XX 7
    assert_true(executor_step(executor));  // PUSH 1
    assert_false(executor_step(executor)); // RETURN
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(1, evalstack_top(executor->evalstack).integer);
    object_free(main_obj);
    executor_free(executor);
}

void executor_cond_jump_false_test(void **state, int32_t first, int32_t second, uint8_t jump_type)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH, .operand = first};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = second};
    instructions[4] = (Instruction){.opcode = jump_type, .operand = 7};
    instructions[5] = (Instruction){.opcode = PUSH, .operand = 0};
    instructions[6] = (Instruction){.opcode = JUMP, .operand = 8};
    instructions[7] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[8] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor));  // CALL <main>
    assert_true(executor_step(executor));  // PUSH first
    assert_true(executor_step(executor));  // PUSH second
    assert_true(executor_step(executor));  // JUMP_XX 7
    assert_true(executor_step(executor));  // PUSH 0
    assert_true(executor_step(executor));  // JUMP 8
    assert_false(executor_step(executor)); // RETURN
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(0, evalstack_top(executor->evalstack).integer);
    object_free(main_obj);
    executor_free(executor);
}

void executor_jump_eq_true_test(void **state)
{
    executor_cond_jump_true_test(state, 50, 50, JUMP_EQ);
}

void executor_jump_eq_false_test(void **state)
{
    executor_cond_jump_false_test(state, 50, 51, JUMP_EQ);
}

void executor_jump_ne_true_test(void **state)
{
    executor_cond_jump_true_test(state, 50, 51, JUMP_NE);
}

void executor_jump_ne_false_test(void **state)
{
    executor_cond_jump_false_test(state, 50, 50, JUMP_NE);
}

void executor_jump_lt_true_test(void **state)
{
    executor_cond_jump_true_test(state, 50, 51, JUMP_LT);
}

void executor_jump_lt_false_eq_test(void **state)
{
    executor_cond_jump_false_test(state, 50, 50, JUMP_LT);
}

void executor_jump_lt_false_gt_test(void **state)
{
    executor_cond_jump_false_test(state, 51, 50, JUMP_LT);
}

void executor_jump_le_true_eq_test(void **state)
{
    executor_cond_jump_true_test(state, 50, 50, JUMP_LE);
}

void executor_jump_le_true_lt_test(void **state)
{
    executor_cond_jump_true_test(state, 50, 51, JUMP_LE);
}

void executor_jump_le_false_gt_test(void **state)
{
    executor_cond_jump_false_test(state, 51, 50, JUMP_LE);
}

void executor_jump_gt_true_test(void **state)
{
    executor_cond_jump_true_test(state, 51, 50, JUMP_GT);
}

void executor_jump_gt_false_eq_test(void **state)
{
    executor_cond_jump_false_test(state, 50, 50, JUMP_GT);
}

void executor_jump_gt_false_lt_test(void **state)
{
    executor_cond_jump_false_test(state, 50, 51, JUMP_GT);
}

void executor_jump_ge_true_eq_test(void **state)
{
    executor_cond_jump_true_test(state, 50, 50, JUMP_GE);
}

void executor_jump_ge_true_gt_test(void **state)
{
    executor_cond_jump_true_test(state, 51, 50, JUMP_GE);
}

void executor_jump_ge_false_lt_test(void **state)
{
    executor_cond_jump_false_test(state, 50, 51, JUMP_GE);
}

void executor_dup_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[3] = (Instruction){.opcode = DUP, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // DUP
    assert_int_equal(2, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_new_object_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = 3};
    instructions[3] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 2
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(3, *(uint32_t *)evalstack_top(executor->evalstack).pointer);
    // Free the allocated object.
    config._free(evalstack_top(executor->evalstack).pointer);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_push_and_pop_fields_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = 3};
    for (int i = 3; i < 8; i++)
    {
        instructions[i] = (Instruction){.opcode = DUP, .operand = 0};
    }
    instructions[8] = (Instruction){.opcode = PUSH, .operand = 100};
    instructions[9] = (Instruction){.opcode = POP_FIELD, .operand = 4};
    instructions[10] = (Instruction){.opcode = PUSH, .operand = 200};
    instructions[11] = (Instruction){.opcode = POP_FIELD, .operand = 5};
    instructions[12] = (Instruction){.opcode = PUSH, .operand = 300};
    instructions[13] = (Instruction){.opcode = POP_FIELD, .operand = 6};
    instructions[14] = (Instruction){.opcode = PUSH_FIELD, .operand = 4};
    instructions[15] = (Instruction){.opcode = POP, .operand = 0};
    instructions[16] = (Instruction){.opcode = PUSH_FIELD, .operand = 5};
    instructions[17] = (Instruction){.opcode = POP, .operand = 0};
    instructions[18] = (Instruction){.opcode = PUSH_FIELD, .operand = 6};
    instructions[19] = (Instruction){.opcode = POP, .operand = 0};
    instructions[20] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 3
    void *object = evalstack_top(executor->evalstack).pointer;

    for (int i = 3; i < 8; i++)
    {
        assert_true(executor_step(executor)); // DUP
    }

    assert_true(executor_step(executor)); // PUSH 100
    assert_true(executor_step(executor)); // POP_FIELD 4
    assert_true(executor_step(executor)); // PUSH 200
    assert_true(executor_step(executor)); // POP_FIELD 5
    assert_true(executor_step(executor)); // PUSH 300
    assert_true(executor_step(executor)); // POP_FIELD 6

    assert_true(executor_step(executor)); // PUSH_FIELD 4
    assert_int_equal(100, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP

    assert_true(executor_step(executor)); // PUSH_FIELD 5
    assert_int_equal(200, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP

    assert_true(executor_step(executor)); // PUSH_FIELD 6
    assert_int_equal(300, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP

    assert_int_equal(0, executor->evalstack->length);

    // Free the allocated object.
    object_free(object);

    assert_false(executor_step(executor)); // RETURN

    object_free(main_obj);

    executor_free(executor);
}

void executor_push_and_pop_var_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 100};
    instructions[3] = (Instruction){.opcode = POP_VAR, .operand = 0};
    instructions[4] = (Instruction){.opcode = PUSH_VAR, .operand = 0};
    instructions[5] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 100
    assert_true(executor_step(executor)); // POP_VAR 0
    assert_int_equal(0, executor->evalstack->length);
    assert_int_equal(100, callstack_top(executor->callstack).vars[0].integer);
    assert_true(executor_step(executor)); // PUSH_VAR 0
    assert_int_equal(100, evalstack_top(executor->evalstack).integer);
    assert_int_equal(100, callstack_top(executor->callstack).vars[0].integer);
    assert_false(executor_step(executor)); // RETURN
    object_free(main_obj);
    executor_free(executor);
}

void executor_call_max_test(void **state, int32_t a, int32_t b)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    // <main> function.
    instructions[2] = (Instruction){.opcode = NEW, .operand = 7};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = a};
    instructions[4] = (Instruction){.opcode = PUSH, .operand = b};
    instructions[5] = (Instruction){.opcode = CALL, .operand = 8};
    instructions[6] = (Instruction){.opcode = RETURN, .operand = 0};

    // Math.max(a, b) function.
    instructions[30] = (Instruction){.opcode = PUSH_VAR, .operand = 1};
    instructions[31] = (Instruction){.opcode = PUSH_VAR, .operand = 2};
    instructions[32] = (Instruction){.opcode = JUMP_GT, .operand = 35};
    instructions[33] = (Instruction){.opcode = PUSH_VAR, .operand = 2};
    instructions[34] = (Instruction){.opcode = RETURN, .operand = 0};
    instructions[35] = (Instruction){.opcode = PUSH_VAR, .operand = 1};
    instructions[36] = (Instruction){.opcode = RETURN, .operand = 0};

    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 7
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH a
    assert_true(executor_step(executor)); // PUSH b
    assert_true(executor_step(executor)); // CALL 8

    assert_true(executor_step(executor)); // PUSH_VAR 1
    assert_int_equal(a, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH_VAR 2
    assert_int_equal(b, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // JUMP_GT 35
    assert_true(executor_step(executor)); // PUSH_VAR max(a, b)
    assert_true(executor_step(executor)); // RETURN

    assert_int_equal(a > b ? a : b, evalstack_top(executor->evalstack).integer);

    // Free the allocated object.
    object_free(object);
    assert_false(executor_step(executor)); // RETURN

    object_free(main_obj);
    executor_free(executor);
}

void executor_call_max_lt_test(void **state)
{
    executor_call_max_test(state, 10, 20);
}

void executor_call_max_eq_test(void **state)
{
    executor_call_max_test(state, 20, 20);
}

void executor_call_max_gt_test(void **state)
{
    executor_call_max_test(state, 20, 10);
}

void executor_call_fac_test(void **state, int32_t n, int32_t fac_of_n)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    // <main> function.
    instructions[2] = (Instruction){.opcode = NEW, .operand = 9};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = n};
    instructions[4] = (Instruction){.opcode = CALL, .operand = 10};
    instructions[5] = (Instruction){.opcode = RETURN, .operand = 0};

    // Factorial.fac(n) function.
    instructions[30] = (Instruction){.opcode = PUSH_VAR, .operand = 1};
    instructions[31] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[32] = (Instruction){.opcode = JUMP_GT, .operand = 35};

    // if n <= 1
    instructions[33] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[34] = (Instruction){.opcode = RETURN, .operand = 0};

    // if n > 1
    instructions[35] = (Instruction){.opcode = PUSH_VAR, .operand = 1};
    instructions[36] = (Instruction){.opcode = PUSH_VAR, .operand = 0};
    instructions[37] = (Instruction){.opcode = PUSH_VAR, .operand = 1};
    instructions[38] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[39] = (Instruction){.opcode = SUB, .operand = 0};
    instructions[40] = (Instruction){.opcode = CALL, .operand = 10};
    instructions[41] = (Instruction){.opcode = MUL, .operand = 0};
    instructions[42] = (Instruction){.opcode = RETURN, .operand = 0};

    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 9
    void *object = evalstack_top(executor->evalstack).pointer;

    executor_step_all(executor);

    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(fac_of_n, evalstack_top(executor->evalstack).integer);

    // Free the allocated object.
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_call_fac_0_test(void **state)
{
    executor_call_fac_test(state, 0, 1);
}

void executor_call_fac_1_test(void **state)
{
    executor_call_fac_test(state, 1, 1);
}

void executor_call_fac_2_test(void **state)
{
    executor_call_fac_test(state, 2, 2);
}

void executor_call_fac_3_test(void **state)
{
    executor_call_fac_test(state, 3, 6);
}

void executor_call_fac_4_test(void **state)
{
    executor_call_fac_test(state, 4, 24);
}

void executor_call_fac_5_test(void **state)
{
    executor_call_fac_test(state, 5, 120);
}

void executor_call_fac_6_test(void **state)
{
    executor_call_fac_test(state, 6, 720);
}

void executor_call_fac_7_test(void **state)
{
    executor_call_fac_test(state, 7, 5040);
}

void executor_call_fac_8_test(void **state)
{
    executor_call_fac_test(state, 8, 40320);
}

void executor_polymorphism_test(void **state, uint32_t class_type, uint32_t sound_addr, uint32_t jump_addr)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = class_type};
    instructions[3] = (Instruction){.opcode = DUP, .operand = 0};
    instructions[4] = (Instruction){.opcode = CALL, .operand = 12};
    instructions[5] = (Instruction){.opcode = CALL, .operand = 13};
    instructions[6] = (Instruction){.opcode = RETURN, .operand = 0};
    instructions[30] = (Instruction){.opcode = RETURN, .operand = 0};
    instructions[40] = (Instruction){.opcode = RETURN, .operand = 0};
    instructions[50] = (Instruction){.opcode = RETURN, .operand = 0};
    instructions[60] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW class_type (Animal, Dog or Cat)
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // DUP
    assert_true(executor_step(executor)); // Call Animal.sound()
    assert_int_equal(executor->inststream->current, sound_addr);
    assert_true(executor_step(executor)); // RETURN
    assert_true(executor_step(executor)); // Call Animal.jump()
    assert_int_equal(executor->inststream->current, jump_addr);
    assert_true(executor_step(executor));  // RETURN
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_polymorphism_animal_test(void **state)
{
    executor_polymorphism_test(state, 11, 30, 40);
}

void executor_polymorphism_dog_test(void **state)
{
    executor_polymorphism_test(state, 14, 50, 40);
}

void executor_polymorphism_cat_test(void **state)
{
    executor_polymorphism_test(state, 16, 30, 60);
}

void executor_string_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = PUSH_STRING, .operand = 18};
    instructions[3] = (Instruction){.opcode = PUSH_STRING, .operand = 19};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH_STRING 18
    void *string_object = evalstack_top(executor->evalstack).pointer;
    assert_string_equal("Hello!", string_get_value(string_object));
    assert_true(executor_step(executor)); // PUSH_STRING 19
    void *string_object_2 = evalstack_top(executor->evalstack).pointer;
    assert_string_equal("Bye bye!", string_get_value(string_object_2));
    assert_false(executor_step(executor)); // RETURN
    object_free(string_object);
    object_free(string_object_2);
    object_free(main_obj);
    executor_free(executor);
}

void executor_new_console_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_CONSOLE};
    instructions[3] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW Console
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_int_equal(CONSTPOOL_CLASS_CONSOLE, object_get_class(object));
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_call_console_println_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_CONSOLE};
    instructions[3] = (Instruction){.opcode = PUSH_STRING, .operand = 18};
    instructions[4] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_CONSOLE_PRINTLN};
    instructions[5] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[6] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW Console
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH_STRING 18
    void *string_object = evalstack_top(executor->evalstack).pointer;
    assert_int_equal(2, executor->evalstack->length);
    assert_true(executor_step(executor)); // CALL Console.println()
    assert_int_equal(0, executor->evalstack->length);
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack->length);
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(string_object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_new_string_builder_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_int_equal(CONSTPOOL_CLASS_STRING_BUILDER, object_get_class(object));
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_string_builder_to_string_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL StringBuilder.toString()
    void *string_object = evalstack_top(executor->evalstack).pointer;
    assert_int_equal(1, executor->evalstack->length);
    assert_string_equal("", string_get_value(string_object));
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_string_builder_append_string_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = PUSH_STRING, .operand = 18};
    instructions[4] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_STRING};
    instructions[5] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING};
    instructions[6] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH_STRING 18
    void *string_object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendString()
    assert_int_equal(1, executor->evalstack->length);
    assert_true(executor_step(executor)); // CALL Stringbuilder.toString()
    assert_string_equal("Hello!", string_get_value(evalstack_top(executor->evalstack).pointer));
    assert_false(executor_step(executor)); // RETURN
    object_free(string_object);
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_string_builder_append_string_twice_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = PUSH_STRING, .operand = 18};
    instructions[4] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_STRING};
    instructions[5] = (Instruction){.opcode = PUSH_STRING, .operand = 19};
    instructions[6] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_STRING};
    instructions[7] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING};
    instructions[8] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH_STRING 18
    void *string_object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendString()
    assert_true(executor_step(executor)); // PUSH_STRING 19
    void *string_object_2 = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendString()
    assert_true(executor_step(executor)); // CALL Stringbuilder.toString()
    assert_string_equal("Hello!Bye bye!", string_get_value(evalstack_top(executor->evalstack).pointer));
    assert_false(executor_step(executor)); // RETURN
    object_free(string_object);
    object_free(string_object_2);
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_string_builder_append_bool_test(void **state, bool bool_value, const char *result)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = bool_value ? 1 : 0};
    instructions[4] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_BOOL};
    instructions[5] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING};
    instructions[6] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH bool_value
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendBool()
    assert_int_equal(1, executor->evalstack->length);
    assert_true(executor_step(executor)); // CALL Stringbuilder.toString()
    assert_string_equal(result, string_get_value(evalstack_top(executor->evalstack).pointer));
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_string_builder_append_bool_twice_test(void **state, bool first_bool, bool second_bool, const char *result)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = first_bool ? 1 : 0};
    instructions[4] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_BOOL};
    instructions[5] = (Instruction){.opcode = PUSH, .operand = second_bool ? 1 : 0};
    instructions[6] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_BOOL};
    instructions[7] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING};
    instructions[8] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH first_bool
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendBool()
    assert_int_equal(1, executor->evalstack->length);
    assert_true(executor_step(executor)); // PUSH second_bool
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendBool()
    assert_int_equal(1, executor->evalstack->length);
    assert_true(executor_step(executor)); // CALL Stringbuilder.toString()
    assert_string_equal(result, string_get_value(evalstack_top(executor->evalstack).pointer));
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_string_builder_append_bool_true_test(void **state)
{
    executor_string_builder_append_bool_test(state, true, "true");
}

void executor_string_builder_append_bool_false_test(void **state)
{
    executor_string_builder_append_bool_test(state, false, "false");
}

void executor_string_builder_append_bool_false_false_test(void **state)
{
    executor_string_builder_append_bool_twice_test(state, false, false, "falsefalse");
}

void executor_string_builder_append_bool_false_true_test(void **state)
{
    executor_string_builder_append_bool_twice_test(state, false, true, "falsetrue");
}

void executor_string_builder_append_bool_true_false_test(void **state)
{
    executor_string_builder_append_bool_twice_test(state, true, false, "truefalse");
}

void executor_string_builder_append_bool_true_true_test(void **state)
{
    executor_string_builder_append_bool_twice_test(state, true, true, "truetrue");
}

void executor_string_builder_append_int_test(void **state, int value, const char *result)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = value};
    instructions[4] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_INT};
    instructions[5] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING};
    instructions[6] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH value
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendInt()
    assert_int_equal(1, executor->evalstack->length);
    assert_true(executor_step(executor)); // CALL Stringbuilder.toString()
    assert_string_equal(result, string_get_value(evalstack_top(executor->evalstack).pointer));
    assert_false(executor_step(executor)); // RETURN
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

void executor_string_builder_append_int_0_test(void **state)
{
    executor_string_builder_append_int_test(state, 0, "0");
}

void executor_string_builder_append_int_1_test(void **state)
{
    executor_string_builder_append_int_test(state, 1, "1");
}

void executor_string_builder_append_int_123456789_test(void **state)
{
    executor_string_builder_append_int_test(state, 123456789, "123456789");
}

void executor_string_builder_append_int_minus_123456789_test(void **state)
{
    executor_string_builder_append_int_test(state, -123456789, "-123456789");
}

void executor_string_builder_append_string_bool_int_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[2] = (Instruction){.opcode = NEW, .operand = CONSTPOOL_CLASS_STRING_BUILDER};
    instructions[3] = (Instruction){.opcode = PUSH_STRING, .operand = 18};
    instructions[4] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_STRING};
    instructions[5] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[6] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_BOOL};
    instructions[7] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[8] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_APPEND_INT};
    instructions[9] = (Instruction){.opcode = CALL, .operand = CONSTPOOL_METHOD_STRING_BUILDER_TO_STRING};
    instructions[10] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // NEW <Main>
    void *main_obj = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW StringBuilder
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH_STRING 18
    void *string_object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendString()
    assert_true(executor_step(executor)); // PUSH 1
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendBool()
    assert_true(executor_step(executor)); // PUSH 123
    assert_true(executor_step(executor)); // CALL Stringbuilder.appendInt()
    assert_true(executor_step(executor)); // CALL Stringbuilder.toString()
    assert_string_equal("Hello!true123", string_get_value(evalstack_top(executor->evalstack).pointer));
    assert_false(executor_step(executor)); // RETURN
    object_free(string_object);
    object_free(object);
    object_free(main_obj);
    executor_free(executor);
}

int main()
{
    set_config(test_malloc_func, test_calloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(executor_new_test),
            cmocka_unit_test_setup_teardown(executor_main_method_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_push_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_push_and_pop_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_add_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_sub_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_sub_negative_result_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_mul_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_div_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_skip_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_no_effect_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_eq_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_eq_false_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_ne_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_ne_false_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_lt_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_lt_false_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_lt_false_gt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_le_true_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_le_true_lt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_le_false_gt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_gt_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_gt_false_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_gt_false_lt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_ge_true_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_ge_true_gt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_ge_false_lt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_dup_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_new_object_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_push_and_pop_fields_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_push_and_pop_var_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_max_lt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_max_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_max_gt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_0_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_1_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_2_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_3_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_4_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_5_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_6_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_7_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_fac_8_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_polymorphism_animal_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_polymorphism_dog_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_polymorphism_cat_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_new_console_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_call_console_println_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_new_string_builder_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_to_string_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_string_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_string_twice_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_bool_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_bool_false_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_bool_false_false_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_bool_false_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_bool_true_false_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_bool_true_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_int_0_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_int_1_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_int_123456789_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_int_minus_123456789_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_string_builder_append_string_bool_int_test, executor_with_main_method_setup, executor_with_main_method_teardown),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}