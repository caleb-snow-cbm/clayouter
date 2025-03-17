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
static Clay_Color _Clay_Color;
const size_t _Clay_Color_Sizes[] = {
    sizeof(_Clay_Color.r),
    sizeof(_Clay_Color.g),
    sizeof(_Clay_Color.b),
    sizeof(_Clay_Color.a),
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
static Clay_TextElementConfig _Clay_TextElementConfig;
const size_t _Clay_TextElementConfig_Sizes[] = {
    sizeof(_Clay_TextElementConfig.textColor),
    sizeof(_Clay_TextElementConfig.fontId),
    sizeof(_Clay_TextElementConfig.fontSize),
    sizeof(_Clay_TextElementConfig.letterSpacing),
    sizeof(_Clay_TextElementConfig.lineHeight),
    sizeof(_Clay_TextElementConfig.wrapMode),
    sizeof(_Clay_TextElementConfig.textAlignment),
    sizeof(_Clay_TextElementConfig.hashStringContents),
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
static Clay_String _Clay_String;
const size_t _Clay_String_Sizes[] = {
    sizeof(_Clay_String.length),
    sizeof(_Clay_String.chars),
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
static Clay_ElementId _Clay_ElementId;
const size_t _Clay_ElementId_Sizes[] = {
    sizeof(_Clay_ElementId.id),
    sizeof(_Clay_ElementId.offset),
    sizeof(_Clay_ElementId.baseId),
    sizeof(_Clay_ElementId.stringId),
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
static Clay_SizingMinMax _Clay_SizingMinMax;
const size_t _Clay_SizingMinMax_Sizes[] = {
    sizeof(_Clay_SizingMinMax.min),
    sizeof(_Clay_SizingMinMax.max),
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

static Clay_SizingAxis _Clay_SizingAxis;

/************************************************************************************/

const char* _Clay_SizingAxis_Union_Members[] = {
    "minMax",
    "percent",
};
const size_t _Clay_SizingAxis_Union_Sizes[] = {
    sizeof(_Clay_SizingAxis.size),
    sizeof(_Clay_SizingAxis.size),
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
    sizeof(_Clay_SizingAxis.size),
    sizeof(_Clay_SizingAxis.type),
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
static Clay_Sizing _Clay_Sizing;
const size_t _Clay_Sizing_Sizes[] = {
    sizeof(_Clay_Sizing.width),
    sizeof(_Clay_Sizing.height),
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
static Clay_Padding _Clay_Padding;
const size_t _Clay_Padding_Sizes[] = {
    sizeof(_Clay_Padding.left),
    sizeof(_Clay_Padding.right),
    sizeof(_Clay_Padding.top),
    sizeof(_Clay_Padding.bottom),
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
static Clay_ChildAlignment _Clay_ChildAlignment;
const size_t _Clay_ChildAlignment_Sizes[] = {
    sizeof(_Clay_ChildAlignment.x),
    sizeof(_Clay_ChildAlignment.y),
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
static Clay_LayoutConfig _Clay_LayoutConfig;
const size_t _Clay_LayoutConfig_Sizes[] = {
    sizeof(_Clay_LayoutConfig.sizing),
    sizeof(_Clay_LayoutConfig.padding),
    sizeof(_Clay_LayoutConfig.childGap),
    sizeof(_Clay_LayoutConfig.childAlignment),
    sizeof(_Clay_LayoutConfig.layoutDirection),
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
static Clay_CornerRadius _Clay_CornerRadius;
const size_t _Clay_CornerRadius_Sizes[] = {
    sizeof(_Clay_CornerRadius.topLeft),
    sizeof(_Clay_CornerRadius.topRight),
    sizeof(_Clay_CornerRadius.bottomLeft),
    sizeof(_Clay_CornerRadius.bottomRight),
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
static Clay_Dimensions _Clay_Dimensions;
const size_t _Clay_Dimensions_Sizes[] = {
    sizeof(_Clay_Dimensions.width),
    sizeof(_Clay_Dimensions.height),
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
static Clay_ImageElementConfig _Clay_ImageElementConfig;
const size_t _Clay_ImageElementConfig_Sizes[] = {
    sizeof(_Clay_ImageElementConfig.imageData),
    sizeof(_Clay_ImageElementConfig.sourceDimensions),
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
static Clay_Vector2 _Clay_Vector2;
const size_t _Clay_Vector2_Sizes[] = {
    sizeof(_Clay_Vector2.x),
    sizeof(_Clay_Vector2.y),
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
static Clay_FloatingAttachPoints _Clay_FloatingAttachPoints;
const size_t _Clay_FloatingAttachPoints_Sizes[] = {
    sizeof(_Clay_FloatingAttachPoints.element),
    sizeof(_Clay_FloatingAttachPoints.parent),
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
static Clay_FloatingElementConfig _Clay_FloatingElementConfig;
const size_t _Clay_FloatingElementConfig_Sizes[] = {
    sizeof(_Clay_FloatingElementConfig.offset),
    sizeof(_Clay_FloatingElementConfig.expand),
    sizeof(_Clay_FloatingElementConfig.parentId),
    sizeof(_Clay_FloatingElementConfig.zIndex),
    sizeof(_Clay_FloatingElementConfig.attachPoints),
    sizeof(_Clay_FloatingElementConfig.pointerCaptureMode),
    sizeof(_Clay_FloatingElementConfig.attachTo),
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
static Clay_CustomElementConfig _Clay_CustomElementConfig;
const size_t _Clay_CustomElementConfig_Sizes[] = {
    sizeof(_Clay_CustomElementConfig.customData),
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
static Clay_ScrollElementConfig _Clay_ScrollElementConfig;
const size_t _Clay_ScrollElementConfig_Sizes[] = {
    sizeof(_Clay_ScrollElementConfig.horizontal),
    sizeof(_Clay_ScrollElementConfig.vertical),
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
static Clay_BorderWidth _Clay_BorderWidth;
const size_t _Clay_BorderWidth_Sizes[] = {
    sizeof(_Clay_BorderWidth.left),
    sizeof(_Clay_BorderWidth.right),
    sizeof(_Clay_BorderWidth.top),
    sizeof(_Clay_BorderWidth.bottom),
    sizeof(_Clay_BorderWidth.betweenChildren),
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
static Clay_BorderElementConfig _Clay_BorderElementConfig;
const size_t _Clay_BorderElementConfig_Sizes[] = {
    sizeof(_Clay_BorderElementConfig.color),
    sizeof(_Clay_BorderElementConfig.width),
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
static Clay_ElementDeclaration _Clay_ElementDeclaration;
const size_t _Clay_ElementDeclaration_Sizes[] = {
    sizeof(_Clay_ElementDeclaration.id),
    sizeof(_Clay_ElementDeclaration.layout),
    sizeof(_Clay_ElementDeclaration.backgroundColor),
    sizeof(_Clay_ElementDeclaration.cornerRadius),
    sizeof(_Clay_ElementDeclaration.image),
    sizeof(_Clay_ElementDeclaration.floating),
    sizeof(_Clay_ElementDeclaration.custom),
    sizeof(_Clay_ElementDeclaration.scroll),
    sizeof(_Clay_ElementDeclaration.border),
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
    { .type = TYPE_STRUCT, .struct_info = &_Clay_Color_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_CornerRadius_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_ImageElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_FloatingElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_CustomElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_ScrollElementConfig_Info },
    { .type = TYPE_STRUCT, .struct_info = &_Clay_BorderElementConfig_Info },
    { .type = TYPE_INTEGRAL },
};
DEFINE_STRUCT_INFO(Clay_ElementDeclaration);
