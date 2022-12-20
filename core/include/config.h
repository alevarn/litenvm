#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

typedef struct
{
    void *(*allocator)(size_t);
    void (*deallocator)(void *);
    int evalblock_length;
} Config;

extern Config config;

void set_config(void *(*allocator)(size_t), void (*deallocator)(void *), int evalblock_length);

#endif