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

void executor_add_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[3] = (Instruction){.opcode = ADD, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 50
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(50, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(2, executor->evalstack.length);
    assert_int_equal(25, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // ADD
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(75, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
}

void executor_sub_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[3] = (Instruction){.opcode = SUB, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 50
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(50, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(2, executor->evalstack.length);
    assert_int_equal(25, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // SUB
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(25, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
}

void executor_mul_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[3] = (Instruction){.opcode = MUL, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 50
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(50, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(2, executor->evalstack.length);
    assert_int_equal(25, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // MUL
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(1250, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
}

void executor_div_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 25};
    instructions[3] = (Instruction){.opcode = DIV, .operand = 0};
    instructions[4] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 50
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(50, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // PUSH 25
    assert_int_equal(2, executor->evalstack.length);
    assert_int_equal(25, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor)); // DIV
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(2, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
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

void executor_jump_eq_true_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[3] = (Instruction){.opcode = JUMP_EQ, .operand = 6};
    instructions[4] = (Instruction){.opcode = PUSH, .operand = 0};
    instructions[5] = (Instruction){.opcode = JUMP, .operand = 7};
    instructions[6] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[7] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 50
    assert_true(executor_step(executor)); // PUSH 50
    assert_true(executor_step(executor)); // JUMP_EQ 6
    assert_true(executor_step(executor)); // PUSH 1
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(1, evalstack_top(&executor->evalstack).integer);
    assert_false(executor_step(executor)); // RETURN
}

void executor_jump_eq_false_test(void **state)
{
    Executor *executor = *state;
    Instruction *instructions = executor->stream.instructions;
    instructions[1] = (Instruction){.opcode = PUSH, .operand = 50};
    instructions[2] = (Instruction){.opcode = PUSH, .operand = 51};
    instructions[3] = (Instruction){.opcode = JUMP_EQ, .operand = 6};
    instructions[4] = (Instruction){.opcode = PUSH, .operand = 0};
    instructions[5] = (Instruction){.opcode = JUMP, .operand = 7};
    instructions[6] = (Instruction){.opcode = PUSH, .operand = 1};
    instructions[7] = (Instruction){.opcode = RETURN, .operand = 0};
    assert_true(executor_step(executor)); // CALL <main>
    assert_true(executor_step(executor)); // PUSH 50
    assert_true(executor_step(executor)); // PUSH 51
    assert_true(executor_step(executor)); // JUMP_EQ 6
    assert_true(executor_step(executor)); // PUSH 0
    assert_int_equal(1, executor->evalstack.length);
    assert_int_equal(0, evalstack_top(&executor->evalstack).integer);
    assert_true(executor_step(executor));  // JUMP 7
    assert_false(executor_step(executor)); // RETURN
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
            cmocka_unit_test_setup_teardown(executor_mul_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_div_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_skip_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_no_effect_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_eq_true_test, executor_with_main_method_setup, executor_with_main_method_teardown),
            cmocka_unit_test_setup_teardown(executor_jump_eq_false_test, executor_with_main_method_setup, executor_with_main_method_teardown),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}