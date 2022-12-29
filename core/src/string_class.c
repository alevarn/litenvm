#include <string.h>

#include "config.h"
#include "constantpool.h"
#include "object.h"
#include "string_class.h"

void *string_new(const char *value)
{
    void *string_object = object_new(CONSTPOOL_CLASS_STRING, 1);
    *object_get_field(string_object, 0) = (EvalStackElement){.pointer = config._malloc(strlen(value) + 1)};
    strcpy(object_get_field(string_object, 0)->pointer, value);
    return string_object;
}

void *string_free(void *string_object)
{
    config._free(object_get_field(string_object, 0)->pointer);
    config._free(string_object);
}

const char *string_get_value(void *string_object)
{
    return (char *)object_get_field(string_object, 0)->pointer;
}