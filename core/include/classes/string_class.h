#ifndef STRING_CLASS_H
#define STRING_CLASS_H

void *string_new(const char *value);

void *string_free(void *string_object);

const char *string_get_value(void *string_object);

#endif