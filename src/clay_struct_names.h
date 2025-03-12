#ifndef CLAY_STRUCT_NAMES_H
#define CLAY_STRUCT_NAMES_H

#include <stddef.h>

#include "clay.h"
#include "clay_enum_names.h"

typedef enum {
    TYPE_BOOL,
    TYPE_INTEGRAL,
    TYPE_FLOAT,
    TYPE_ENUM,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_CUSTOM, // handled differently (e.g. with CLAY_ID())
} type_t;

typedef struct {
    type_t type;
    union {
        const struct struct_info* struct_info;
        const enum_info_t* enum_info;
    };
} member_info_t;

typedef struct struct_info {
    const char* name;
    const char** members;
    const member_info_t* info;
    const size_t* sizes;
    const size_t* offsets;
    size_t count;
} struct_info_t;

#define STRUCT_INFO(x) (&_##x##_Info)
#define DECLARE_STRUCT_INFO(s) extern struct_info_t _##s##_Info

DECLARE_STRUCT_INFO(Clay_TextElementConfig);
DECLARE_STRUCT_INFO(Clay_ElementDeclaration);
DECLARE_STRUCT_INFO(Clay_SizingAxis);
DECLARE_STRUCT_INFO(Clay_Padding);
DECLARE_STRUCT_INFO(Clay_CornerRadius);

#endif // CLAY_STRUCT_NAMES_H
