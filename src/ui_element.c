#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clay_enum_names.h"
#include "ui_element.h"

void ui_element_unused(void) { (void) CLAY__ELEMENT_DEFINITION_LATCH; }

ui_element_t* ui_element_add(ui_element_t* parent, ui_element_type_t type)
{
    ui_element_t* me = (ui_element_t*) malloc(sizeof(ui_element_t));
    me->type = type;
    if (type == UI_ELEMENT_DECLARATION) {
        me->ptr = (Clay_ElementDeclaration*) malloc(sizeof(Clay_ElementDeclaration));
        memset(me->ptr, 0, sizeof(Clay_ElementDeclaration));
        me->children = NULL;
        me->num_children = 0;
    } else {
        me->text_config = (Clay_TextElementConfig*) malloc(sizeof(Clay_TextElementConfig));
        memset(me->text_config, 0, sizeof(Clay_TextElementConfig));
        memset(&me->text, 0, sizeof(me->text));
    }
    me->parent = parent;

    parent->num_children++;
    parent->children = realloc(parent->children, sizeof(ui_element_t*) * parent->num_children);
    parent->children[parent->num_children - 1] = me;

    return me;
}

void ui_element_remove(ui_element_t* me)
{
    if (me->type == UI_ELEMENT_DECLARATION) {
        free(me->ptr);
        size_t num_children = me->num_children;
        for (size_t i = 0; i < num_children; ++i) {
            ui_element_remove(me->children[i]);
        }
        assert(me->num_children == 0);
        free(me->children);
    } else if (me->type == UI_ELEMENT_TEXT) {
        free((char*) me->text.s.chars);
        free(me->text_config);
    }
    me->parent->num_children--;
    free(me);
}

static uint8_t zero[sizeof(Clay_ElementDeclaration)] = { 0 };
#define IS_NON_ZERO(region) memcmp(&region, zero, sizeof(region))

static void dump_clay_layout(FILE* f, Clay_LayoutConfig* d)
{
    fprintf(f, ".layout = { ");
    if (IS_NON_ZERO(d->sizing)) {
        fprintf(f, ".sizing = { ");
        if (IS_NON_ZERO(d->sizing.width)) {
            fprintf(f, ".width = CLAY_SIZING_%s(%.3f), ",
                CLAY_ENUM_VALUE_MACRO(Clay__SizingType, d->sizing.width.type),
                d->sizing.width.size.percent);
        }
        if (IS_NON_ZERO(d->sizing.height)) {
            fprintf(f, ".height = CLAY_SIZING_%s(%.3f) ",
                CLAY_ENUM_VALUE_MACRO(Clay__SizingType, d->sizing.height.type),
                d->sizing.height.size.percent);
        }
        fprintf(f, "}, ");
    }
    if (IS_NON_ZERO(d->padding)) {
        fprintf(f, ".padding = (Clay_Padding) { %hu, %hu, %hu, %hu }, ", d->padding.left,
            d->padding.right, d->padding.top, d->padding.bottom);
    }
    if (d->childGap) {
        fprintf(f, ".childGap = %hu, ", d->childGap);
    }
    if (IS_NON_ZERO(d->childAlignment)) {
        fprintf(f, ".childAlignment = { ");
        if (d->childAlignment.x) {
            fprintf(f, ".x = CLAY_ALIGN_X_%s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_LayoutAlignmentX, d->childAlignment.x));
        }
        if (d->childAlignment.y) {
            fprintf(f, ".y = CLAY_ALIGN_Y_%s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_LayoutAlignmentY, d->childAlignment.y));
        }
        fprintf(f, "}, ");
    }
    if (d->layoutDirection) {
        fprintf(
            f, ".layoutDirection = CLAY_%s, ", _Clay_LayoutDirection_Macros[d->layoutDirection]);
    }
    fprintf(f, "}, ");
}

static void dump_clay_floating(FILE* f, Clay_FloatingElementConfig* d)
{
    fprintf(f, ".floating = { ");
    if (IS_NON_ZERO(d->offset)) {
        fprintf(f, ".offset = {");
        if (d->offset.x) {
            fprintf(f, ".x = %.3f, ", d->offset.x);
        }
        if (d->offset.y) {
            fprintf(f, ".y = %.3f ", d->offset.y);
        }
        fprintf(f, "}, ");
    }
    if (IS_NON_ZERO(d->expand)) {
        fprintf(f, ".expand = { ");
        if (d->expand.width)
            fprintf(f, ".width = %.3f, ", d->expand.width);
        if (d->expand.height)
            fprintf(f, ".height = %.3f ", d->expand.height);
        fprintf(f, "}, ");
    }
    // TODO: parentId
    if (d->zIndex)
        fprintf(f, ".zIndex = %hd, ", d->zIndex);
    if (IS_NON_ZERO(d->attachPoints)) {
        fprintf(f, ".attachPoints = { ");
        if (d->attachPoints.element) {
            fprintf(f, ".element = CLAY_ATTACH_POINT_%s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachPointType, d->attachPoints.element));
        }
        if (d->attachPoints.parent) {
            fprintf(f, ".element = CLAY_ATTACH_POINT_%s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachPointType, d->attachPoints.parent));
        }
        fprintf(f, "}, ");
    }
    if (d->pointerCaptureMode) {
        fprintf(f, ".pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_%s, ",
            CLAY_ENUM_VALUE_MACRO(Clay_PointerCaptureMode, d->pointerCaptureMode));
    }
    if (d->attachTo) {
        fprintf(f, ".attachTo = CLAY_ATTACH_TO_%s",
            CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachToElement, d->attachTo));
    }
    fprintf(f, "}, ");
}

void dump_clay_border(FILE* f, Clay_BorderElementConfig* d)
{

}

static void dump_clay_color(FILE* f, Clay_Color* c)
{
    fprintf(
        f, ".backgroundColor = (Clay_Color) { %.0f, %.0f, %.0f, %.0f }, ", c->r, c->g, c->b, c->a);
}

static void dump_clay_declaration(FILE* f, Clay_ElementDeclaration* d)
{
    if (d->id.stringId.length) {
        fprintf(f, ".id = CLAY_ID(\"%.*s\"), ", d->id.stringId.length, d->id.stringId.chars);
    }
    if (IS_NON_ZERO(d->layout)) {
        dump_clay_layout(f, &d->layout);
    }
    if (IS_NON_ZERO(d->backgroundColor)) {
        dump_clay_color(f, &d->backgroundColor);
    }
    if (IS_NON_ZERO(d->floating)) {
        dump_clay_floating(f, &d->floating);
    }
    // TODO: custom ?
    if (IS_NON_ZERO(d->scroll)) {
        fprintf(f, ".scroll = { ");
        if (d->scroll.horizontal)
            fprintf(f, ".horizontal = true, ");
        if (d->scroll.vertical)
            fprintf(f, ".vertical = true ");
        fprintf(f, "}, ");
    }

    if (IS_NON_ZERO(d->border)) {
        dump_clay_border(f, &d->border);
    }
}

static void dump_clay_text(FILE* f, Clay_String s, Clay_TextElementConfig* c)
{
    fprintf(f, "CLAY_TEXT(CLAY_STRING(\"%.*s\"), CLAY_TEXT_CONFIG({ ", s.length, s.chars);
    fprintf(f, ".textColor = (Clay_Color) { %.0f, %.0f, %.0f, %.0f }, ", c->textColor.r,
        c->textColor.g, c->textColor.b, c->textColor.a);
    if (c->fontId)
        fprintf(f, ".fontId = %hu, ", c->fontId);
    fprintf(f, ".fontSize = %hu, ", c->fontSize);
    if (c->letterSpacing)
        fprintf(f, ".letterSpacing = %hu, ", c->letterSpacing);
    if (c->lineHeight)
        fprintf(f, ".lineHeight = %hu, ", c->lineHeight);
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

void dump_tree(FILE* f, ui_element_t* root, int depth)
{
    if (root->type == UI_ELEMENT_DECLARATION) {
        fprintf(f, "CLAY({");
        dump_clay_declaration(f, root->ptr);
        fprintf(f, "}) {\n");
        for (size_t i = 0; i < root->num_children; ++i) {
            dump_tree(f, root->children[i], depth + 1);
        }
        fprintf(f, "}\n");
    } else if (root->type == UI_ELEMENT_TEXT) {
        dump_clay_text(f, root->text.s, root->text_config);
    }
}
