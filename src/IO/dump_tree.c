#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "clay.h"
#include "ui_element.h"
#include "clay_enum_names.h"

static uint8_t zero[sizeof(Clay_ElementDeclaration)] = { 0 };
#define IS_NON_ZERO(region) memcmp(&region, zero, sizeof(region))

static void dump_clay_layout(FILE* f, Clay_LayoutConfig* d)
{
    fprintf(f, ".layout = { ");
    if (IS_NON_ZERO(d->sizing)) {
        fprintf(f, ".sizing = { ");
        if (IS_NON_ZERO(d->sizing.width)) {
            fprintf(f, ".width = %s(%.3f), ",
                _Clay__SizingType_Extra_Macros[d->sizing.width.type],
                d->sizing.width.size.percent);
        }
        if (IS_NON_ZERO(d->sizing.height)) {
            fprintf(f, ".height = %s(%.3f) ",
                _Clay__SizingType_Extra_Macros[d->sizing.width.type],
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
            fprintf(f, ".x = %s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_LayoutAlignmentX, d->childAlignment.x));
        }
        if (d->childAlignment.y) {
            fprintf(f, ".y = %s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_LayoutAlignmentY, d->childAlignment.y));
        }
        fprintf(f, "}, ");
    }
    if (d->layoutDirection) {
        fprintf(
            f, ".layoutDirection = %s, ", _Clay_LayoutDirection_Macros[d->layoutDirection]);
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
            fprintf(f, ".element = %s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachPointType, d->attachPoints.element));
        }
        if (d->attachPoints.parent) {
            fprintf(f, ".element = %s, ",
                CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachPointType, d->attachPoints.parent));
        }
        fprintf(f, "}, ");
    }
    if (d->pointerCaptureMode) {
        fprintf(f, ".pointerCaptureMode = %s, ",
            CLAY_ENUM_VALUE_MACRO(Clay_PointerCaptureMode, d->pointerCaptureMode));
    }
    if (d->attachTo) {
        fprintf(f, ".attachTo = %s",
            CLAY_ENUM_VALUE_MACRO(Clay_FloatingAttachToElement, d->attachTo));
    }
    fprintf(f, "}, ");
}

void dump_clay_border(FILE* f, Clay_BorderElementConfig* d)
{
    fprintf(f, ".border = { ");
    if (IS_NON_ZERO(d->color)) {
        fprintf(f, ".color = (Clay_Color) { %.0f, %.0f, %.0f, %.0f }, ",
            d->color.r, d->color.g, d->color.b, d->color.a);
    }
    if (IS_NON_ZERO(d->width)) {
        fprintf(f, ".width = ");
        if (d->width.left == d->width.right &&
            d->width.top == d->width.bottom &&
            d->width.left == d->width.top) {
            if (d->width.betweenChildren == d->width.left) {
                fprintf(f, "CLAY_BORDER_ALL(%hu), ", d->width.left);
            } else if (d->width.betweenChildren == 0) {
                fprintf(f, "CLAY_BORDER_OUTSIDE(%hu), ", d->width.left);
            } else goto print_all;
        } else {
print_all:
            fprintf(f, "{ .left = %hu, .right = %hu, .top = %hu, .bottom = %hu, .betweenChildren = %hu }, ",
                d->width.left,
                d->width.right,
                d->width.top,
                d->width.bottom,
                d->width.betweenChildren);
        }
    }
    fprintf(f, "}, ");
}

static void dump_clay_color(FILE* f, on_hover_config_t* oh)
{
    fprintf(f, ".backgroundColor = ");
    if (oh->enabled) {
        fprintf(f, "Clay_Hovered() ? (Clay_Color) { %.0f, %.0f, %.0f, %.0f } : ",
            oh->hovered_color.r, oh->hovered_color.g, oh->hovered_color.b, oh->hovered_color.a);
    }
    fprintf(f, "(Clay_Color) { %.0f, %.0f, %.0f, %.0f }, ",
        oh->non_hovered_color.r, oh->non_hovered_color.g, oh->non_hovered_color.b, oh->non_hovered_color.a);
}

static void dump_clay_declaration(FILE* f, Clay_ElementDeclaration* d, on_hover_config_t* oh)
{
    if (d->id.stringId.length) {
        fprintf(f, ".id = CLAY_ID(\"%.*s\"), ", d->id.stringId.length, d->id.stringId.chars);
    }
    if (IS_NON_ZERO(d->layout)) {
        dump_clay_layout(f, &d->layout);
    }
    if (IS_NON_ZERO(d->backgroundColor) || IS_NON_ZERO(oh->hovered_color)) {
        dump_clay_color(f, oh);
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

    if (IS_NON_ZERO(d->border.width)) {
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

void dump_tree_r(FILE* f, ui_element_t* root, int depth)
{
    if (root->type == UI_ELEMENT_DECLARATION) {
        fprintf(f, "CLAY({");
        dump_clay_declaration(f, root->ptr, &root->on_hover);
        fprintf(f, "}) {\n");
        if (root->on_hover.enabled) {
            fprintf(f, "Clay_OnHover(%.*s, 0); // TODO: implement\n", root->on_hover.callback.length, root->on_hover.callback.chars);
        }
        for (size_t i = 0; i < root->num_children; ++i) {
            dump_tree_r(f, root->children[i], depth + 1);
        }
        fprintf(f, "}\n");
    } else if (root->type == UI_ELEMENT_TEXT) {
        dump_clay_text(f, root->text.s, root->text_config);
    }
}

void dump_tree(const char* filename, ui_element_t* root, int depth)
{
    FILE* f = fopen(filename, "w");
    dump_tree_r(f, root, depth);
    fclose(f);
}
