#include "config.h"
#include "constantpool.h"
#include "string_class.h"
#include "string_builder_class.h"
#include "object.h"

void *object_new(uint32_t constpool_class, uint32_t fields_length)
{
    uint32_t *object = (uint32_t *)config._malloc(sizeof(uint32_t) + fields_length * sizeof(EvalStackElement));
    *object = constpool_class;
    return object;
}

void object_free(void *object)
{
    switch (object_get_class(object))
    {
    case CONSTPOOL_CLASS_STRING:
        string_free(object);
        break;
    case CONSTPOOL_CLASS_STRING_BUILDER:
        string_builder_free(object);
        break;
    default:
        config._free(object);
        break;
    }
}

uint32_t object_get_class(void *object)
{
    return *(uint32_t *)object;
}

EvalStackElement *object_get_field(void *object, uint32_t index)
{
    return (EvalStackElement *)((char *)object + sizeof(uint32_t) + index * sizeof(EvalStackElement));
}