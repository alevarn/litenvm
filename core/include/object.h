#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>

#include "evalstack.h"

void *object_new(uint32_t constpool_class, uint32_t fields_length);

void object_free(void *object);

uint32_t object_get_class(void *object);

EvalStackElement *object_get_field(void *object, uint32_t index);

#endif