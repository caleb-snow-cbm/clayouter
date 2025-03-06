#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ui_element.h"

void ui_element_unused(void)
{
    (void) CLAY__ELEMENT_DEFINITION_LATCH;
}

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
        free((char*)me->text.s.chars);
        free(me->text_config);
    }
    me->parent->num_children--;
    free(me);
}

static uint8_t zero[sizeof(Clay_ElementDeclaration)] = { 0 };
#define IS_NON_ZERO(region) memcmp(&region, zero, sizeof(region))

const char* clay_sizing_macros[] = {
    "FIT",
    "GROW",
    "PERCENT",
    "FIXED",
};

const char* clay_x_align_macros[] = {
    "LEFT",
    "RIGHT",
    "CENTER",
};

const char* clay_y_align_macros[] = {
    "TOP",
    "BOTTOM",
    "CENTER",
};

const char* clay_layout_direction_macros[] = {
    "LEFT_TO_RIGHT",
    "TOP_TO_BOTTOM",
};

const char* Clay_TextElementConfigWrapMode_macros[] = {
    "WORDS",
    "NEWLINES",
    "NONE"
};

const char* Clay_TextAlignment_macros[] = {
    "LEFT",
    "CENTER",
    "RIGHT"
};

static void dump_clay_layout(FILE* f, Clay_LayoutConfig* d)
{
    fprintf(f, ".layout = { ");
    if (IS_NON_ZERO(d->sizing)) {
        fprintf(f, ".sizing = { ");
        if (IS_NON_ZERO(d->sizing.width)) {
            fprintf(f, ".width = CLAY_SIZING_%s(%.3f), ",
                clay_sizing_macros[d->sizing.width.type],
                d->sizing.width.size.percent);
        }
        if (IS_NON_ZERO(d->sizing.height)) {
            fprintf(f, ".height = CLAY_SIZING_%s(%.3f) ",
                clay_sizing_macros[d->sizing.height.type],
                d->sizing.height.size.percent);
        }
        fprintf(f, "}, ");
    }
    if (IS_NON_ZERO(d->padding)) {
        fprintf(f, ".padding = (Clay_Padding) { %hu, %hu, %hu, %hu }, ",
            d->padding.left,
            d->padding.right,
            d->padding.top,
            d->padding.bottom);
    }
    if (d->childGap) {
        fprintf(f, ".childGap = %hu, ", d->childGap);
    }
    if (IS_NON_ZERO(d->childAlignment)) {
        fprintf(f, ".childAlignment = { ");
        if (d->childAlignment.x) {
            fprintf(f, ".x = CLAY_ALIGN_X_%s, ", clay_x_align_macros[d->childAlignment.x]);
        }
        if (d->childAlignment.y) {
            fprintf(f, ".y = CLAY_ALIGN_Y_%s, ", clay_y_align_macros[d->childAlignment.y]);
        }
        fprintf(f, "}, ");
    }
    if (d->layoutDirection) {
        fprintf(f, ".layoutDirection = CLAY_%s, ", clay_layout_direction_macros[d->layoutDirection]);
    }
    fprintf(f, "}, ");
}

static void dump_clay_color(FILE* f, Clay_Color* c)
{
    fprintf(f, ".backgroundColor = (Clay_Color) { %.0f, %.0f, %.0f, %.0f }, ",
        c->r, c->g, c->b, c->a);
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
}

static void dump_clay_text(FILE* f, Clay_String s, Clay_TextElementConfig* c)
{
    fprintf(f, "CLAY_TEXT(CLAY_STRING(\"%.*s\"), CLAY_TEXT_CONFIG( {", s.length, s.chars);
    fprintf(f, ".textColor = (Clay_Color) { %.0f, %.0f, %.0f, %.0f }, ",
        c->textColor.r, c->textColor.g, c->textColor.b, c->textColor.a);
    if (c->fontId)
        fprintf(f, ".fontId = %hu, ", c->fontId);
    fprintf(f, ".fontSize = %hu, ", c->fontSize);
    if (c->letterSpacing)
        fprintf(f, ".letterSpacing = %hu, ", c->letterSpacing);
    if (c->lineHeight)
        fprintf(f, ".lineHeight = %hu, ", c->lineHeight);
    if (c->wrapMode)
        fprintf(f, ".wrapMode = CLAY_TEXT_WRAP_%s, ", Clay_TextElementConfigWrapMode_macros[c->wrapMode]);
    if (c->textAlignment)
        fprintf(f, ".textAlignment = CLAY_TEXT_ALIGN_%s, ", Clay_TextAlignment_macros[c->textAlignment]);
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
