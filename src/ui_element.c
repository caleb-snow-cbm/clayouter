#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clay_enum_names.h"
#include "clay_struct_names.h"
#include "ui_element.h"
#include "utilities.h"

static ui_element_t* ui_element_add(ui_element_t* parent, ui_element_type_t type)
{
    ui_element_t* me = (ui_element_t*) malloc_assert(sizeof(ui_element_t));
    memset(me, 0, sizeof *me);
    me->type = type;
    if (type == UI_ELEMENT_DECLARATION) {
        me->ptr = (Clay_ElementDeclaration*) malloc_assert(sizeof(Clay_ElementDeclaration));
        memset(me->ptr, 0, sizeof(Clay_ElementDeclaration));
        me->ptr->backgroundColor.a = 255.0f;
        me->children = NULL;
        me->num_children = 0;
    } else {
        me->text_config = (Clay_TextElementConfig*) malloc_assert(sizeof(Clay_TextElementConfig));
        memset(me->text_config, 0, sizeof(Clay_TextElementConfig));
        me->text_config->textColor.a = 255.0f;
    }
    me->parent = parent;
    return me;
}

ui_element_t* ui_element_insert_before(ui_element_t* parent, ui_element_t* pos, ui_element_type_t type)
{
    ui_element_t* me = ui_element_add(parent, type);
    if (parent) {
        parent->num_children++;
        REALLOC_ASSERT(parent->children, sizeof(ui_element_t*) * parent->num_children);
        if (pos == NULL) {
            parent->children[parent->num_children - 1] = me;
        } else for (int i = (int) parent->num_children - 2; i >= 0; --i) {
            parent->children[i + 1] = parent->children[i];
            if (parent->children[i] == pos) {
                parent->children[i] = me;
                break;
            }
        }
    }
    return me;
}

ui_element_t* ui_element_insert_after(ui_element_t* parent, ui_element_t* pos, ui_element_type_t type)
{
    ui_element_t* me = ui_element_add(parent, type);
    if (parent) {
        parent->num_children++;
        REALLOC_ASSERT(parent->children, sizeof(ui_element_t*) * parent->num_children);
        int i;
        for (i = (int) parent->num_children - 2; i >= 0; --i) {
            if (parent->children[i] == pos) {
                break;
            }
            parent->children[i + 1] = parent->children[i];
        }
        parent->children[i + 1] = me;
    }
    return me;
}

void ui_element_remove(ui_element_t* me)
{
    if (me == NULL) return;
    if (me->type == UI_ELEMENT_DECLARATION) {
        free((char*) me->ptr->id.stringId.chars);
        free(me->ptr);
        free((char*) me->on_hover.callback.chars);
        size_t num_children = me->num_children;
        for (size_t i = 0; i < num_children; ++i) {
            // Child will shift other children back when
            // deleting self, so always remove head
            ui_element_remove(me->children[0]);
        }
        assert(me->num_children == 0);
        free(me->children);
    } else if (me->type == UI_ELEMENT_TEXT) {
        free((char*) me->text.s.chars);
        free(me->text_config);
    }
    if (me->parent) {
        for (size_t i = 0; i < me->parent->num_children; ++i) {
            if (me->parent->children[i] == me) {
                for (size_t j = i + 1; j < me->parent->num_children; ++j) {
                    me->parent->children[j - 1] = me->parent->children[j];
                }
                break;
            }
        }
        me->parent->num_children--;
    }
    free(me);
}

