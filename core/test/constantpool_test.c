#include "unit_testing.h"

#include "config.h"
#include "constantpool.h"

#define STACK_INITIAL_CAPACITY 8

void constantpool_new_test(void **state)
{
    ConstantPool *constpool = constantpool_new(2);
    assert_int_equal(2, constpool->length);
    constantpool_free(constpool);
}

void constantpool_add_get_test(void **state)
{
    ConstantPool *constpool = constantpool_new(2);
    assert_int_equal(2, constpool->length);
    constantpool_add(constpool, 1, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Animal", .fields = 2, .methods = 3, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 2, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Dog", .fields = 0, .methods = 1, .parent = 1, .vtable = NULL}});

    assert_int_equal(TYPE_CLASS, constantpool_get(constpool, 1)->type);
    assert_string_equal("Animal", constantpool_get(constpool, 1)->data._class.name);
    assert_int_equal(2, constantpool_get(constpool, 1)->data._class.fields);
    assert_int_equal(3, constantpool_get(constpool, 1)->data._class.methods);
    assert_int_equal(0, constantpool_get(constpool, 1)->data._class.parent);

    assert_int_equal(TYPE_CLASS, constantpool_get(constpool, 2)->type);
    assert_string_equal("Dog", constantpool_get(constpool, 2)->data._class.name);
    assert_int_equal(0, constantpool_get(constpool, 2)->data._class.fields);
    assert_int_equal(1, constantpool_get(constpool, 2)->data._class.methods);
    assert_int_equal(1, constantpool_get(constpool, 2)->data._class.parent);

    constantpool_free(constpool);
}

void constantpool_compute_vtables_test(void **state)
{
    ConstantPool *constpool = constantpool_new(9);
    constantpool_add(constpool, 1, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Animal", .fields = 0, .methods = 3, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 2, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "makeSound", ._class = 1, .address = 20, .args = 0, .locals = 0}});
    constantpool_add(constpool, 3, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "jump", ._class = 1, .address = 25, .args = 0, .locals = 0}});
    constantpool_add(constpool, 4, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = false, .name = "isAnimal", ._class = 1, .address = 30, .args = 0, .locals = 0}});

    constantpool_add(constpool, 5, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Dog", .fields = 0, .methods = 4, .parent = 1, .vtable = NULL}});
    constantpool_add(constpool, 6, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "makeSound", ._class = 5, .address = 40, .args = 0, .locals = 0}});

    constantpool_add(constpool, 7, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Cat", .fields = 0, .methods = 5, .parent = 1, .vtable = NULL}});
    constantpool_add(constpool, 8, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "makeSound", ._class = 7, .address = 50, .args = 0, .locals = 0}});
    constantpool_add(constpool, 9, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "jump", ._class = 7, .address = 70, .args = 0, .locals = 0}});

    constantpool_compute_vtables(constpool);

    VTable *animal_vtable = constantpool_get(constpool, 1)->data._class.vtable;
    VTable *dog_vtable = constantpool_get(constpool, 5)->data._class.vtable;
    VTable *cat_vtable = constantpool_get(constpool, 7)->data._class.vtable;

    assert_int_equal(2, vtable_size(animal_vtable));
    assert_int_equal(2, vtable_size(dog_vtable));
    assert_int_equal(2, vtable_size(cat_vtable));

    assert_int_equal(2, vtable_get(animal_vtable, "makeSound"));
    assert_int_equal(3, vtable_get(animal_vtable, "jump"));
    assert_int_equal(6, vtable_get(dog_vtable, "makeSound"));
    assert_int_equal(3, vtable_get(dog_vtable, "jump"));
    assert_int_equal(8, vtable_get(cat_vtable, "makeSound"));
    assert_int_equal(9, vtable_get(cat_vtable, "jump"));

    constantpool_free(constpool);
}

void constantpool_compute_vtables_depth_three_test(void **state)
{
    ConstantPool *constpool = constantpool_new(10);
    constantpool_add(constpool, 1, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Vehicle", .fields = 0, .methods = 2, .parent = 0, .vtable = NULL}});
    constantpool_add(constpool, 2, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "hasEngine", ._class = 1, .address = 0, .args = 0, .locals = 0}});
    constantpool_add(constpool, 3, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "isVehicle", ._class = 1, .address = 0, .args = 0, .locals = 0}});

    constantpool_add(constpool, 4, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Car", .fields = 0, .methods = 5, .parent = 1, .vtable = NULL}});
    constantpool_add(constpool, 5, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "hasEngine", ._class = 4, .address = 0, .args = 0, .locals = 0}});
    constantpool_add(constpool, 6, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "gas", ._class = 4, .address = 0, .args = 0, .locals = 0}});
    constantpool_add(constpool, 7, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "brake", ._class = 4, .address = 0, .args = 0, .locals = 0}});

    constantpool_add(constpool, 8, (ConstantPoolEntry){.type = TYPE_CLASS, .data._class = {.name = "Volvo", .fields = 0, .methods = 7, .parent = 4, .vtable = NULL}});
    constantpool_add(constpool, 9, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "brake", ._class = 8, .address = 0, .args = 0, .locals = 0}});
    constantpool_add(constpool, 10, (ConstantPoolEntry){.type = TYPE_METHOD, .data.method = {.virtual = true, .name = "autopilot", ._class = 8, .address = 0, .args = 0, .locals = 0}});

    constantpool_compute_vtables(constpool);

    VTable *vehicle_table = constantpool_get(constpool, 1)->data._class.vtable;
    VTable *car_table = constantpool_get(constpool, 4)->data._class.vtable;
    VTable *volvo_table = constantpool_get(constpool, 8)->data._class.vtable;

    assert_int_equal(2, vtable_size(vehicle_table));
    assert_int_equal(4, vtable_size(car_table));
    assert_int_equal(5, vtable_size(volvo_table));

    assert_int_equal(2, vtable_get(vehicle_table, "hasEngine"));
    assert_int_equal(3, vtable_get(vehicle_table, "isVehicle"));

    assert_int_equal(5, vtable_get(car_table, "hasEngine"));
    assert_int_equal(3, vtable_get(car_table, "isVehicle"));
    assert_int_equal(6, vtable_get(car_table, "gas"));
    assert_int_equal(7, vtable_get(car_table, "brake"));

    assert_int_equal(5, vtable_get(volvo_table, "hasEngine"));
    assert_int_equal(3, vtable_get(volvo_table, "isVehicle"));
    assert_int_equal(6, vtable_get(volvo_table, "gas"));
    assert_int_equal(9, vtable_get(volvo_table, "brake"));
    assert_int_equal(10, vtable_get(volvo_table, "autopilot"));

    constantpool_free(constpool);
}

int main()
{
    set_config(test_malloc_func, test_calloc_func, test_realloc_func, test_free_func, STACK_INITIAL_CAPACITY);

    const struct CMUnitTest tests[] =
        {
            cmocka_unit_test(constantpool_new_test),
            cmocka_unit_test(constantpool_add_get_test),
            cmocka_unit_test(constantpool_compute_vtables_test),
            cmocka_unit_test(constantpool_compute_vtables_depth_three_test),
        };

    return cmocka_run_group_tests(tests, NULL, NULL);
}