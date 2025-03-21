#ifndef UI_ELEMENT_H
#define UI_ELEMENT_H

#include <stdio.h>
#include <stddef.h>

#include "clay.h"
#include "components/clay_components.h"

typedef enum {
    UI_ELEMENT_DECLARATION,
    UI_ELEMENT_TEXT,
} ui_element_type_t;

typedef struct {
    bool enabled;
    Clay_String callback;
    Clay_ElementDeclaration* ptr;
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

/**
 * @brief Inserts a UI element into tree before `pos`
 * 
 * @param parent Parent element, can be `NULL`
 * @param pos Child element to place new element before, `NULL` would insert at end
 * @param type Type of new element, declaration or text
 * @return `ui_element_t*` Pointer to inserted element
 */
ui_element_t* ui_element_insert_before(ui_element_t* parent, ui_element_t* pos, ui_element_type_t type);

/**
 * @brief Inserts a UI element into tree after `pos`
 * 
 * @param parent Parent element, can be `NULL`
 * @param pos Child element to place new element after, `NULL` would insert at beginning
 * @param type Type of new element, declaration or text
 * @return `ui_element_t*` Pointer to inserted element
 */
ui_element_t* ui_element_insert_after(ui_element_t* parent, ui_element_t* pos, ui_element_type_t type);
void ui_element_remove(ui_element_t* me);

#endif // UI_ELEMENT_H
