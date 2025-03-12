#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clay_enum_names.h"
#include "clay_struct_names.h"
#include "ui_element.h"

void ui_element_unused(void) { (void) CLAY__ELEMENT_DEFINITION_LATCH; }

ui_element_t* ui_element_add(ui_element_t* parent, ui_element_type_t type)
{
    ui_element_t* me = (ui_element_t*) malloc(sizeof(ui_element_t));
    memset(me, 0, sizeof *me);
    me->type = type;
    if (type == UI_ELEMENT_DECLARATION) {
        me->ptr = (Clay_ElementDeclaration*) malloc(sizeof(Clay_ElementDeclaration));
        memset(me->ptr, 0, sizeof(Clay_ElementDeclaration));
        me->children = NULL;
        me->num_children = 0;
    } else {
        me->text_config = (Clay_TextElementConfig*) malloc(sizeof(Clay_TextElementConfig));
        memset(me->text_config, 0, sizeof(Clay_TextElementConfig));
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
    if (me->parent) me->parent->num_children--;
    free(me);
}

