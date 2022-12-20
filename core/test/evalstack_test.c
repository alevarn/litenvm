#include "unit_testing.h"

#include "config.h"
#include "evalstack.h"

void evalstack_new_test(void **state)
{
    EvalBlock *evalstack = evalstack_new();
    assert_non_null(evalstack);
    evalstack = evalstack_shrink(evalstack);
    assert_null(evalstack);
}

void evalstack_grow_test(void **state)
{
    EvalBlock *evalstack = evalstack_new();
    assert_non_null(evalstack);
    evalstack = evalstack_grow(evalstack);
    assert_non_null(evalstack);
    evalstack = evalstack_shrink(evalstack);
    assert_non_null(evalstack);
    evalstack = evalstack_shrink(evalstack);
    assert_null(evalstack);
}

void evalstack_grow_twice_test(void **state)
{
    EvalBlock *evalstack = evalstack_new();
    assert_non_null(evalstack);
    evalstack = evalstack_grow(evalstack);
    assert_non_null(evalstack);
    evalstack = evalstack_grow(evalstack);
    assert_non_null(evalstack);
    evalstack = evalstack_shrink(evalstack);
    assert_non_null(evalstack);
    evalstack = evalstack_shrink(evalstack);
    assert_non_null(evalstack);
    evalstack = evalstack_shrink(evalstack);
    assert_null(evalstack);
}

int main()
{
    set_config(test_malloc_func, test_free_func, 2);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(evalstack_new_test),
            cmocka_unit_test(evalstack_grow_test),
            cmocka_unit_test(evalstack_grow_twice_test)
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}