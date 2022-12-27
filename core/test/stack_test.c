#include "unit_testing.h"

#include "config.h"
#include "stack.h"

#define STACK_INITIAL_CAPACITY 8

void stack_new_test(void **state)
{
    Stack *stack = stack_new(sizeof(int));
    assert_int_equal(STACK_INITIAL_CAPACITY, stack->capacity);
    assert_int_equal(0, stack->length);
    assert_int_equal(sizeof(int), stack->elemsize);
    stack_free(stack);
}

void stack_single_push_pop_test(void **state)
{
    Stack *stack = stack_new(sizeof(int));
    int item = 100;
    stack_push(stack, &item);
    assert_int_equal(STACK_INITIAL_CAPACITY, stack->capacity);
    assert_int_equal(1, stack->length);
    assert_int_equal(sizeof(int), stack->elemsize);
    assert_int_equal(100, *(int *)stack_top(stack));
    stack_pop(stack);
    assert_int_equal(STACK_INITIAL_CAPACITY, stack->capacity);
    assert_int_equal(0, stack->length);
    assert_int_equal(sizeof(int), stack->elemsize);
    stack_free(stack);
}

void stack_multiple_push_pop_test(void **state)
{
    Stack *stack = stack_new(sizeof(int));

    for (int i = 0; i <= 500; i++)
    {
        assert_int_equal(i, stack->length);
        stack_push(stack, &i);
    }

    assert_int_equal(512, stack->capacity);

    for (int i = 500; i >= 0; i--)
    {
        assert_int_equal(i, *(int *)stack_top(stack));
        stack_pop(stack);
        assert_int_equal(i, stack->length);
    }

    assert_int_equal(STACK_INITIAL_CAPACITY, stack->capacity);

    stack_free(stack);
}

int main()
{
    set_config(test_malloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(stack_new_test),
            cmocka_unit_test(stack_single_push_pop_test),
            cmocka_unit_test(stack_multiple_push_pop_test),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}