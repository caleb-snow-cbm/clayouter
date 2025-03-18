#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "clay.h"
#include "clay_components.h"

#ifdef _WIN32
#define EMPTY    0
#else
#define EMPTY
#endif

#define DEFAULT_CORNER_RADIUS CLAY_CORNER_RADIUS(8)

#define COLOR_PICKER_WIDTH      (300.0f)
#define COLOR_PICKER_HEIGHT     (256.0f)
#define COLOR_PICKER_PADDING    (8.0f)
#define COLOR_PICKER_BULB_MAX   ((-COLOR_PICKER_HEIGHT / 2) + COLOR_PICKER_PADDING)
#define COLOR_PICKER_BULB_MIN   (COLOR_PICKER_HEIGHT / 2 - COLOR_PICKER_PADDING)

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef struct {
    dstring_t** items;
    size_t capacity;
    size_t size;
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
        .textColor = { .r = 255, .g = 255, .b = 255, .a = 255 },
        .fontId = 0,
        .fontSize = 20,
    },
    [TT_HEADER] = {
        .textColor = { .r = 255, .g = 255, .b = 255, .a = 255 },
        .fontId = 1,
        .fontSize = 26,
    },
    [TT_TITLE] = {
        .textColor = { .r = 255, .g = 255, .b = 255, .a = 255 },
        .fontId = 2,
        .fontSize = 32,
    }
};

static theme_t theme = {
    .background = { .r =  22, .g =  22, .b =  22, .a = 255 },
    .clickable  = { .r =  55, .g =  55, .b =  55, .a = 255 },
    .hovered    = { .r =  55, .g =  55, .b = 150, .a = 255 },
    .selected   = { .r =  55, .g = 100, .b = 200, .a = 255 },
    .highlight  = { .r = 255, .g = 255, .b = 255, .a = 255 },
    .text_types = default_text_types,
    .text_types_count = sizeof(default_text_types) / sizeof(*default_text_types),
};

typedef struct {
    float brightness;
    float x;
    float y;
    color_string_t* string;
    Clay_Color color;
    bool visible;
} color_picker_info_t;

typedef struct {
    color_picker_info_t* items;
    size_t capacity;
    size_t size;
} color_picker_infos_t;

static color_picker_infos_t cpi;

static void calculate_color_string(size_t index);

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
        sm_visible = false;
    }
    sm_item_t info = sm_items.items[user_data];
    info.cb(id, data, info.user_data);
}

static void color_picker_set_brightness(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    if (data.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        return;
    }
    Clay_ElementData element = Clay_GetElementData(id);
    float percentage = (float) (data.position.y - element.boundingBox.y) / element.boundingBox.height;
    percentage *= 2;
    percentage -= 1;
    size_t index = (size_t) user_data;
    cpi.items[index].brightness = COLOR_PICKER_BULB_MIN * percentage;
    calculate_color_string(index);
}

static void color_picker_set_color(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) user_data;
    if (data.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        return;
    }
    Clay_ElementData element = Clay_GetElementData(id);
    size_t index = (size_t) user_data;
    cpi.items[index].x = (data.position.x - element.boundingBox.x) / element.boundingBox.width;
    cpi.items[index].y = (data.position.y - element.boundingBox.y) / element.boundingBox.height;
    calculate_color_string(index);
}

/*********************
 * Private functions *
 *********************/

static void load_float(dstring_t* dst, float src)
{
    int required_cap = snprintf(NULL, 0, "%.1f", src) + 1;
    if (dst->capacity < required_cap) {
        dst->capacity = 2 * required_cap;
        void* tmp = realloc((char*) dst->s.chars, dst->capacity);
        assert(tmp);
        dst->s.chars = tmp;
    }
    dst->s.length = sprintf((char*) dst->s.chars, "%.1f", src);
}

static void load_color(size_t index, Clay_Color src)
{
    color_string_t* dst = cpi.items[index].string;
    cpi.items[index].color = src;
    load_float(&dst->r, src.r);
    load_float(&dst->g, src.g);
    load_float(&dst->b, src.b);
}

static void calculate_color_string(size_t index)
{
    float brightness = 0.5f - (cpi.items[index].brightness / COLOR_PICKER_HEIGHT);
    Clay_Color c_out = (Clay_Color) { 0, 0, 0, 255 };
    float *min, *mid, *max;
    float offset = 0;
    float sector = 0;
    float sign = 1.0f;
    float x = cpi.items[index].x;
    float y = cpi.items[index].y;
    if (x < 1.0f / 6.0f) {
        max = &c_out.r;
        mid = &c_out.g;
        min = &c_out.b;
    } else if (x < 2.0f / 6.0f) {
        max = &c_out.g;
        mid = &c_out.r;
        min = &c_out.b;
        sector = 1.0f;
        offset = 255.0f;
        sign = -sign;
    } else if (x < 3.0f / 6.0f) {
        max = &c_out.g;
        mid = &c_out.b;
        min = &c_out.r;
        sector = 2.0f;
    } else if (x < 4.0f / 6.0f) {
        max = &c_out.b;
        mid = &c_out.g;
        min = &c_out.r;
        sector = 3.0f;
        offset = 255.0f;
        sign = -sign;
    } else if (x < 5.0f / 6.0f) {
        max = &c_out.b;
        mid = &c_out.r;
        min = &c_out.g;
        sector = 4.0f;
    } else {
        max = &c_out.r;
        mid = &c_out.b;
        min = &c_out.g;
        sector = 5.0f;
        offset = 255.0f;
        sign = -sign;
    }
    *max = brightness * 255.0f;
    float mid_y_intercept = sign * 255.0f * (x - sector / 6.0f) * 6.0f + offset;
    float mid_slope = (255.0f - mid_y_intercept);
    *mid = brightness * (mid_slope * y + mid_y_intercept);
    *min = brightness * (y * 255.0f);
    load_color(index, c_out);
}

static void calculate_color_offsets(size_t index)
{
    const size_t min = 0;
    const size_t mid = 1;
    const size_t max = 2;
    Clay_Color c = cc_parse_color(cpi.items[index].string);
    cpi.items[index].color = c;
    float colors[3] = { c.r, c.g, c.b };
    // 3 element sort
    if (colors[min] > colors[mid]) { float temp = colors[min]; colors[min] = colors[mid]; colors[mid] = temp; }
    if (colors[mid] > colors[max]) { float temp = colors[mid]; colors[mid] = colors[max]; colors[max] = temp; }
    if (colors[min] > colors[mid]) { float temp = colors[min]; colors[min] = colors[mid]; colors[mid] = temp; }
    float brightness = colors[max] / 255.0f;
    cpi.items[index].brightness = -(brightness - 0.5f) * COLOR_PICKER_HEIGHT;
    if (brightness == 0.0f) {
        cpi.items[index].x = 0.0f;
        cpi.items[index].y = 1.0f;
        return;
    }
    c.r /= brightness;
    c.g /= brightness;
    c.b /= brightness;
    colors[min] /= brightness;
    colors[mid] /= brightness;
    colors[max] /= brightness;
    cpi.items[index].y = colors[min] / 255.0f;
    if (cpi.items[index].y == 1) {
        cpi.items[index].x = 0.0f;
        return;
    }
    float x_offset = (255.0f - (255.0f - colors[mid]) / (1 - cpi.items[index].y)) / (255.0f * 6.0f);
    if (c.r == colors[max]) {
        if (c.g == colors[mid]) {
            cpi.items[index].x = x_offset;
        } else {
            cpi.items[index].x = 1 - x_offset;
        }
    } else if (c.g == colors[max]) {
        if (c.r == colors[mid]) {
            cpi.items[index].x = (2.0f / 6.0f) - x_offset;
        } else {
            cpi.items[index].x = (2.0f / 6.0f) + x_offset;
        }
    } else {
        if (c.g == colors[mid]) {
            cpi.items[index].x = (4.0f / 6.0f) - x_offset;
        } else {
            cpi.items[index].x = (4.0f / 6.0f) + x_offset;
        }
    }
}

static void clamp_color(float* c)
{
    if (*c > 255.0f) {
        *c = 255.0f;
    } else if (*c < 0) {
        *c = 0;
    }
}

/********************
 * Public functions *
 ********************/

void cc_begin_layout(void) {
    text_boxes.size = 0;
    cpi.size = 0;
}

void cc_free(void)
{
    free(text_boxes.items);
    free(cpi.items);
}

void cc_set_theme(theme_t t) { theme = t; }

const theme_t* cc_get_theme(void) { return &theme; }

Clay_Color cc_parse_color(const color_string_t* c)
{
    Clay_Color ret = { 0, 0, 0, 255 };
    if (c->r.s.length) {
        ret.r = strtof(c->r.s.chars, NULL);
        clamp_color(&ret.r);
    }
    if (c->g.s.length) {
        ret.g = strtof(c->g.s.chars, NULL);
        clamp_color(&ret.g);
    }
    if (c->b.s.length) {
        ret.b = strtof(c->b.s.chars, NULL);
        clamp_color(&ret.a);
    }
    if (c->a.s.length) {
        ret.a = strtof(c->a.s.chars, NULL);
        clamp_color(&ret.a);
    }
    return ret;
}

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
        text_boxes.items = realloc(text_boxes.items, sizeof(void*) * text_boxes.capacity);
        assert(text_boxes.items);
    }
    text_boxes.items[text_boxes.size++] = text;
    bool selected = text == selected_text_box;
    CLAY({
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(60) },
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
    // find currently selected textbox
    for (size_t i = 0; i < text_boxes.size; ++i) {
        if (text_boxes.items[i] == selected_text_box) {
            selected_text_box = text_boxes.items[(i + 1) % text_boxes.size];
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
        text->s.chars = realloc((char*) text->s.chars, (size_t) text->capacity);
        memset((char*) &text->s.chars[text->s.length], 0, (size_t) (text->capacity - text->s.length));
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

void cc_selection_menu(cc_selection_menu_t* menu)
{
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
        CLAY_TEXT(menu->label, &theme.text_types[TT_TITLE]);
        CLAY({
            .layout = { .childGap = 8,
                        .childAlignment = { .x = CLAY_ALIGN_X_CENTER },
                        .layoutDirection = CLAY_TOP_TO_BOTTOM, },
            .scroll = { .vertical = true, }
        })
        {
            for (size_t i = 0; i < menu->count; ++i) {
                if (i == sm_items.capacity) {
                    sm_items.capacity += 16;
                    sm_items.items = realloc(sm_items.items, sizeof(sm_item_t) * sm_items.capacity);
                }
                sm_items.items[i].cb = menu->cbs[i];
                sm_items.items[i].user_data = menu->user_data[i];
                cc_button(menu->options[i], selection_menu_callback, (intptr_t) i);
            }
        }
        cc_button(CLAY_STRING("Close"), toggle_bool, (intptr_t) &menu->visible);
    }
}

void cc_color_picker(Clay_ImageElementConfig im, size_t index)
{
    CLAY({
        .id = CLAY_IDI("color_picker", index),
        .layout = { .sizing = { .width = CLAY_SIZING_FIT(COLOR_PICKER_WIDTH),
                                .height = CLAY_SIZING_FIT(COLOR_PICKER_HEIGHT) },
                    .padding = CLAY_PADDING_ALL(8),
                    .childGap = 8,
                    .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .backgroundColor = theme.background,
        .cornerRadius = DEFAULT_CORNER_RADIUS,
        .floating = { .attachPoints = { .element = CLAY_ATTACH_POINT_CENTER_CENTER, .parent = CLAY_ATTACH_POINT_CENTER_CENTER},
                      .attachTo = CLAY_ATTACH_TO_ROOT },
        .border = { .color = theme.highlight, .width = CLAY_BORDER_OUTSIDE(1) }})
    {
        CLAY({ EMPTY }) {
            CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_FIT(im.sourceDimensions.width),
                                        .height = CLAY_SIZING_FIT(im.sourceDimensions.height) } },
                .image = im }) {
                // picker
                Clay_OnHover(color_picker_set_color, (intptr_t) index);
                CLAY({
                    .layout = { .sizing = { .width = CLAY_SIZING_FIXED(20), .height = CLAY_SIZING_FIXED(20) },
                                .padding = CLAY_PADDING_ALL(8) },
                    .cornerRadius = CLAY_CORNER_RADIUS(10),
                    .border = { .color = (Clay_Color) {0, 0, 0, 255}, .width = CLAY_BORDER_OUTSIDE(1) },
                    .floating = {
                        .offset = { .x = im.sourceDimensions.width * cpi.items[index].x, .y = im.sourceDimensions.height * cpi.items[index].y },
                        .attachPoints = { .element = CLAY_ATTACH_POINT_CENTER_CENTER },
                        .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                        .attachTo = CLAY_ATTACH_TO_PARENT,
                    },
                });
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
                    Clay_OnHover(color_picker_set_brightness, (intptr_t) index);
                    CLAY({
                        .layout = { .sizing
                            = { .width = CLAY_SIZING_FIXED(20), .height = CLAY_SIZING_FIXED(20) } },
                        .backgroundColor = Clay_Hovered() ? theme.selected : theme.hovered,
                        .cornerRadius = CLAY_CORNER_RADIUS(10),
                        .floating = { .offset = { .y = cpi.items[index].brightness },
                                    .attachPoints = { .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                                                        .parent = CLAY_ATTACH_POINT_CENTER_CENTER },
                                    .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                                    .attachTo = CLAY_ATTACH_TO_PARENT
                        }
                    });
                }
            }
        }
        cc_button(CLAY_STRING("Close"), toggle_bool, (intptr_t) &cpi.items[index].visible);
    }
}

void cc_color_selector(Clay_ImageElementConfig im, color_string_t* color)
{
    if (cpi.size == cpi.capacity) {
        if (cpi.capacity == 0) {
            cpi.capacity = 8;
        }
        cpi.capacity *= 2;
        cpi.items = realloc(cpi.items, sizeof(*cpi.items) * cpi.capacity);
        memset(&cpi.items[cpi.size], 0, sizeof(*cpi.items) * (cpi.capacity - cpi.size));
    }
    cpi.items[cpi.size].string = color;

    CLAY({ .id = CLAY_IDI("color_selector", cpi.size) })
    {
        cc_text_box(&color->r, CLAY_STRING("R"));
        cc_text_box(&color->g, CLAY_STRING("G"));
        cc_text_box(&color->b, CLAY_STRING("B"));
        cc_text_box(&color->a, CLAY_STRING("A"));
        CLAY({
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(50) },
                .padding = CLAY_PADDING_ALL(4),
                .childGap = 4,
                .layoutDirection = CLAY_TOP_TO_BOTTOM },
            .backgroundColor = theme.background,
        })
        {
            CLAY_TEXT(CLAY_STRING("  "), &theme.text_types[TT_BODY]);
            CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_FIXED(32), .height = CLAY_SIZING_FIXED(32) } },
                .backgroundColor = (Clay_Color) { cpi.items[cpi.size].color.r,
                                                  cpi.items[cpi.size].color.g,
                                                  cpi.items[cpi.size].color.b,
                                                  255 }
            }) {
                Clay_OnHover(toggle_bool, (intptr_t) &cpi.items[cpi.size].visible);
                calculate_color_offsets(cpi.size);
                if (cpi.items[cpi.size].visible) {
                    cc_color_picker(im, cpi.size);
                }
            }
        }
    }
    ++cpi.size;
}
