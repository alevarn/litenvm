#ifndef UNIT_TESTING_H
#define UNIT_TESTING_H

#include <stdarg.h>
#include <setjmp.h>
#include <stddef.h>
#include <cmocka.h>

void *test_malloc_func(size_t size)
{
    return test_malloc(size);
}

void *test_calloc_func(size_t nitems, size_t size)
{
    return test_calloc(nitems, size);
}

void *test_realloc_func(void *block, size_t size)
{
    return test_realloc(block, size);
}

void test_free_func(void *block)
{
    test_free(block);
}

#endif