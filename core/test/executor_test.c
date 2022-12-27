#include "unit_testing.h"

#include "config.h"
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
    ConstantPool *constpool = constantpool_new(9);
    constantpool_add(constpool, 1, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "<main>", ._class = 0, .address = 1, .args = 0, .locals = 1}});
    constantpool_add(constpool, 2, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "MyClass", .fields = 3, .methods = 0, .parent = 0}});
    constantpool_add(constpool, 3, (ConstantPoolEntry){.type = TYPE_FIELD, .data.field = {.name = "x", ._class = 2, .index = 0}});
    constantpool_add(constpool, 4, (ConstantPoolEntry){.type = TYPE_FIELD, .data.field = {.name = "y", ._class = 2, .index = 1}});
    constantpool_add(constpool, 5, (ConstantPoolEntry){.type = TYPE_FIELD, .data.field = {.name = "x", ._class = 2, .index = 2}});
    constantpool_add(constpool, 6, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Math", .fields = 0, .methods = 1, .parent = 0}});
    constantpool_add(constpool, 7, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "max", ._class = 6, .address = 30, .args = 3, .locals = 0}});
    constantpool_add(constpool, 8, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Factorial", .fields = 0, .methods = 1, .parent = 0}});
    constantpool_add(constpool, 9, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "fac", ._class = 8, .address = 30, .args = 2, .locals = 0}});
    // Should be enough instruction space to perform all the tests we want.
    InstructionStream *inststream = inststream_new(100);
    inststream->instructions[0] = (Instruction) {.opcode = CALL, .operand = 1};
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
    instructions[1] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_int_equal(0, executor->callstack->length);
    assert_int_equal(0, executor->evalstack->length);
    assert_true(executor_step(executor)); // CALL <main>
    assert_int_equal(1, executor->callstack->length);
    assert_int_equal(0, executor->evalstack->length);
    assert_false(executor_step(executor)); // RETURN
    assert_int_equal(0, executor->callstack->length);
    assert_int_equal(0, executor->evalstack->length);
    executor_free(executor);
}

void executor_push_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[2] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_push_and_pop_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[2] = (Instruction){.opcode = POP, .operand = 0};
    instructions[3] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP
    assert_int_equal(0, executor->evalstack->length);
    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_arithmetic_test(void **state, int32_t first, int32_t second, uint8_t arithemtic_type, int32_t result)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = first};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = second};
    instructions[3] = (Instruction){.opcode = arithemtic_type, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
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
    instructions[1] = (Instruction){.opcode = JUMP, .operand = 3};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // JUMP 3
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(25, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_jump_no_effect_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = JUMP, .operand = 2};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // JUMP 2
    assert_true(executor_step(executor)); // PUSH 50
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(50, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(2, executor->evalstack->length);
    assert_int_equal(25, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_cond_jump_true_test(void **state, int32_t first, int32_t second, uint8_t jump_type)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = first};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = second};
    instructions[3] = (Instruction){.opcode = jump_type, .operand = 6};
    instructions[4] = (Instruction){.opcode = PUSH, .operand = 0};
    instructions[5] = (Instruction){.opcode = JUMP, .operand = 7};
    instructions[6] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[7] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor));  // CALL <main>
    assert_true(executor_step(executor));  // PUSH first
    assert_true(executor_step(executor));  // PUSH second
    assert_true(executor_step(executor));  // JUMP_EQ 6
    assert_true(executor_step(executor));  // PUSH 1
    assert_false(executor_step(executor)); // RETURN
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(1, evalstack_top(executor->evalstack).integer);
    executor_free(executor);
}

void executor_cond_jump_false_test(void **state, int32_t first, int32_t second, uint8_t jump_type)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = first};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = second};
    instructions[3] = (Instruction){.opcode = jump_type, .operand = 6};
    instructions[4] = (Instruction){.opcode = PUSH, .operand = 0};
    instructions[5] = (Instruction){.opcode = JUMP, .operand = 7};
    instructions[6] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[7] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor));  // CALL <main>
    assert_true(executor_step(executor));  // PUSH first
    assert_true(executor_step(executor));  // PUSH second
    assert_true(executor_step(executor));  // JUMP_EQ 6
    assert_true(executor_step(executor));  // PUSH 0
    assert_true(executor_step(executor));  // JUMP 7
    assert_false(executor_step(executor)); // RETURN
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(0, evalstack_top(executor->evalstack).integer);
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
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[2] = (Instruction){.opcode = DUP, .operand = 0};
    instructions[3] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // DUP
    assert_int_equal(2, executor->evalstack->length);
    assert_int_equal(123, evalstack_top(executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_new_object_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = NEW, .operand = 2};
    instructions[2] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 2
    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(2, *(uint32_t *)evalstack_top(executor->evalstack).pointer);
    // Free the allocated object.
    config._free(evalstack_top(executor->evalstack).pointer);
    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_push_and_pop_fields_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = NEW, .operand = 2};
    for (int i = 2; i < 7; i++)
    {
        instructions[i] = (Instruction){.opcode = DUP, .operand = 0};
    }
    instructions[7] = (Instruction){.opcode = PUSH, .operand = 100};
    instructions[8] = (Instruction){.opcode = POP_FIELD, .operand = 3};
    instructions[9] = (Instruction){.opcode = PUSH, .operand = 200};
    instructions[10] = (Instruction){.opcode = POP_FIELD, .operand = 4};
    instructions[11] = (Instruction){.opcode = PUSH, .operand = 300};
    instructions[12] = (Instruction){.opcode = POP_FIELD, .operand = 5};
    instructions[13] = (Instruction){.opcode = PUSH_FIELD, .operand = 3};
    instructions[14] = (Instruction){.opcode = POP, .operand = 0};
    instructions[15] = (Instruction){.opcode = PUSH_FIELD, .operand = 4};
    instructions[16] = (Instruction){.opcode = POP, .operand = 0};
    instructions[17] = (Instruction){.opcode = PUSH_FIELD, .operand = 5};
    instructions[18] = (Instruction){.opcode = POP, .operand = 0};
    instructions[19] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 2
    void *object = evalstack_top(executor->evalstack).pointer;

    for (int i = 2; i < 7; i++)
    {
        assert_true(executor_step(executor)); // DUP
    }

    assert_true(executor_step(executor)); // PUSH 100
    assert_true(executor_step(executor)); // POP_FIELD 3
    assert_true(executor_step(executor)); // PUSH 200
    assert_true(executor_step(executor)); // POP_FIELD 4
    assert_true(executor_step(executor)); // PUSH 300
    assert_true(executor_step(executor)); // POP_FIELD 5

    assert_true(executor_step(executor)); // PUSH_FIELD 3
    assert_int_equal(100, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP

    assert_true(executor_step(executor)); // PUSH_FIELD 4
    assert_int_equal(200, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP

    assert_true(executor_step(executor)); // PUSH_FIELD 5
    assert_int_equal(300, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // POP

    assert_int_equal(0, executor->evalstack->length);

    // Free the allocated object.
    config._free(object);

    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_push_and_pop_var_test(void **state)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 100};
    instructions[2] = (Instruction){.opcode = POP_VAR, .operand = 0};
    instructions[3] = (Instruction){.opcode = PUSH_VAR, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 100
    assert_true(executor_step(executor)); // POP_VAR 0
    assert_int_equal(0, executor->evalstack->length);
    assert_int_equal(100, callstack_top(executor->callstack).vars[0].integer);
    assert_true(executor_step(executor)); // PUSH_VAR 0
    assert_int_equal(100, evalstack_top(executor->evalstack).integer);
    assert_int_equal(100, callstack_top(executor->callstack).vars[0].integer);
    assert_false(executor_step(executor)); // RETURN
    executor_free(executor);
}

void executor_call_max_test(void **state, int32_t a, int32_t b)
{
    CMockaState *cmocka_state = *state;
    Executor *executor = executor_new(cmocka_state->constpool, cmocka_state->inststream);
    Instruction *instructions = executor->inststream->instructions;
    // <main> function.
    instructions[1] = (Instruction){.opcode = NEW, .operand = 6};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = a};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = b};
    instructions[4] = (Instruction){.opcode = CALL, .operand = 7};
    instructions[5] = (Instruction){.opcode = RETURN, .operand = 0};

    // Math.max(a, b) function.
    instructions[30] = (Instruction){.opcode = PUSH_VAR, .operand = 1};
    instructions[31] = (Instruction){.opcode = PUSH_VAR, .operand = 2};
    instructions[32] = (Instruction){.opcode = JUMP_GT, .operand = 35};
    instructions[33] = (Instruction){.opcode = PUSH_VAR, .operand = 2};
    instructions[34] = (Instruction){.opcode = RETURN, .operand = 0};
    instructions[35] = (Instruction){.opcode = PUSH_VAR, .operand = 1};
    instructions[36] = (Instruction){.opcode = RETURN, .operand = 0};

    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 6
    void *object = evalstack_top(executor->evalstack).pointer;
    assert_true(executor_step(executor)); // PUSH a
    assert_true(executor_step(executor)); // PUSH b
    assert_true(executor_step(executor)); // CALL 7

    assert_true(executor_step(executor)); // PUSH_VAR 1
    assert_int_equal(a, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH_VAR 2
    assert_int_equal(b, evalstack_top(executor->evalstack).integer);
    assert_true(executor_step(executor)); // JUMP_GT 35
    assert_true(executor_step(executor)); // PUSH_VAR max(a, b)
    assert_true(executor_step(executor)); // RETURN

    assert_int_equal(a > b ? a : b, evalstack_top(executor->evalstack).integer);

    // Free the allocated object.
    config._free(object);
    assert_false(executor_step(executor)); // RETURN
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
    instructions[1] = (Instruction){.opcode = NEW, .operand = 8};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = n};
    instructions[3] = (Instruction){.opcode = CALL, .operand = 9};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};

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
    instructions[40] = (Instruction){.opcode = CALL, .operand = 9};
    instructions[41] = (Instruction){.opcode = MUL, .operand = 0};
    instructions[42] = (Instruction){.opcode = RETURN, .operand = 0};

    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // NEW 8
    void *object = evalstack_top(executor->evalstack).pointer;

    executor_step_all(executor);

    assert_int_equal(1, executor->evalstack->length);
    assert_int_equal(fac_of_n, evalstack_top(executor->evalstack).integer);

    // Free the allocated object.
    config._free(object);

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

int main()
{
    set_config(test_malloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

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
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}