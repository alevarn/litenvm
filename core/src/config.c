#include "config.h"

Config config = {
    .allocator = malloc,
    .deallocator = free,
    .evalblock_length = 64};

void set_config(void *(*allocator)(size_t), void (*deallocator)(void *), int evalblock_length)
{
    config.allocator = allocator;
    config.deallocator = deallocator;
    config.evalblock_length = evalblock_length;
}