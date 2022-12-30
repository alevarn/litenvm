#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "constantpool.h"
#include "object.h"
#include "string_class.h"
#include "string_builder_class.h"

void *string_builder_new()
{
    void *string_builder_object = object_new(CONSTPOOL_CLASS_STRING_BUILDER, 1);
    *object_get_field(string_builder_object, 0) = (EvalStackElement){.pointer = string_new("")};
    return string_builder_object;
}

void *string_builder_free(void *string_builder_object)
{
    string_free(object_get_field(string_builder_object, 0)->pointer);
    config._free(string_builder_object);
}

static void *concat_string(const char *str1, const char *str2)
{
    size_t str1_len = strlen(str1);
    size_t str2_len = strlen(str2);
    char *result = config._malloc(str1_len + str2_len + 1);
    strcpy(result, str1);
    strcat(result, str2);
    void *string_object = string_new(result);
    config._free(result);
    return string_object;
}

void *string_builder_append_string(void *string_builder_object, void *string_object)
{
    void *new_string_object = concat_string(string_get_value(object_get_field(string_builder_object, 0)->pointer), string_get_value(string_object));
    // Free the previous string object.
    string_free(object_get_field(string_builder_object, 0)->pointer);
    // Set the new string object.
    *object_get_field(string_builder_object, 0) = (EvalStackElement){.pointer = new_string_object};
}

void *string_builder_append_int(void *string_builder_object, int32_t int_value)
{
    char buffer[32];
    itoa(int_value, buffer, 10);
    void *new_string_object = concat_string(string_get_value(object_get_field(string_builder_object, 0)->pointer), buffer);
    // Free the previous string object.
    string_free(object_get_field(string_builder_object, 0)->pointer);
    // Set the new string object.
    *object_get_field(string_builder_object, 0) = (EvalStackElement){.pointer = new_string_object};
}

void *string_builder_append_bool(void *string_builder_object, int32_t bool_value)
{
    void *new_string_object = concat_string(string_get_value(object_get_field(string_builder_object, 0)->pointer), bool_value ? "true" : "false");
    // Free the previous string object.
    string_free(object_get_field(string_builder_object, 0)->pointer);
    // Set the new string object.
    *object_get_field(string_builder_object, 0) = (EvalStackElement){.pointer = new_string_object};
}

void *string_builder_to_string(void *string_builder_object)
{
    return object_get_field(string_builder_object, 0)->pointer;
}