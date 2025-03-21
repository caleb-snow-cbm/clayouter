#ifndef CLAY_COMPONENTS_H
#define CLAY_COMPONENTS_H

#include <stdbool.h>

#include "clay.h"

typedef void (*layout_fn_t)(void*);
typedef void (*on_hover_cb_t)(Clay_ElementId id, Clay_PointerData data, intptr_t user_data);

typedef enum { TT_BODY, TT_HEADER, TT_TITLE } text_types_t;

typedef struct {
    int32_t capacity;
    Clay_String s;
} dstring_t;

typedef struct {
    dstring_t r;
    dstring_t g;
    dstring_t b;
    dstring_t a;
} color_string_t;

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

typedef struct {
    Clay_String label;
    Clay_String* options;
    on_hover_cb_t* cbs;
    intptr_t* user_data;
    size_t count;
    bool visible;
} cc_selection_menu_t;

void cc_set_theme(theme_t t);
const theme_t* cc_get_theme(void);
void cc_button(Clay_String text, on_hover_cb_t hover_cb, intptr_t user_data);
void cc_check_box(bool* value, Clay_String label);
void cc_begin_layout(void);
void cc_free(void);

dstring_t* cc_get_selected_text_box(void);
void cc_text_box_append(dstring_t* text_box, char c);
void cc_selection_item(Clay_String name, const Clay_String* values, size_t count, uint8_t* value_ptr);
void cc_text_box(dstring_t* text, Clay_String label);
void cc_tab_page(tab_page_t* page, void* user_data);
void cc_initialize_tab_page(tab_page_t* page);
void cc_text_box_advance(void);
void cc_selection_menu(cc_selection_menu_t* menu);
void cc_color_selector(Clay_ImageElementConfig im, color_string_t* color);
Clay_Color cc_parse_color(const color_string_t* c);

#endif // CLAY_COMPONENTS_H
