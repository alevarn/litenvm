#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

typedef struct
{
    void *(*_malloc)(size_t);
    void *(*_realloc)(void *, size_t);
    void (*_free)(void *);
    size_t min_stack_capacity;
} Config;

extern Config config;

void set_config(
    void *(*_malloc)(size_t),
    void *(*_realloc)(void *, size_t),
    void (*_free)(void *),
    size_t min_stack_capacity);

#endif