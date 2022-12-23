#include "unit_testing.h"

#include "config.h"
#include "constantpool.h"

#define STACK_INITIAL_CAPACITY 8

void constantpool_new_test(void **state)
{
    ConstantPool constpool = constantpool_new(2);
    assert_int_equal(2, constpool.length);
    constantpool_free(&constpool);
}

void constantpool_add_get_test(void **state)
{
    ConstantPool constpool = constantpool_new(2);
    assert_int_equal(2, constpool.length);
    constantpool_add(&constpool, 1, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Animal", .fields = 2, .methods = 3, .parent = 0}});
    constantpool_add(&constpool, 2, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Dog", .fields = 0, .methods = 1, .parent = 1}});

    assert_int_equal(TYPE_CLASS, constantpool_get(&constpool, 1)->type);
    assert_string_equal("Animal", constantpool_get(&constpool, 1)->data._class.name);
    assert_int_equal(2, constantpool_get(&constpool, 1)->data._class.fields);
    assert_int_equal(3, constantpool_get(&constpool, 1)->data._class.methods);
    assert_int_equal(0, constantpool_get(&constpool, 1)->data._class.parent);

    assert_int_equal(TYPE_CLASS, constantpool_get(&constpool, 2)->type);
    assert_string_equal("Dog", constantpool_get(&constpool, 2)->data._class.name);
    assert_int_equal(0, constantpool_get(&constpool, 2)->data._class.fields);
    assert_int_equal(1, constantpool_get(&constpool, 2)->data._class.methods);
    assert_int_equal(1, constantpool_get(&constpool, 2)->data._class.parent);

    constantpool_free(&constpool);
}

int main()
{
    set_config(test_malloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(constantpool_new_test),
            cmocka_unit_test(constantpool_add_get_test),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}