#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <cmocka.h>

#include "sample.h"

void get_magic_number_test(void **state)
{
    assert_int_equal(get_magic_number(), MY_MAGIC_NUMBER + 1);
}

int main()
{
    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(get_magic_number_test)
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}