#include <stdlib.h>
#include "clay.h"

#include "clay_struct_names.h"

#define STRUCT_MEMBER_NAMES(s) _##s##_Members
#ifdef _WIN32
#define STRUCT_MEMBER_COUNT(s) _countof(STRUCT_MEMBER_NAMES(s))
#else
#define STRUCT_MEMBER_COUNT(s) sizeof(STRUCT_MEMBER_NAMES(s)) / sizeof(STRUCT_MEMBER_NAMES(s)[0])
#endif

#define DEFINE_STRUCT_INFO(s)                                                                      \
    struct_info_t _##s##_Info = { .name = #s,                                                      \
        .members = _##s##_Members,                                                                 \
        .info = _##s##_Member_Info,                                                                \
        .sizes = _##s##_Sizes,                                                                     \
        .offsets = _##s##_Offsets,                                                                 \
        .count =  STRUCT_MEMBER_COUNT(s)}

/************************************************************************************/

const char* _Clay_Color_Members[] = { "r", "g", "b", "a" };
const size_t _Clay_Color_Sizes[] = {
    sizeof(((Clay_Color*) 0)->r),
    sizeof(((Clay_Color*) 0)->g),
    sizeof(((Clay_Color*) 0)->b),
    sizeof(((Clay_Color*) 0)->a),
};
const size_t _Clay_Color_Offsets[] = {
    offsetof(Clay_Color, r),
    offsetof(Clay_Color, g),
    offsetof(Clay_Color, b),
    offsetof(Clay_Color, a),
};
const member_info_t _Clay_Color_Member_Info[] = {
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
};
DEFINE_STRUCT_INFO(Clay_Color);

/************************************************************************************/

const char* _Clay_TextElementConfig_Members[] = {
    "textColor",
    "fontId",
    "fontSize",
    "letterSpacing",
    "lineHeight",
    "wrapMode",
    "textAlignment"
};
const size_t _Clay_TextElementConfig_Sizes[] = {
    sizeof(((Clay_TextElementConfig*) 0)->textColor),
    sizeof(((Clay_TextElementConfig*) 0)->fontId),
    sizeof(((Clay_TextElementConfig*) 0)->fontSize),
    sizeof(((Clay_TextElementConfig*) 0)->letterSpacing),
    sizeof(((Clay_TextElementConfig*) 0)->lineHeight),
    sizeof(((Clay_TextElementConfig*) 0)->wrapMode),
    sizeof(((Clay_TextElementConfig*) 0)->textAlignment),
    sizeof(((Clay_TextElementConfig*) 0)->hashStringContents),
};
const size_t _Clay_TextElementConfig_Offsets[] = {
    offsetof(Clay_TextElementConfig, textColor),
    offsetof(Clay_TextElementConfig, fontId),
    offsetof(Clay_TextElementConfig, fontSize),
    offsetof(Clay_TextElementConfig, letterSpacing),
    offsetof(Clay_TextElementConfig, lineHeight),
    offsetof(Clay_TextElementConfig, wrapMode),
    offsetof(Clay_TextElementConfig, textAlignment),
    offsetof(Clay_TextElementConfig, hashStringContents),
};
const member_info_t _Clay_TextElementConfig_Member_Info[] = {
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Color_Info },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_TextElementConfigWrapMode) },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_TextAlignment) },
    { .type = TYPE_BOOL },
};
DEFINE_STRUCT_INFO(Clay_TextElementConfig);

/************************************************************************************/

const char* _Clay_String_Members[] = {
    "length",
    "chars",
};
const size_t _Clay_String_Sizes[] = {
    sizeof(((Clay_String*) 0)->length),
    sizeof(((Clay_String*) 0)->chars),
};
const size_t _Clay_String_Offsets[] = {
    offsetof(Clay_String, length),
    offsetof(Clay_String, chars),
};
const member_info_t _Clay_String_Member_Info[] = {
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
};
DEFINE_STRUCT_INFO(Clay_String);

/************************************************************************************/

const char* _Clay_ElementId_Members[] = {
    "id",
    "offset",
    "baseId",
    "stringId",
};
const size_t _Clay_ElementId_Sizes[] = {
    sizeof(((Clay_ElementId*) 0)->id),
    sizeof(((Clay_ElementId*) 0)->offset),
    sizeof(((Clay_ElementId*) 0)->baseId),
    sizeof(((Clay_ElementId*) 0)->stringId),
};
const size_t _Clay_ElementId_Offsets[] = {
    offsetof(Clay_ElementId, id),
    offsetof(Clay_ElementId, offset),
    offsetof(Clay_ElementId, baseId),
    offsetof(Clay_ElementId, stringId),
};
const member_info_t _Clay_ElementId_Member_Info[] = {
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_String_Info },
};
DEFINE_STRUCT_INFO(Clay_ElementId);

/************************************************************************************/

const char* _Clay_SizingMinMax_Members[] = {
    "min",
    "max",
};
const size_t _Clay_SizingMinMax_Sizes[] = {
    sizeof(((Clay_SizingMinMax*) 0)->min),
    sizeof(((Clay_SizingMinMax*) 0)->max),
};
const size_t _Clay_SizingMinMax_Offsets[] = {
    offsetof(Clay_SizingMinMax, min),
    offsetof(Clay_SizingMinMax, max),
};
const member_info_t _Clay_SizingMinMax_Member_Info[] = {
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
};
DEFINE_STRUCT_INFO(Clay_SizingMinMax);

/************************************************************************************/

const char* _Clay_SizingAxis_Union_Members[] = {
    "minMax",
    "percent",
};
const size_t _Clay_SizingAxis_Union_Sizes[] = {
    sizeof(((Clay_SizingAxis*) 0)->size),
    sizeof(((Clay_SizingAxis*) 0)->size),
};
const size_t _Clay_SizingAxis_Union_Offsets[] = {
    0,
    0,
};
const member_info_t _Clay_SizingAxis_Union_Member_Info[] = {
    { .type = TYPE_STRUCT, .struct_info = &_Clay_SizingMinMax_Info },
    { .type = TYPE_FLOAT },
};
DEFINE_STRUCT_INFO(Clay_SizingAxis_Union);

/************************************************************************************/

const char* _Clay_SizingAxis_Members[] = {
    "size",
    "type",
};

const size_t _Clay_SizingAxis_Sizes[] = {
    sizeof(((Clay_SizingAxis*) 0)->size),
    sizeof(((Clay_SizingAxis*) 0)->type),
};
const size_t _Clay_SizingAxis_Offsets[] = {
    offsetof(Clay_SizingAxis, size),
    offsetof(Clay_SizingAxis, type),
};
const member_info_t _Clay_SizingAxis_Member_Info[] = {
    { .type = TYPE_UNION, .struct_info = &_Clay_SizingAxis_Union_Info },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay__SizingType) },
};
DEFINE_STRUCT_INFO(Clay_SizingAxis);

/************************************************************************************/

const char* _Clay_Sizing_Members[] = {
    "width",
    "height",
};
const size_t _Clay_Sizing_Sizes[] = {
    sizeof(((Clay_Sizing*) 0)->width),
    sizeof(((Clay_Sizing*) 0)->height),
};
const size_t _Clay_Sizing_Offsets[] = {
    offsetof(Clay_Sizing, width),
    offsetof(Clay_Sizing, height),
};
const member_info_t _Clay_Sizing_Member_Info[] = {
    { .type = TYPE_STRUCT, .struct_info = &_Clay_SizingAxis_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_SizingAxis_Info },
};
DEFINE_STRUCT_INFO(Clay_Sizing);

/************************************************************************************/

const char* _Clay_Padding_Members[] = {
    "left",
    "right",
    "top",
    "bottom",

};
const size_t _Clay_Padding_Sizes[] = {
    sizeof(((Clay_Padding*) 0)->left),
    sizeof(((Clay_Padding*) 0)->right),
    sizeof(((Clay_Padding*) 0)->top),
    sizeof(((Clay_Padding*) 0)->bottom),
};
const size_t _Clay_Padding_Offsets[] = {
    offsetof(Clay_Padding, left),
    offsetof(Clay_Padding, right),
    offsetof(Clay_Padding, top),
    offsetof(Clay_Padding, bottom),
};
const member_info_t _Clay_Padding_Member_Info[] = {
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
};
DEFINE_STRUCT_INFO(Clay_Padding);

/************************************************************************************/

const char* _Clay_ChildAlignment_Members[] = {
    "x",
    "y",
};
const size_t _Clay_ChildAlignment_Sizes[] = {
    sizeof(((Clay_ChildAlignment*) 0)->x),
    sizeof(((Clay_ChildAlignment*) 0)->y),
};
const size_t _Clay_ChildAlignment_Offsets[] = {
offsetof(Clay_ChildAlignment, x),
offsetof(Clay_ChildAlignment, y),
};
const member_info_t _Clay_ChildAlignment_Member_Info[] = {
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_LayoutAlignmentX) },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_LayoutAlignmentY) },
};
DEFINE_STRUCT_INFO(Clay_ChildAlignment);

/************************************************************************************/

const char* _Clay_LayoutConfig_Members[] = {
    "sizing",
    "padding",
    "childGap",
    "childAlignment",
    "layoutDirection",
};
const size_t _Clay_LayoutConfig_Sizes[] = {
    sizeof(((Clay_LayoutConfig*) 0)->sizing),
    sizeof(((Clay_LayoutConfig*) 0)->padding),
    sizeof(((Clay_LayoutConfig*) 0)->childGap),
    sizeof(((Clay_LayoutConfig*) 0)->childAlignment),
    sizeof(((Clay_LayoutConfig*) 0)->layoutDirection),
};
const size_t _Clay_LayoutConfig_Offsets[] = {
    offsetof(Clay_LayoutConfig, sizing),
    offsetof(Clay_LayoutConfig, padding),
    offsetof(Clay_LayoutConfig, childGap),
    offsetof(Clay_LayoutConfig, childAlignment),
    offsetof(Clay_LayoutConfig, layoutDirection),
};
const member_info_t _Clay_LayoutConfig_Member_Info[] = {
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Sizing_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Padding_Info },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_ChildAlignment_Info },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_LayoutDirection) },
};
DEFINE_STRUCT_INFO(Clay_LayoutConfig);

/************************************************************************************/

const char* _Clay_CornerRadius_Members[] = {
    "topLeft",
    "topRight",
    "bottomLeft",
    "bottomRight",
};
const size_t _Clay_CornerRadius_Sizes[] = {
    sizeof(((Clay_CornerRadius*) 0)->topLeft),
    sizeof(((Clay_CornerRadius*) 0)->topRight),
    sizeof(((Clay_CornerRadius*) 0)->bottomLeft),
    sizeof(((Clay_CornerRadius*) 0)->bottomRight),
};
const size_t _Clay_CornerRadius_Offsets[] = {
offsetof(Clay_CornerRadius, topLeft),
offsetof(Clay_CornerRadius, topRight),
offsetof(Clay_CornerRadius, bottomLeft),
offsetof(Clay_CornerRadius, bottomRight),
};
const member_info_t _Clay_CornerRadius_Member_Info[] = {
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
};
DEFINE_STRUCT_INFO(Clay_CornerRadius);

/************************************************************************************/

const char* _Clay_Dimensions_Members[] = {
    "width",
    "height",
};
const size_t _Clay_Dimensions_Sizes[] = {
    sizeof(((Clay_Dimensions*) 0)->width),
    sizeof(((Clay_Dimensions*) 0)->height),
};
const size_t _Clay_Dimensions_Offsets[] = {
    offsetof(Clay_Dimensions, width),
    offsetof(Clay_Dimensions, height),
};
const member_info_t _Clay_Dimensions_Member_Info[] = {
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
};
DEFINE_STRUCT_INFO(Clay_Dimensions);

/************************************************************************************/

const char* _Clay_ImageElementConfig_Members[] = {
    "imageData",
    "sourceDimensions",
};
const size_t _Clay_ImageElementConfig_Sizes[] = {
    sizeof(((Clay_ImageElementConfig*) 0)->imageData),
    sizeof(((Clay_ImageElementConfig*) 0)->sourceDimensions),
};
const size_t _Clay_ImageElementConfig_Offsets[] = {
offsetof(Clay_ImageElementConfig, imageData),
offsetof(Clay_ImageElementConfig, sourceDimensions),
};
const member_info_t _Clay_ImageElementConfig_Member_Info[] = {
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Dimensions_Info },
};
DEFINE_STRUCT_INFO(Clay_ImageElementConfig);

/************************************************************************************/

const char* _Clay_Vector2_Members[] = {
    "x",
    "y",
};
const size_t _Clay_Vector2_Sizes[] = {
    sizeof(((Clay_Vector2*) 0)->x),
    sizeof(((Clay_Vector2*) 0)->y),
};
const size_t _Clay_Vector2_Offsets[] = {
    offsetof(Clay_Vector2, x),
    offsetof(Clay_Vector2, y),
};
const member_info_t _Clay_Vector2_Member_Info[] = {
    { .type = TYPE_FLOAT },
    { .type = TYPE_FLOAT },
};
DEFINE_STRUCT_INFO(Clay_Vector2);

/************************************************************************************/

const char* _Clay_FloatingAttachPoints_Members[] = {
    "element",
    "parent",
};
const size_t _Clay_FloatingAttachPoints_Sizes[] = {
    sizeof(((Clay_FloatingAttachPoints*) 0)->element),
    sizeof(((Clay_FloatingAttachPoints*) 0)->parent),
};
const size_t _Clay_FloatingAttachPoints_Offsets[] = {
    offsetof(Clay_FloatingAttachPoints, element),
    offsetof(Clay_FloatingAttachPoints, parent),
};
const member_info_t _Clay_FloatingAttachPoints_Member_Info[] = {
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_FloatingAttachPointType) },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_FloatingAttachPointType) },
};
DEFINE_STRUCT_INFO(Clay_FloatingAttachPoints);

/************************************************************************************/

const char* _Clay_FloatingElementConfig_Members[] = {
    "offset",
    "expand",
    "parentId",
    "zIndex",
    "attachPoints",
    "pointerCaptureMode",
    "attachTo",
};
const size_t _Clay_FloatingElementConfig_Sizes[] = {
    sizeof(((Clay_FloatingElementConfig*) 0)->offset),
    sizeof(((Clay_FloatingElementConfig*) 0)->expand),
    sizeof(((Clay_FloatingElementConfig*) 0)->parentId),
    sizeof(((Clay_FloatingElementConfig*) 0)->zIndex),
    sizeof(((Clay_FloatingElementConfig*) 0)->attachPoints),
    sizeof(((Clay_FloatingElementConfig*) 0)->pointerCaptureMode),
    sizeof(((Clay_FloatingElementConfig*) 0)->attachTo),
};
const size_t _Clay_FloatingElementConfig_Offsets[] = {
    offsetof(Clay_FloatingElementConfig, offset),
    offsetof(Clay_FloatingElementConfig, expand),
    offsetof(Clay_FloatingElementConfig, parentId),
    offsetof(Clay_FloatingElementConfig, zIndex),
    offsetof(Clay_FloatingElementConfig, attachPoints),
    offsetof(Clay_FloatingElementConfig, pointerCaptureMode),
    offsetof(Clay_FloatingElementConfig, attachTo),
};
const member_info_t _Clay_FloatingElementConfig_Member_Info[] = {
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Vector2_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Dimensions_Info },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_FloatingAttachPoints_Info },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_PointerCaptureMode) },
    { .type = TYPE_ENUM, .enum_info = CLAY_ENUM_INFO(Clay_FloatingAttachToElement) },
};
DEFINE_STRUCT_INFO(Clay_FloatingElementConfig);

/************************************************************************************/

const char* _Clay_CustomElementConfig_Members[] = {
    "customData",
};
const size_t _Clay_CustomElementConfig_Sizes[] = {
    sizeof(((Clay_CustomElementConfig*) 0)->customData),
};
const size_t _Clay_CustomElementConfig_Offsets[] = {
    offsetof(Clay_CustomElementConfig, customData),
};
const member_info_t _Clay_CustomElementConfig_Member_Info[] = {
    { .type = TYPE_INTEGRAL },
};
DEFINE_STRUCT_INFO(Clay_CustomElementConfig);

/************************************************************************************/

const char* _Clay_ScrollElementConfig_Members[] = {
    "horizontal",
    "vertical",
};
const size_t _Clay_ScrollElementConfig_Sizes[] = {
    sizeof(((Clay_ScrollElementConfig*) 0)->horizontal),
    sizeof(((Clay_ScrollElementConfig*) 0)->vertical),
};
const size_t _Clay_ScrollElementConfig_Offsets[] = {
    offsetof(Clay_ScrollElementConfig, horizontal),
    offsetof(Clay_ScrollElementConfig, vertical),
};
const member_info_t _Clay_ScrollElementConfig_Member_Info[] = {
    { .type = TYPE_BOOL },
    { .type = TYPE_BOOL },
};
DEFINE_STRUCT_INFO(Clay_ScrollElementConfig);

/************************************************************************************/

const char* _Clay_BorderWidth_Members[] = {
    "left",
    "right",
    "top",
    "bottom",
    "betweenChildren",
};
const size_t _Clay_BorderWidth_Sizes[] = {
    sizeof(((Clay_BorderWidth*) 0)->left),
    sizeof(((Clay_BorderWidth*) 0)->right),
    sizeof(((Clay_BorderWidth*) 0)->top),
    sizeof(((Clay_BorderWidth*) 0)->bottom),
    sizeof(((Clay_BorderWidth*) 0)->betweenChildren),
};
const size_t _Clay_BorderWidth_Offsets[] = {
    offsetof(Clay_BorderWidth, left),
    offsetof(Clay_BorderWidth, right),
    offsetof(Clay_BorderWidth, top),
    offsetof(Clay_BorderWidth, bottom),
    offsetof(Clay_BorderWidth, betweenChildren),
};
const member_info_t _Clay_BorderWidth_Member_Info[] = {
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
    { .type = TYPE_INTEGRAL },
};
DEFINE_STRUCT_INFO(Clay_BorderWidth);

/************************************************************************************/

const char* _Clay_BorderElementConfig_Members[] = {
    "color",
    "width",
};
const size_t _Clay_BorderElementConfig_Sizes[] = {
    sizeof(((Clay_BorderElementConfig*) 0)->color),
    sizeof(((Clay_BorderElementConfig*) 0)->width),
};
const size_t _Clay_BorderElementConfig_Offsets[] = {
    offsetof(Clay_BorderElementConfig, color),
    offsetof(Clay_BorderElementConfig, width),
};
const member_info_t _Clay_BorderElementConfig_Member_Info[] = {
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Color_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_BorderWidth_Info },
};
DEFINE_STRUCT_INFO(Clay_BorderElementConfig);

/************************************************************************************/

const char* _Clay_ElementDeclaration_Members[] = {
    "id",
    "layout",
    "backgroundColor",
    "cornerRadius",
    "image",
    "floating",
    "custom",
    "scroll",
    "border",
    "userData",
};
const size_t _Clay_ElementDeclaration_Sizes[] = {
    sizeof(((Clay_ElementDeclaration*) 0)->id),
    sizeof(((Clay_ElementDeclaration*) 0)->layout),
    sizeof(((Clay_ElementDeclaration*) 0)->backgroundColor),
    sizeof(((Clay_ElementDeclaration*) 0)->cornerRadius),
    sizeof(((Clay_ElementDeclaration*) 0)->image),
    sizeof(((Clay_ElementDeclaration*) 0)->floating),
    sizeof(((Clay_ElementDeclaration*) 0)->custom),
    sizeof(((Clay_ElementDeclaration*) 0)->scroll),
    sizeof(((Clay_ElementDeclaration*) 0)->border),
};
const size_t _Clay_ElementDeclaration_Offsets[] = {
    offsetof(Clay_ElementDeclaration, id),
    offsetof(Clay_ElementDeclaration, layout),
    offsetof(Clay_ElementDeclaration, backgroundColor),
    offsetof(Clay_ElementDeclaration, cornerRadius),
    offsetof(Clay_ElementDeclaration, image),
    offsetof(Clay_ElementDeclaration, floating),
    offsetof(Clay_ElementDeclaration, custom),
    offsetof(Clay_ElementDeclaration, scroll),
    offsetof(Clay_ElementDeclaration, border),
};
const member_info_t _Clay_ElementDeclaration_Member_Info[] = {
    { .type = TYPE_CUSTOM, .struct_info = &_Clay_ElementId_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_LayoutConfig_Info },
    { .type = TYPE_CUSTOM, .struct_info = &_Clay_Color_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_CornerRadius_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_ImageElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_FloatingElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_CustomElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_ScrollElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_BorderElementConfig_Info },
    { .type = TYPE_INTEGRAL },
};
DEFINE_STRUCT_INFO(Clay_ElementDeclaration);
