#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "clay.h"
#include "clay_enum_names.h"
#include "export_layout.h"
#include "ui_element.h"

static uint8_t zero[sizeof(Clay_ElementDeclaration)] = { 0 };
#define IS_NON_ZERO(region) memcmp(&(region), zero, sizeof(region))
#define IS_DIFFERENT(field)                                                                        \
    (node->on_hover.enabled && memcmp(&(d->field), &(oh->field), sizeof(d->field)))
#define EXPORT_FIELD(field)                                                                        \
    if (IS_NON_ZERO(d->field) || IS_DIFFERENT(field)) {                                            \
        fprintf(f, "." #field " = ");                                                              \
        if (IS_DIFFERENT(field)) {                                                                 \
            fprintf(f, "Clay_Hovered() ? ");                                                       \
            export_clay_##field(f, oh->field);                                                     \
            fprintf(f, " : ");                                                                     \
        }                                                                                          \
        export_clay_##field(f, d->field);                                                          \
        fprintf(f, ", ");                                                                          \
    }

static void export_clay_sizing(FILE* f, Clay_Sizing s)
{
    fprintf(f, "{ ");
    if (IS_NON_ZERO(s.width)) {
        fprintf(f, ".width = %s(%.3f), ",
            _Clay__SizingType_Extra_Macros[s.width.type],
            s.width.size.percent);
    }
    if (IS_NON_ZERO(s.height)) {
        fprintf(f, ".height = %s(%.3f) ",
            _Clay__SizingType_Extra_Macros[s.height.type],
            s.height.size.percent);
    }
    fprintf(f, "}");
}

static void export_clay_padding(FILE* f, Clay_Padding p)
{
    fprintf(f, "(Clay_Padding) { %" PRIu16 ", %" PRIu16 ", %" PRIu16 ", %" PRIu16 " }", p.left,
        p.right, p.top, p.bottom);
}

static void export_clay_childGap(FILE* f, uint16_t gap)
{
    fprintf(f, "%" PRIu16, gap);
}

static void export_clay_childAlignment(FILE* f, Clay_ChildAlignment a)
{
    fprintf(f, "{ ");
    if (a.x) {
        fprintf(f, ".x = %s, ", CLAY_ENUM_VALUE_MACRO(Clay_LayoutAlignmentX, a.x));
    }
    if (a.y) {
        fprintf(f, ".y = %s, ", CLAY_ENUM_VALUE_MACRO(Clay_LayoutAlignmentY, a.y));
    }
    fprintf(f, "}");
}

static void export_clay_layoutDirection(FILE* f, Clay_LayoutDirection d)
{
    fprintf(f, "%s", _Clay_LayoutDirection_Macros[d]);
}

static void export_clay_layout(FILE* f, ui_element_t* node)
{
    Clay_LayoutConfig* d = &node->ptr->layout;
    Clay_LayoutConfig* oh = &node->on_hover.ptr->layout;
    fprintf(f, ".layout = { ");
    EXPORT_FIELD(sizing);
    EXPORT_FIELD(padding);
    EXPORT_FIELD(childGap);
    EXPORT_FIELD(childAlignment);
    EXPORT_FIELD(layoutDirection);
    fprintf(f, "}, ");
}

static void export_clay_cornerRadius(FILE* f, Clay_CornerRadius r)
{
    if (r.bottomLeft == r.bottomRight &&
        r.topLeft == r.topRight &&
        r.bottomLeft == r.topLeft) {
        fprintf(f, "CLAY_CORNER_RADIUS(%.1f)", r.bottomLeft);
    } else {
        fprintf(f, "(Clay_CornerRadius) { %.1f,  %.1f,  %.1f,  %.1f }",
            r.topLeft, r.topRight, r.bottomLeft, r.bottomRight);
    }
}

static void export_clay_offset(FILE* f, Clay_Vector2 o)
{
    fprintf(f, "{ ");
    if (o.x) {
        fprintf(f, ".x = %.3f, ", o.x);
    }
    if (o.y) {
        fprintf(f, ".y = %.3f ", o.y);
    }
    fprintf(f, "}");
}

static void export_clay_expand(FILE* f, Clay_Dimensions d)
{
    fprintf(f, "{ ");
    if (d.width)
        fprintf(f, ".width = %.3f, ", d.width);
    if (d.height)
        fprintf(f, ".height = %.3f ", d.height);
    fprintf(f, "}");
}

static void export_clay_zIndex(FILE* f, int16_t index)
{
    fprintf(f, "%hd", index);
}

static void export_clay_attachPoints(FILE* f, Clay_FloatingAttachPoints p)
{
    fprintf(f, "{ ");
    if (p.element) {
        fprintf(
            f, ".element = %s, ", CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachPointType, p.element));
    }
    if (p.parent) {
        fprintf(
            f, ".parent = %s, ", CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachPointType, p.parent));
    }
    fprintf(f, "}");
}

static void export_clay_pointerCaptureMode(FILE* f, Clay_PointerCaptureMode m)
{
    fprintf(f, "%s", CLAY_ENUM_VALUE_MACRO(Clay_PointerCaptureMode, m));
}

static void export_clay_attachTo(FILE* f, Clay_FloatingAttachToElement a)
{
    fprintf(f, "%s", CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachToElement, a));
}

static void export_clay_floating(FILE* f, ui_element_t* node)
{
    Clay_FloatingElementConfig* d = &node->ptr->floating;
    Clay_FloatingElementConfig* oh = &node->on_hover.ptr->floating;
    fprintf(f, ".floating = { ");
    EXPORT_FIELD(offset);
    EXPORT_FIELD(expand);
    // TODO: parentId
    EXPORT_FIELD(zIndex);
    EXPORT_FIELD(attachPoints);
    EXPORT_FIELD(pointerCaptureMode);
    EXPORT_FIELD(attachTo);
    fprintf(f, "}, ");
}

static void export_clay_color(FILE* f, Clay_Color c)
{
    fprintf(f, "(Clay_Color) { %.0f, %.0f, %.0f, %.0f }", c.r, c.g, c.b, c.a);
}

static void export_clay_width(FILE* f, Clay_BorderWidth w)
{
    if (w.left == w.right && w.top == w.bottom && w.left == w.top) {
        if (w.betweenChildren == w.left) {
            fprintf(f, "CLAY_BORDER_ALL(%" PRIu16 ")", w.left);
        } else if (w.betweenChildren == 0) {
            fprintf(f, "CLAY_BORDER_OUTSIDE(%" PRIu16 ")", w.left);
        } else
            goto print_all;
    } else {
    print_all:
        fprintf(f,
            "{ .left = %" PRIu16 ", .right = %" PRIu16 ", .top = %" PRIu16 ", .bottom = %" PRIu16
            ", .betweenChildren = %" PRIu16 " }",
            w.left, w.right, w.top, w.bottom, w.betweenChildren);
    }
}

static void export_clay_border(FILE* f, ui_element_t* node)
{
    Clay_BorderElementConfig* d = &node->ptr->border;
    Clay_BorderElementConfig* oh = &node->on_hover.ptr->border;
    fprintf(f, ".border = { ");
    EXPORT_FIELD(color);
    EXPORT_FIELD(width);
    fprintf(f, "}, ");
}

static void export_clay_backgroundColor(FILE* f, Clay_Color c)
{
    fprintf(f, "(Clay_Color) { %.0f, %.0f, %.0f, %.0f }", c.r, c.g, c.b, c.a);
}

static void export_clay_scroll(FILE* f, Clay_ScrollElementConfig s)
{
    fprintf(f, "{ ");
    if (s.horizontal)
        fprintf(f, ".horizontal = true, ");
    if (s.vertical)
        fprintf(f, ".vertical = true ");
    fprintf(f, "}");
}

static void export_clay_declaration(FILE* f, ui_element_t* node)
{
    Clay_ElementDeclaration* d = node->ptr;
    Clay_ElementDeclaration* oh = node->on_hover.ptr;
    if (d->id.stringId.length) {
        fprintf(f, ".id = CLAY_ID(\"%.*s\"), ", d->id.stringId.length, d->id.stringId.chars);
    }
    if (IS_NON_ZERO(d->layout)) {
        export_clay_layout(f, node);
    }
    EXPORT_FIELD(backgroundColor);
    EXPORT_FIELD(cornerRadius);
    if (IS_NON_ZERO(d->floating)) {
        export_clay_floating(f, node);
    }
    // TODO: custom ?
    EXPORT_FIELD(scroll);
    if (IS_NON_ZERO(d->border.width)) {
        export_clay_border(f, node);
    }
}

static void export_clay_text(FILE* f, Clay_String s, Clay_TextElementConfig* c)
{
    fprintf(f, "CLAY_TEXT(CLAY_STRING(\"%.*s\"), CLAY_TEXT_CONFIG({ ", s.length, s.chars);
    fprintf(f, ".textColor = (Clay_Color) { %.0f, %.0f, %.0f, %.0f }, ", c->textColor.r,
        c->textColor.g, c->textColor.b, c->textColor.a);
    if (c->fontId)
        fprintf(f, ".fontId = %" PRIu16 ", ", c->fontId);
    fprintf(f, ".fontSize = %" PRIu16 ", ", c->fontSize);
    if (c->letterSpacing)
        fprintf(f, ".letterSpacing = %" PRIu16 ", ", c->letterSpacing);
    if (c->lineHeight)
        fprintf(f, ".lineHeight = %" PRIu16 ", ", c->lineHeight);
    if (c->wrapMode)
        fprintf(f, ".wrapMode = CLAY_TEXT_WRAP_%s, ",
            CLAY_ENUM_VALUE_MACRO(Clay_TextElementConfigWrapMode, c->wrapMode));
    if (c->textAlignment)
        fprintf(f, ".textAlignment = CLAY_TEXT_ALIGN_%s, ",
            CLAY_ENUM_VALUE_MACRO(Clay_TextAlignment, c->textAlignment));
    if (c->hashStringContents)
        fprintf(f, ".hashStringContents = true, ");
    fprintf(f, "}));\n");
}

static void export_layout_r(FILE* f, ui_element_t* root, int depth)
{
    if (root->type == UI_ELEMENT_DECLARATION) {
        fprintf(f, "CLAY({");
        export_clay_declaration(f, root);
        fprintf(f, "}) {\n");
        if (root->on_hover.enabled) {
            fprintf(f, "Clay_OnHover(%.*s, 0); // TODO: implement\n", root->on_hover.callback.length, root->on_hover.callback.chars);
        }
        for (size_t i = 0; i < root->num_children; ++i) {
            export_layout_r(f, root->children[i], depth + 1);
        }
        fprintf(f, "}\n");
    } else if (root->type == UI_ELEMENT_TEXT) {
        export_clay_text(f, root->text.s, root->text_config);
    }
}

void export_layout(const char* filename, ui_element_t* root)
{
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        fprintf(stderr, "Unable to open output file %s\n", filename);
        return;
    }
    export_layout_r(f, root, 0);
    fclose(f);
}
