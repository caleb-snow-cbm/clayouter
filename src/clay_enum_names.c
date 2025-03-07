#include <stdint.h>

#include "clay.h"
#include "clay_enum_names.h"

void unused(void)
{
    (void) CLAY__ELEMENT_DEFINITION_LATCH;
}

#define ENUM_INFO(x)                                                                               \
    const enum_info_t _##x##_Info = { .name = &_##x##_Name,                                        \
        .values = _##x##_Names,                                                                    \
        .macros = _##x##_Macros,                                                                   \
        .count = sizeof(_##x##_Names) / sizeof(_##x##_Names[0]) }

const Clay_String _Clay_LayoutDirection_Name = CLAY_STRING("Layout Direction");
const Clay_String _Clay_LayoutDirection_Names[] = {
    CLAY_STRING("Left to right"),
    CLAY_STRING("Top to bottom")
};
const char* _Clay_LayoutDirection_Macros[] = {
    "LEFT_TO_RIGHT",
    "TOP_TO_BOTTOM",
};
ENUM_INFO(Clay_LayoutDirection);

const Clay_String _Clay_LayoutAlignmentX_Name = CLAY_STRING("X Alignment");
const Clay_String _Clay_LayoutAlignmentX_Names[] = {
    CLAY_STRING("Left"),
    CLAY_STRING("Right"),
    CLAY_STRING("Center")
};
const char* _Clay_LayoutAlignmentX_Macros[] = {
    "LEFT",
    "RIGHT",
    "CENTER",
};
ENUM_INFO(Clay_LayoutAlignmentX);

const Clay_String _Clay_LayoutAlignmentY_Name = CLAY_STRING("Y Alignment");
const Clay_String _Clay_LayoutAlignmentY_Names[] = {
    CLAY_STRING("Top"),
    CLAY_STRING("Bottom"),
    CLAY_STRING("Center"),
};
const char* _Clay_LayoutAlignmentY_Macros[] = {
    "TOP",
    "BOTTOM",
    "CENTER",
};
ENUM_INFO(Clay_LayoutAlignmentY);

const Clay_String _Clay__SizingType_Name = CLAY_STRING("Sizing");
const Clay_String _Clay__SizingType_Names[] = {
    CLAY_STRING("Fit"),
    CLAY_STRING("Grow"),
    CLAY_STRING("Percent"),
    CLAY_STRING("Fixed"),
};
ENUM_INFO(Clay__SizingType);
const char* _Clay__SizingType_Macros[] = {
    "FIT",
    "GROW",
    "PERCENT",
    "FIXED",
};

const Clay_String _Clay_TextElementConfigWrapMode_Name = CLAY_STRING("Wrap mode");
const Clay_String _Clay_TextElementConfigWrapMode_Names[] = {
    CLAY_STRING("Words"),
    CLAY_STRING("Newlines"),
    CLAY_STRING("None"),
};
const char* _Clay_TextElementConfigWrapMode_Macros[] = {
    "WORDS",
    "NEWLINES",
    "NONE"
};
ENUM_INFO(Clay_TextElementConfigWrapMode);

const Clay_String _Clay_TextAlignment_Name = CLAY_STRING("Alignment");
const Clay_String _Clay_TextAlignment_Names[] = {
    CLAY_STRING("Left"),
    CLAY_STRING("Center"),
    CLAY_STRING("Right"),
};
const char* _Clay_TextAlignment_Macros[] = {
    "LEFT",
    "CENTER",
    "RIGHT"
};
ENUM_INFO(Clay_TextAlignment);

const Clay_String _Clay_FloatingAttachPointType_Name = CLAY_STRING("Attach Point");
const Clay_String _Clay_FloatingAttachPointType_Names[] = {
    CLAY_STRING("Left Top"),
    CLAY_STRING("Left Center"),
    CLAY_STRING("Left Bottom"),
    CLAY_STRING("Center Top"),
    CLAY_STRING("Center Center"),
    CLAY_STRING("Center Bottom"),
    CLAY_STRING("Right Top"),
    CLAY_STRING("Right Center"),
    CLAY_STRING("Right Bottom"),
};
const char* _Clay_FloatingAttachPointType_Macros[] = {
    "LEFT_TOP",
    "LEFT_CENTER",
    "LEFT_BOTTOM",
    "CENTER_TOP",
    "CENTER_CENTER",
    "CENTER_BOTTOM",
    "RIGHT_TOP",
    "RIGHT_CENTER",
    "RIGHT_BOTTOM",
};
ENUM_INFO(Clay_FloatingAttachPointType);

const Clay_String _Clay_PointerCaptureMode_Name = CLAY_STRING("Capture Mode");
const Clay_String _Clay_PointerCaptureMode_Names[] = {
    CLAY_STRING("Capture"),
    CLAY_STRING("Passthrough"),
};
const char* _Clay_PointerCaptureMode_Macros[] = {
    "CAPTURE",
    "PASSTHROUGH",
};
ENUM_INFO(Clay_PointerCaptureMode);

const Clay_String _Clay_FloatingAttachToElement_Name = CLAY_STRING("Attach to");
const Clay_String _Clay_FloatingAttachToElement_Names[] = {
    CLAY_STRING("None"),
    CLAY_STRING("Parent"),
    CLAY_STRING("Element by ID"),
    CLAY_STRING("Root"),
};
const char* _Clay_FloatingAttachToElement_Macros[] = {
    "NONE",
    "PARENT",
    "ELEMENT_WITH_ID",
    "ROOT"
};
ENUM_INFO(Clay_FloatingAttachToElement);
