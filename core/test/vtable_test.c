#include "unit_testing.h"

#include "config.h"
#include "vtable.h"

#define STACK_INITIAL_CAPACITY 8
#define VTABLE_LENGTH 4

void vtable_new_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    assert_int_equal(VTABLE_LENGTH, vtable->length);
    assert_int_equal(0, vtable_size(vtable));
    vtable_free(vtable);
}

void vtable_hash_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    char str[] = {'A', '\0'};
    while (str[0] != 'Z')
    {
        assert_in_range(vtable_hash(vtable, str), 0, VTABLE_LENGTH - 1);
        str[0]++;
    }
    vtable_free(vtable);
}

void vtable_exists_false_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    assert_false(vtable_exists(vtable, "abc"));
    vtable_free(vtable);
}

void vtable_exists_true_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    vtable_put(vtable, (VTableEntry){.method_name = "abc", .const_index = 1});
    assert_true(vtable_exists(vtable, "abc"));
    vtable_free(vtable);
}

void vtable_get_non_existing_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    assert_int_equal(0, vtable_get(vtable, "abc"));
    vtable_free(vtable);
}

void vtable_get_existing_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    vtable_put(vtable, (VTableEntry){.method_name = "abc", .const_index = 1});
    assert_int_equal(1, vtable_get(vtable, "abc"));
    vtable_free(vtable);
}

void vtable_put_replace_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    vtable_put(vtable, (VTableEntry){.method_name = "abc", .const_index = 1});
    assert_int_equal(1, vtable_get(vtable, "abc"));
    assert_int_equal(1, vtable_size(vtable));
    vtable_put(vtable, (VTableEntry){.method_name = "abc", .const_index = 2});
    assert_int_equal(2, vtable_get(vtable, "abc"));
    assert_int_equal(1, vtable_size(vtable));
    vtable_free(vtable);
}

void vtable_collision_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    assert_int_equal(vtable_hash(vtable, "A"), vtable_hash(vtable, "E"));
    vtable_put(vtable, (VTableEntry){.method_name = "A", .const_index = 1});
    vtable_put(vtable, (VTableEntry){.method_name = "E", .const_index = 2});
    assert_int_equal(1, vtable_get(vtable, "A"));
    assert_int_equal(2, vtable_get(vtable, "E"));
    assert_int_equal(2, vtable_size(vtable));
    vtable_free(vtable);
}

void vtable_almost_full_test(void **state)
{
    VTable *vtable = vtable_new(VTABLE_LENGTH);
    vtable_put(vtable, (VTableEntry){.method_name = "a", .const_index = 1});
    vtable_put(vtable, (VTableEntry){.method_name = "b", .const_index = 2});
    vtable_put(vtable, (VTableEntry){.method_name = "c", .const_index = 3});
    assert_int_equal(3, vtable_size(vtable));
    assert_true(vtable_exists(vtable, "a"));
    assert_true(vtable_exists(vtable, "b"));
    assert_true(vtable_exists(vtable, "c"));
    assert_false(vtable_exists(vtable, "d"));
    assert_false(vtable_exists(vtable, "e"));
    assert_false(vtable_exists(vtable, "f"));
    assert_int_equal(1, vtable_get(vtable, "a"));
    assert_int_equal(2, vtable_get(vtable, "b"));
    assert_int_equal(3, vtable_get(vtable, "c"));
    assert_int_equal(0, vtable_get(vtable, "d"));
    assert_int_equal(0, vtable_get(vtable, "e"));
    assert_int_equal(0, vtable_get(vtable, "f"));
    vtable_free(vtable);
}

void vtable_copy_test(void **state)
{
    VTable *src = vtable_new(VTABLE_LENGTH);
    vtable_put(src, (VTableEntry){.method_name = "a", .const_index = 1});
    vtable_put(src, (VTableEntry){.method_name = "b", .const_index = 2});
    VTable *dest = vtable_new(VTABLE_LENGTH);
    vtable_copy(dest, src);
    assert_true(vtable_exists(dest, "a"));
    assert_true(vtable_exists(dest, "b"));
    assert_int_equal(1, vtable_get(dest, "a"));
    assert_int_equal(2, vtable_get(dest, "b"));
    assert_int_equal(2, vtable_size(dest));
    vtable_free(src);
    vtable_free(dest);
}

int main()
{
    set_config(test_malloc_func, test_calloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(vtable_new_test),
            cmocka_unit_test(vtable_hash_test),
            cmocka_unit_test(vtable_exists_false_test),
            cmocka_unit_test(vtable_exists_true_test),
            cmocka_unit_test(vtable_get_non_existing_test),
            cmocka_unit_test(vtable_get_existing_test),
            cmocka_unit_test(vtable_put_replace_test),
            cmocka_unit_test(vtable_collision_test),
            cmocka_unit_test(vtable_almost_full_test),
            cmocka_unit_test(vtable_copy_test),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}