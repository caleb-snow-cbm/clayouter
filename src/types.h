#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

#include "clay.h"

typedef struct {
    int32_t capacity;
    Clay_String s;
} dstring_t;

#endif // TYPES_H
