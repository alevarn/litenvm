#ifndef STRING_BUILDER_CLASS_H
#define STRING_BUILDER_CLASS_H

#include <stdint.h>

void *string_builder_new();

void *string_builder_free(void *string_builder_object);

void *string_builder_append_string(void *string_builder_object, void *string_object);

void *string_builder_append_int(void *string_builder_object, int32_t int_value);

void *string_builder_append_bool(void *string_builder_object, int32_t bool_value);

void *string_builder_to_string(void *string_builder_object);

#endif