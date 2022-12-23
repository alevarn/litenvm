#include "unit_testing.h"

#include "config.h"
#include "executor.h"

#define STACK_INITIAL_CAPACITY 8

void executor_new_test(void **state)
{
    ConstantPool constpool = constantpool_new(0);
    InstructionStream stream = {.current = 0, .length = 0, .instructions = NULL};
    Executor executor = executor_new(constpool, stream);
    constantpool_free(&constpool);
    executor_free(&executor);
}

static int executor_with_main_method_setup(void **state)
{
    ConstantPool constpool = constantpool_new(1);
    constantpool_add(&constpool, 1, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.name = "<main>", ._class = 0, .address = 1, .args = 0, .locals = 0}});
    // Should be enough instruction space to perform all the tests we want in just a single main method without args/locals.
    uint32_t instructions_length = 100;
    Instruction *instructions = config._malloc(instructions_length * sizeof(Instruction));
    instructions[0] = (Instruction){.opcode = CALL, .operand = 1};
    InstructionStream stream = {.current = 0, .length = instructions_length, .instructions = instructions};
    Executor *executor = config._malloc(sizeof(Executor));
    *executor = executor_new(constpool, stream);
    *state = executor;
    return 0;
}

static int executor_with_main_method_teardown(void **state)
{
    Executor *executor = *state;
    config._free(executor->stream.instructions);
    constantpool_free(&executor->constpool);
    executor_free(executor);
    config._free(executor);
    return 0;
}

void executor_main_method_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_int_equal(0, executor->callstack.length);
    assert_int_equal(0, executor->evalstack.length);
    assert_true(executor_step(executor)); // CALL <main>
    assert_int_equal(1, executor->callstack.length);
    assert_int_equal(0, executor->evalstack.length);
    assert_false(executor_step(executor)); // RETURN
    assert_int_equal(0, executor->callstack.length);
    assert_int_equal(0, executor->evalstack.length);
}

void executor_push_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 123};
    instructions[2] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 123
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(123, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
}

void executor_arithmetic_test(void **state, int32_t first, int32_t second, uint8_t arithemtic_type, int32_t result)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = first};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = second};
    instructions[3] = (Instruction){.opcode = arithemtic_type, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH first
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(first, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH second
    assert_int_equal(2, executor->evalstack.length);
    assert_int_equal(second, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // arithemtic_type (ADD, SUB, MUL or DIV)
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(result, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
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
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = JUMP, .operand = 3};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // JUMP 3
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(25, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
}

void executor_jump_no_effect_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = JUMP, .operand = 2};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[3] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // JUMP 2
    assert_true(executor_step(executor)); // PUSH 50
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(50, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(2, executor->evalstack.length);
    assert_int_equal(25, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
}

void executor_cond_jump_true_test(void **state, int32_t first, int32_t second, uint8_t jump_type)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
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
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(1, evalstack_top(&executor->evalstack).integer);
}

void executor_cond_jump_false_test(void **state, int32_t first, int32_t second, uint8_t jump_type)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
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
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(0, evalstack_top(&executor->evalstack).integer);
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

int main()
{
    set_config(test_malloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(executor_new_test),
            cmocka_unit_test_setup_teardown(executor_main_method_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_push_test, executor_with_main_method_setup, executor_with_main_method_teardown),
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
            cmocka_unit_test_setup_teardown(executor_jump_lt_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_lt_false_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_lt_false_gt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_le_true_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_le_true_lt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_le_false_gt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_gt_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_gt_false_eq_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_gt_false_lt_test, executor_with_main_method_setup, executor_with_main_method_teardown),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}