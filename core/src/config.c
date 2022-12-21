#include "config.h"

Config config = {
    ._malloc = malloc,
    ._realloc = realloc,
    ._free = free,
    .min_stack_capacity = 128,
};

void set_config(
    void *(*_malloc)(size_t),
    void *(*_realloc)(void *, size_t),
    void (*_free)(void *),
    size_t min_stack_capacity)
{
    config._malloc = _malloc;
    config._realloc = _realloc;
    config._free = _free;
    config.min_stack_capacity = min_stack_capacity;
}