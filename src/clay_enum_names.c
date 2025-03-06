#include "clay.h"

void unused(void)
{
    (void) CLAY__ELEMENT_DEFINITION_LATCH;
}

#define ENUM_COUNT(x) const uint8_t _##x##_Count = sizeof(_##x##_Names) / sizeof(_##x##_Names[0])

const Clay_String _Clay_LayoutDirection_Name = CLAY_STRING("Layout Direction");
const Clay_String _Clay_LayoutDirection_Names[] = {
    CLAY_STRING("Left to right"),
    CLAY_STRING("Top to bottom")
};
ENUM_COUNT(Clay_LayoutDirection);

const Clay_String _Clay_LayoutAlignmentX_Name = CLAY_STRING("X Alignment");
const Clay_String _Clay_LayoutAlignmentX_Names[] = {
    CLAY_STRING("Left"),
    CLAY_STRING("Right"),
    CLAY_STRING("Center")
};
ENUM_COUNT(Clay_LayoutAlignmentX);

const Clay_String _Clay_LayoutAlignmentY_Name = CLAY_STRING("Y Alignment");
const Clay_String _Clay_LayoutAlignmentY_Names[] = {
    CLAY_STRING("Top"),
    CLAY_STRING("Bottom"),
    CLAY_STRING("Center"),
};
ENUM_COUNT(Clay_LayoutAlignmentY);

const Clay_String _Clay__SizingType_Name = CLAY_STRING("Sizing");
const Clay_String _Clay__SizingType_Names[] = {
    CLAY_STRING("Fit"),
    CLAY_STRING("Grow"),
    CLAY_STRING("Percent"),
    CLAY_STRING("Fixed"),
};
ENUM_COUNT(Clay__SizingType);

const Clay_String _Clay_TextElementConfigWrapMode_Name = CLAY_STRING("Wrap mode");
const Clay_String _Clay_TextElementConfigWrapMode_Names[] = {
    CLAY_STRING("Words"),
    CLAY_STRING("Newlines"),
    CLAY_STRING("None"),
};
ENUM_COUNT(Clay_TextElementConfigWrapMode);

const Clay_String _Clay_TextAlignment_Name = CLAY_STRING("Alignment");
const Clay_String _Clay_TextAlignment_Names[] = {
    CLAY_STRING("Left"),
    CLAY_STRING("Center"),
    CLAY_STRING("Right"),
};
ENUM_COUNT(Clay_TextAlignment);

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
ENUM_COUNT(Clay_FloatingAttachPointType);

const Clay_String _Clay_PointerCaptureMode_Name = CLAY_STRING("Capture Mode");
const Clay_String _Clay_PointerCaptureMode_Names[] = {
    CLAY_STRING("Capture"),
    CLAY_STRING("Passthrough"),
};
ENUM_COUNT(Clay_PointerCaptureMode);

const Clay_String _Clay_FloatingAttachToElement_Name = CLAY_STRING("Attach to");
const Clay_String _Clay_FloatingAttachToElement_Names[] = {
    CLAY_STRING("None"),
    CLAY_STRING("Parent"),
    CLAY_STRING("Element by ID"),
    CLAY_STRING("Root"),
};
ENUM_COUNT(Clay_FloatingAttachToElement);
