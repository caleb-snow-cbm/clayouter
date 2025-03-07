#ifndef COMPONENTS_CLAY_COMPONENTS_H
#define COMPONENTS_CLAY_COMPONENTS_H

#include <stdbool.h>

#include "clay.h"
#include "types.h"

typedef void (*layout_fn_t)(void*);
typedef void (*on_hover_cb_t)(Clay_ElementId id, Clay_PointerData data, intptr_t user_data);

typedef enum { TT_BODY, TT_HEADER, TT_TITLE } text_types_t;

typedef struct {
    Clay_Color background;
    Clay_Color clickable;
    Clay_Color hovered;
    Clay_Color selected;
    Clay_Color highlight;
    Clay_TextElementConfig* text_types;
    size_t text_types_count;
} theme_t;

typedef struct {
    size_t* selected_tab_ptr;
    size_t index;
} selected_tab_info_t;

typedef struct {
    Clay_String* tab_names;
    layout_fn_t* layout_funcs;
    selected_tab_info_t* selected_tab_infos;
    size_t num_tabs;
    size_t selected_tab;
} tab_page_t;

void cc_set_theme(theme_t t);
const theme_t* cc_get_theme(void);
void cc_button(Clay_String text, on_hover_cb_t hover_cb, intptr_t user_data);
void cc_check_box(bool* value, Clay_String label);
void cc_begin_layout(void);
dstring_t* cc_get_selected_text_box(void);
void cc_text_box_append(dstring_t* text_box, char c);
void cc_selection_item(Clay_String name, const Clay_String* values, size_t count, uint8_t* value_ptr);
void cc_text_box(dstring_t* text, Clay_String label);
void cc_tab_page(tab_page_t* page, void* user_data);
void cc_initialize_tab_page(tab_page_t* page);
void cc_text_box_advance(void);
void cc_open_selection_menu(void);
void cc_selection_menu(Clay_String label, const Clay_String* options, on_hover_cb_t* cbs, intptr_t* user_data, size_t count);
void cc_close_selection_menu(void);

#endif // COMPONENTS_CLAY_COMPONENTS_H
