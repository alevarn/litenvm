#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <cmocka.h>

#include "sample.h"

void get_magic_number_test(void **state)
{
    assert_int_equal(get_magic_number(), MY_MAGIC_NUMBER);
}

void get_two_test(void **state)
{
    assert_int_equal(get_two(), 2);
}

void get_hello_str_test(void **state)
{
    assert_string_equal(get_hello_str(), "Hello");
}

int main()
{
    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(get_magic_number_test),
            cmocka_unit_test(get_two_test),
            cmocka_unit_test(get_hello_str_test),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}