#ifndef CLAY_ENUM_NAMES_H
#define CLAY_ENUM_NAMES_H

#include "clay.h"

typedef struct {
    const Clay_String* name;
    const Clay_String* values;
    const char** macros;
    uint8_t count;
} enum_info_t;

#define CLAY_ENUM_NAME(x) (_##x##_Name)
#define CLAY_ENUM_VALUE_NAME(x, value) _##x##_Names[value]
#define CLAY_ENUM_VALUE_MACRO(x, value) _##x##_Macros[value]
#define CLAY_ENUM_COUNT(x) (_##x##_Count)
#define CLAY_ENUM_INFO(x) (&_##x##_Info)
#define DECLARE_CLAY_ENUM_INFO(x)            \
    extern const Clay_String _##x##_Name;    \
    extern const Clay_String _##x##_Names[]; \
    extern const char* _##x##_Macros[];      \
    extern const uint8_t _##x##_Count;       \
    extern const enum_info_t _##x##_Info;

DECLARE_CLAY_ENUM_INFO(Clay_LayoutDirection);
DECLARE_CLAY_ENUM_INFO(Clay_LayoutAlignmentX);
DECLARE_CLAY_ENUM_INFO(Clay_LayoutAlignmentY);
DECLARE_CLAY_ENUM_INFO(Clay__SizingType);
DECLARE_CLAY_ENUM_INFO(Clay_TextElementConfigWrapMode);
DECLARE_CLAY_ENUM_INFO(Clay_TextAlignment);
DECLARE_CLAY_ENUM_INFO(Clay_FloatingAttachPointType);
DECLARE_CLAY_ENUM_INFO(Clay_PointerCaptureMode);
DECLARE_CLAY_ENUM_INFO(Clay_FloatingAttachToElement);

extern const char* _Clay__SizingType_Extra_Macros[];

#endif // CLAY_ENUM_NAMES_H
