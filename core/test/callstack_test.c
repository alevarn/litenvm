#include "unit_testing.h"

#include "config.h"
#include "callstack.h"

#define STACK_INITIAL_CAPACITY 8

void callstack_new_test(void **state)
{
    CallStack *callstack = callstack_new();
    assert_int_equal(STACK_INITIAL_CAPACITY, callstack->capacity);
    assert_int_equal(0, callstack->length);
    assert_int_equal(sizeof(CallStackFrame), callstack->elemsize);
    callstack_free(callstack);
}

void callstack_single_push_pop_test(void **state)
{
    CallStack *callstack = callstack_new();
    CallStackFrame frame = {.vars_count = 0, .vars = NULL, .return_address = 100};
    callstack_push(callstack, frame);
    assert_int_equal(STACK_INITIAL_CAPACITY, callstack->capacity);
    assert_int_equal(1, callstack->length);
    assert_int_equal(sizeof(CallStackFrame), callstack->elemsize);
    assert_int_equal(0, callstack_top(callstack).vars_count);
    assert_null(callstack_top(callstack).vars);
    assert_int_equal(100, callstack_top(callstack).return_address);
    callstack_pop(callstack);
    assert_int_equal(STACK_INITIAL_CAPACITY, callstack->capacity);
    assert_int_equal(0, callstack->length);
    assert_int_equal(sizeof(CallStackFrame), callstack->elemsize);
    callstack_free(callstack);
}

void callstack_multiple_push_pop_test(void **state)
{
    CallStack *callstack = callstack_new();

    for (int i = 0; i <= 500; i++)
    {
        assert_int_equal(i, callstack->length);
        callstack_push(callstack, (CallStackFrame){.return_address = i, .vars_count = 0, .vars = NULL});
    }

    assert_int_equal(512, callstack->capacity);

    for (int i = 500; i >= 0; i--)
    {
        assert_int_equal(i, callstack_top(callstack).return_address);
        callstack_pop(callstack);
        assert_int_equal(i, callstack->length);
    }

    assert_int_equal(STACK_INITIAL_CAPACITY, callstack->capacity);

    callstack_free(callstack);
}

int main()
{
    set_config(test_malloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(callstack_new_test),
            cmocka_unit_test(callstack_single_push_pop_test),
            cmocka_unit_test(callstack_multiple_push_pop_test),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}