#include <assert.h>
#include <string.h>

#include "clay.h"
#include "clay_components.h"
#include "types.h"
#include "clay_renderer_raylib.h"

#define DEFAULT_CORNER_RADIUS CLAY_CORNER_RADIUS(8)

typedef struct {
    size_t capacity;
    size_t size;
    dstring_t** text_boxes;
} text_box_tab_list_t;

typedef struct {
    on_hover_cb_t cb;
    intptr_t user_data;
} sm_item_t;

typedef struct {
    size_t capacity;
    sm_item_t* items;
} sm_items_t;

static dstring_t* selected_text_box;
static uint8_t* selection_menu_item;
static text_box_tab_list_t text_boxes;
static sm_items_t sm_items;
static bool sm_visible;

static Clay_TextElementConfig default_text_types[] = {
    [TT_BODY] = {
        .textColor = (Clay_Color) { 255, 255, 255, 255 },
        .fontId = 0,
        .fontSize = 20,
    },
    [TT_HEADER] = {
        .textColor = (Clay_Color) { 255, 255, 255, 255 },
        .fontId = 1,
        .fontSize = 26,
    },
    [TT_TITLE] = {
        .textColor = (Clay_Color) { 255, 255, 255, 255 },
        .fontId = 2,
        .fontSize = 32,
    }
};

static theme_t theme = {
    .background = (Clay_Color) {  22,  22,  22, 255 },
    .clickable  = (Clay_Color) {  55,  55,  55, 255 },
    .hovered    = (Clay_Color) {  55,  55, 150, 255 },
    .selected   = (Clay_Color) {  55, 100, 200, 255 },
    .highlight  = (Clay_Color) { 255, 255, 255, 255 },
    .text_types = default_text_types,
    .text_types_count = sizeof(default_text_types) / sizeof(*default_text_types),
};

/**********************
 * On hover callbacks *
 **********************/

static void toggle_bool(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        *(bool*) user_data = !(*(bool*) user_data);
    }
}

static void select_tab(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_tab_info_t* info = (selected_tab_info_t*) user_data;
        *info->selected_tab_ptr = info->index;
    }
}

static void select_text_box(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_text_box = (dstring_t*) user_data;
    }
}

static void open_mini_selection_menu(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selection_menu_item = (uint8_t*) user_data;
    }
}

static void mini_selection_menu_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    assert(selection_menu_item);
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        *selection_menu_item = (uint8_t) user_data;
        selection_menu_item = NULL;
    }
}

static void selection_menu_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        sm_item_t info = sm_items.items[user_data];
        sm_visible = false;
        info.cb(id, data, info.user_data);
    }
}

static void selection_menu_close_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        sm_visible = false;
    }
}

/********************
 * Public functions *
 ********************/

void cc_begin_layout(void) { text_boxes.size = 0; }

void cc_set_theme(theme_t t) { theme = t; }

const theme_t* cc_get_theme(void) { return &theme; }

/**************
 * Components *
 **************/

void cc_button(Clay_String text, on_hover_cb_t hover_cb, intptr_t user_data)
{
    CLAY({
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0) },
            .padding = CLAY_PADDING_ALL(4) },
        .backgroundColor = Clay_Hovered() ? theme.hovered : theme.clickable,
        .cornerRadius = CLAY_CORNER_RADIUS(4),
    })
    {
        Clay_OnHover(hover_cb, user_data);
        CLAY_TEXT(text, &theme.text_types[TT_BODY]);
    }
}

void cc_text_box(dstring_t* text, Clay_String label)
{
    if (text_boxes.size == text_boxes.capacity) {
        if (text_boxes.capacity == 0) {
            text_boxes.capacity = 4;
        }
        text_boxes.capacity *= 2;
        text_boxes.text_boxes = realloc(text_boxes.text_boxes, sizeof(void*) * text_boxes.capacity);
    }
    text_boxes.text_boxes[text_boxes.size++] = text;
    bool selected = text == selected_text_box;
    CLAY({
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(50) },
            .padding = CLAY_PADDING_ALL(4),
            .childGap = 4,
            .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .backgroundColor = theme.background,
    })
    {
        CLAY_TEXT(label, &theme.text_types[TT_BODY]);
        const Clay_BorderElementConfig border
            = { .color = theme.selected, .width = CLAY_BORDER_OUTSIDE(1) };
        const Clay_BorderElementConfig no_border
            = { .color = theme.selected, .width = CLAY_BORDER_OUTSIDE(0) };
        CLAY({
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(32) },
                .padding = CLAY_PADDING_ALL(4) },
            .backgroundColor = theme.clickable,
            .border = selected ? border : no_border,
        })
        {
            Clay_OnHover(select_text_box, (intptr_t) text);
            CLAY_TEXT(text->s, &theme.text_types[TT_BODY]);
        }
    }
}

void cc_text_box_advance(void)
{
    // find currently selceted textbox
    for (size_t i = 0; i < text_boxes.size; ++i) {
        if (text_boxes.text_boxes[i] == selected_text_box) {
            selected_text_box = text_boxes.text_boxes[(i + 1) % text_boxes.size];
            break;
        }
    }
}

void cc_text_box_append(dstring_t* text, char c)
{
    if (c == '\b') {
        if (text->s.length) {
            text->s.length--;
            ((char*) text->s.chars)[text->s.length] = '\0';
        }
        return;
    }
    if (text->s.length == text->capacity) {
        if (text->capacity == 0) {
            text->capacity = 16;
        } else {
            text->capacity *= 2;
        }
        text->s.chars = realloc((char*) text->s.chars, text->capacity);
        memset((char*) &text->s.chars[text->s.length], 0, text->capacity - text->s.length);
    }
    ((char*) text->s.chars)[text->s.length++] = c;
}

dstring_t* cc_get_selected_text_box(void) { return selected_text_box; }

static void selection_menu(const Clay_String* options, size_t count)
{
    CLAY({ .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .cornerRadius = DEFAULT_CORNER_RADIUS,
        .floating = { .attachTo = CLAY_ATTACH_TO_PARENT },
        .scroll.vertical = true,
        .border = { .color = theme.highlight, .width = CLAY_BORDER_ALL(1) } })
    {
        for (size_t i = 0; i < count; i++) {
            cc_button(options[i], mini_selection_menu_callback, (intptr_t) i);
        }
    }
}

void cc_selection_item(Clay_String name, const Clay_String* values, size_t count, uint8_t* value_ptr)
{
    CLAY({ .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM } })
    {
        CLAY_TEXT(name, &theme.text_types[TT_BODY]);
        cc_button(values[*value_ptr], open_mini_selection_menu, (intptr_t) value_ptr);
        if (selection_menu_item == value_ptr) {
            selection_menu(values, count);
        }
    }
}

void cc_tab_page(tab_page_t* page, void* user_data)
{
    assert(page);
    assert(page->num_tabs > page->selected_tab);
    CLAY({
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
            .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .cornerRadius = DEFAULT_CORNER_RADIUS,
    })
    {
        CLAY({ .layout = {
                       .sizing = { .width = CLAY_SIZING_GROW(0) },
                       .childGap = 4,
                   },
                .backgroundColor = theme.background })
        {
            for (size_t i = 0; i < page->num_tabs; ++i) {
                bool selected = i == page->selected_tab;
                CLAY({
                    .layout = { .sizing = { .width = CLAY_SIZING_FIT(30) },
                        .padding = CLAY_PADDING_ALL(4),
                        .childGap = 4 },
                    .backgroundColor = selected ? theme.selected
                        : Clay_Hovered()        ? theme.hovered
                                                : theme.clickable,
                    .cornerRadius = { .topLeft = 8, .topRight = 8 },
                    .border = { .width = { .bottom = 1 } },
                })
                {
                    Clay_OnHover(select_tab, (intptr_t) &page->selected_tab_infos[i]);
                    CLAY_TEXT(page->tab_names[i], &theme.text_types[TT_BODY]);
                }
            }
        }
        CLAY(
            { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                  .padding = CLAY_PADDING_ALL(4),
                  .childGap = 4,
                  .layoutDirection = CLAY_TOP_TO_BOTTOM },
                .scroll.vertical = true,
                .border = { .color = theme.highlight, .width = CLAY_BORDER_OUTSIDE(1) } })
        {
            page->layout_funcs[page->selected_tab](user_data);
        }
    }
}

void cc_initialize_tab_page(tab_page_t* page)
{
    for (size_t i = 0; i < page->num_tabs; ++i) {
        page->selected_tab_infos[i].selected_tab_ptr = &page->selected_tab;
        page->selected_tab_infos[i].index = i;
    }
}

void cc_check_box(bool* value, Clay_String label)
{
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } })
    {
        CLAY_TEXT(label, &theme.text_types[TT_BODY]);
        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) { }
        // TODO: Dynamic checkbox size?
        CLAY({ .layout
            = { .sizing = { .width = CLAY_SIZING_FIXED(24), .height = CLAY_SIZING_FIXED(24) },
                .padding = CLAY_PADDING_ALL(4) },
            .backgroundColor = Clay_Hovered() && !(*value) ? theme.hovered : (Clay_Color) { 0 },
            .cornerRadius = CLAY_CORNER_RADIUS(4),
            .border = { .color = theme.highlight, .width = CLAY_BORDER_OUTSIDE(1) } })
        {
            Clay_OnHover(toggle_bool, (intptr_t) value);
            CLAY({
                .layout
                = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } },
                .backgroundColor = *value ? theme.selected : (Clay_Color) { 0 },
                .cornerRadius = CLAY_CORNER_RADIUS(4),
            });
        }
    }
}

void cc_open_selection_menu(void)
{
    sm_visible = true; // only supports 1 at a time...
}

void cc_selection_menu(Clay_String label,
                       const Clay_String* options,
                       on_hover_cb_t* cbs,
                       intptr_t* user_data,
                       size_t count)
{
    if (!sm_visible)
        return;
    CLAY({
        .id = CLAY_ID("selection_menu"),
        .layout = { .padding = CLAY_PADDING_ALL(8), .childGap = 8, .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .backgroundColor = theme.background,
        .cornerRadius = DEFAULT_CORNER_RADIUS,
        .floating = {
            .attachPoints = {
                .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                .parent = CLAY_ATTACH_POINT_CENTER_CENTER,
            },
            .attachTo = CLAY_ATTACH_TO_ROOT,
        },
        .border = { .color = theme.highlight, .width = CLAY_BORDER_OUTSIDE(1) }
    })
    {
        CLAY_TEXT(label, &theme.text_types[TT_TITLE]);
        CLAY({
            .layout = { .childGap = 8,
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM, },
            .scroll = { .vertical = true, }
        })
        {
            for (size_t i = 0; i < count; ++i) {
                if (i == sm_items.capacity) {
                    sm_items.capacity += 16;
                    sm_items.items = realloc(sm_items.items, sizeof(sm_item_t) * sm_items.capacity);
                }
                sm_items.items[i].cb = cbs[i];
                sm_items.items[i].user_data = user_data[i];
                cc_button(options[i], selection_menu_callback, i);
            }
        }
        cc_button(CLAY_STRING("Close"), selection_menu_close_callback, 0);
    }
}

void cc_close_selection_menu(void)
{
    sm_visible = false;
}

void cc_color_picker(uint32_t parent_id, Clay_ImageElementConfig im, color_string_t* color)
{
    CLAY({ .id = CLAY_ID("Color picker"),
        .layout = { .sizing = { .width = CLAY_SIZING_FIT(400), .height = CLAY_SIZING_FIT(400) } },
        .backgroundColor = theme.background,
        .floating = { .parentId = parent_id, .attachTo = CLAY_ATTACH_TO_ELEMENT_WITH_ID } })
    {
        CLAY({ .image = im }) {
            
        }
        CLAY({
            .layout = { .sizing = { .height = CLAY_SIZING_GROW(0) },
                .padding = CLAY_PADDING_ALL(8),
                .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER } },
        }) {
            CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(10), .height = CLAY_SIZING_GROW(0) },
                               .childAlignment = { .x = CLAY_ALIGN_X_CENTER, .y = CLAY_ALIGN_Y_CENTER } },
                   .backgroundColor = theme.clickable,
            }) {
                CLAY({
                    .layout = { .sizing
                        = { .width = CLAY_SIZING_FIXED(20), .height = CLAY_SIZING_FIXED(20) } },
                    .backgroundColor = Clay_Hovered() ? theme.hovered : theme.clickable,
                    .cornerRadius = CLAY_CORNER_RADIUS(10),
                })
                {
                }
            }
        }
    }
}
