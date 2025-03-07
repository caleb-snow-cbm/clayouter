#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <stdio.h>
#include <stddef.h>

#include "clay.h"
#include "types.h"

typedef enum {
    UI_ELEMENT_DECLARATION,
    UI_ELEMENT_TEXT,
} ui_element_type_t;

typedef struct {
    bool enabled;
    Clay_String callback;
    Clay_Color hovered_color;
    Clay_Color non_hovered_color;
} on_hover_config_t;

typedef struct ui_element_s {
    struct ui_element_s* parent;
    union {
        struct {
            Clay_ElementDeclaration* ptr;
            struct ui_element_s** children;
            size_t num_children;
            on_hover_config_t on_hover;
        };
        struct {
            dstring_t text;
            Clay_TextElementConfig* text_config;
        };
    };
    ui_element_type_t type;
} ui_element_t;

ui_element_t* ui_element_add(ui_element_t* parent, ui_element_type_t type);
void ui_element_remove(ui_element_t* me);
void dump_tree(FILE* f, ui_element_t* root, int depth);


#endif // UI_ELEMENT_H
