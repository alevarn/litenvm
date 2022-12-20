#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <cmocka.h>

#include "evalstack.h"

void evalstack_new_test(void **state)
{
    EvalBlock *evalstack = evalstack_new();
}

int main()
{
    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(evalstack_new_test)};

    return cmocka_run_group_tests(tests, NULL, NULL);
}