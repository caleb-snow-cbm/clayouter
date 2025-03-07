#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#include "clay.h"

typedef struct {
    int32_t capacity;
    Clay_String s;
} dstring_t;

typedef struct {
    dstring_t r;
    dstring_t g;
    dstring_t b;
    dstring_t a;
} color_string_t;

#endif // TYPES_H
