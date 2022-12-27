#include "unit_testing.h"

#include "config.h"
#include "evalstack.h"

#define STACK_INITIAL_CAPACITY 8

void evalstack_new_test(void **state)
{
    EvalStack *evalstack = evalstack_new();
    assert_int_equal(STACK_INITIAL_CAPACITY, evalstack->capacity);
    assert_int_equal(0, evalstack->length);
    assert_int_equal(sizeof(EvalStackElement), evalstack->elemsize);
    evalstack_free(evalstack);
}

void evalstack_single_push_pop_test(void **state)
{
    EvalStack *evalstack = evalstack_new();
    EvalStackElement element = {.integer = 100};
    evalstack_push(evalstack, element);
    assert_int_equal(STACK_INITIAL_CAPACITY, evalstack->capacity);
    assert_int_equal(1, evalstack->length);
    assert_int_equal(sizeof(EvalStackElement), evalstack->elemsize);
    assert_int_equal(100, evalstack_top(evalstack).integer);
    evalstack_pop(evalstack);
    assert_int_equal(STACK_INITIAL_CAPACITY, evalstack->capacity);
    assert_int_equal(0, evalstack->length);
    assert_int_equal(sizeof(EvalStackElement), evalstack->elemsize);
    evalstack_free(evalstack);
}

void evalstack_multiple_push_pop_test(void **state)
{
    EvalStack *evalstack = evalstack_new();

    for (int i = 0; i <= 500; i++)
    {
        assert_int_equal(i, evalstack->length);
        evalstack_push(evalstack, (EvalStackElement){.integer = i});
    }

    assert_int_equal(512, evalstack->capacity);

    for (int i = 500; i >= 0; i--)
    {
        assert_int_equal(i, evalstack_top(evalstack).integer);
        evalstack_pop(evalstack);
        assert_int_equal(i, evalstack->length);
    }

    assert_int_equal(STACK_INITIAL_CAPACITY, evalstack->capacity);

    evalstack_free(evalstack);
}

int main()
{
    set_config(test_malloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(evalstack_new_test),
            cmocka_unit_test(evalstack_single_push_pop_test),
            cmocka_unit_test(evalstack_multiple_push_pop_test),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}