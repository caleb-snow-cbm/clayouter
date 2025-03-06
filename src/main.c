#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>

#include "clay.h"
#include "clay_enum_names.h"
#include "clay_renderer_raylib.h"
#include "types.h"
#include "ui_element.h"

#define WINDOW_WIDTH (1024)
#define WINDOW_HEIGHT (768)

#define numberof(x) (sizeof(x) / sizeof(*x))

typedef void (*on_hover_cb_t)(Clay_ElementId id, Clay_PointerData data, intptr_t user_data);
typedef void (*layout_fn_t)(void*);

typedef enum {
    FONT_SIZE_BODY,
    FONT_SIZE_HEADER,
    FONT_SIZE_TITLE,

    NUM_FONT_SIZES
} font_size_t;

const int font_sizes[] = {
    [FONT_SIZE_BODY] = 20,
    [FONT_SIZE_HEADER] = 24,
    [FONT_SIZE_TITLE] = 30,
};

typedef struct {
    size_t capacity;
    size_t size;
    dstring_t** text_boxes;
} text_box_tab_list_t;

typedef struct {
    Clay__SizingType sizing_width_type;
    Clay__SizingType sizing_height_type;
    dstring_t sizing_width;
    dstring_t sizing_height;

    dstring_t padding_left;
    dstring_t padding_right;
    dstring_t padding_top;
    dstring_t padding_bottom;

    dstring_t child_gap;

    Clay_LayoutAlignmentX child_alignment_x;
    Clay_LayoutAlignmentY child_alignment_y;

    Clay_LayoutDirection layout_direction;
} layout_properties_t;

typedef struct {
    dstring_t r;
    dstring_t g;
    dstring_t b;
    dstring_t a;
} color_string_t;

typedef struct {
    dstring_t offset_x;
    dstring_t offset_y;
    dstring_t expand_width;
    dstring_t expand_height;
    dstring_t parent_id;
    dstring_t z_index;
    Clay_FloatingAttachPointType element_attach_type;
    Clay_FloatingAttachPointType parent_attach_type;
    Clay_PointerCaptureMode pointer_capture_mode;
    Clay_FloatingAttachToElement attach_to;
} floating_properties_t;

typedef struct {
    dstring_t text;
    color_string_t text_color;
    dstring_t font_id;
    dstring_t font_size;
    dstring_t letter_spacing;
    dstring_t line_height;
    Clay_TextElementConfigWrapMode wrap_mode;
    Clay_TextAlignment text_alignment;
    bool hash_string_contents;
} text_properties_t;

typedef struct {
    dstring_t id;
    layout_properties_t layout;
    color_string_t background_color;
    dstring_t corner_radius_top_left;
    dstring_t corner_radius_top_right;
    dstring_t corner_radius_bottom_left;
    dstring_t corner_radius_bottom_right;
    floating_properties_t floating;
} declaration_properties_t;

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

const Clay_Color BACKGROUND_COLOR = { 24, 24, 24, 255 };
const Clay_Color BUTTON_COLOR = { 55, 55, 55, 255 };
const Clay_Color BUTTON_HOVERED_COLOR = { 4, 24, 100, 255 };
const Clay_Color TEXT_BOX_COLOR = { 55, 55, 55, 255 };
const Clay_Color TEXT_BOX_SELECTED_COLOR = { 55, 55, 100, 255 };
const Clay_Color TEXT_COLOR = { 255, 255, 255, 255 };
const Clay_Color TAB_COLOR = { 55, 55, 55, 255 };
const Clay_Color TAB_SELECTED_COLOR = { 55, 55, 100, 255 };
const Clay_Color TAB_HOVERED_COLOR = { 4, 24, 100, 255 };
#define TITLE_TEXT                                                                                 \
    CLAY_TEXT_CONFIG({ .textColor = TEXT_COLOR,                                                    \
        .fontId = FONT_SIZE_TITLE,                                                                 \
        .fontSize = font_sizes[FONT_SIZE_TITLE] })
#define HEADER_TEXT                                                                                \
    CLAY_TEXT_CONFIG({ .textColor = TEXT_COLOR,                                                    \
        .fontId = FONT_SIZE_HEADER,                                                                \
        .fontSize = font_sizes[FONT_SIZE_HEADER] })
#define BODY_TEXT                                                                                  \
    CLAY_TEXT_CONFIG({ .textColor = TEXT_COLOR,                                                    \
        .fontId = FONT_SIZE_BODY,                                                                  \
        .fontSize = font_sizes[FONT_SIZE_BODY] })
#define DEFAULT_CORNER_RADIUS CLAY_CORNER_RADIUS(8)

static Clay_ElementDeclaration dropdown_menu;
static ui_element_t* dropdown_parent = NULL;
static dstring_t* selected_text_box;
static declaration_properties_t selected_declaration_properties;
static text_properties_t selected_text_properties;
static ui_element_t* selected_ui_element = NULL;
static uint8_t* selection_menu_item;
static text_box_tab_list_t text_boxes;
ui_element_t root;

void button(Clay_String text, on_hover_cb_t hover_cb, intptr_t user_data);

void clay_error(Clay_ErrorData err)
{
    fprintf(stderr, "CLAY ERROR: %d, %.*s\n", (int) err.errorType, (int) err.errorText.length,
        err.errorText.chars);
    exit(1);
}

static char get_char_from_key(int key)
{
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (key >= KEY_A && key <= KEY_RIGHT_BRACKET) {
        return shift ? key : key + 0x20;
    } else if (key >= KEY_KP_0 && key <= KEY_KP_9) {
        return key - KEY_KP_0 + '0';
    } else
        switch (key) {
        case KEY_APOSTROPHE:
            return shift ? '\"' : '\'';
        case KEY_COMMA:
            return shift ? '<' : ',';
        case KEY_MINUS:
            return shift ? '_' : '-';
        case KEY_PERIOD:
            return shift ? '>' : '.';
        case KEY_SLASH:
            return shift ? '?' : '/';
        case KEY_ZERO:
            return shift ? ')' : '0';
        case KEY_ONE:
            return shift ? '!' : '1';
        case KEY_TWO:
            return shift ? '@' : '2';
        case KEY_THREE:
            return shift ? '#' : '3';
        case KEY_FOUR:
            return shift ? '$' : '4';
        case KEY_FIVE:
            return shift ? '%' : '5';
        case KEY_SIX:
            return shift ? '^' : '6';
        case KEY_SEVEN:
            return shift ? '&' : '7';
        case KEY_EIGHT:
            return shift ? '*' : '8';
        case KEY_NINE:
            return shift ? '(' : '9';
        case KEY_SEMICOLON:
            return shift ? ':' : ';';
        case KEY_EQUAL:
            return shift ? '+' : '=';
        case KEY_BACKSPACE:
            return '\b';
        case KEY_KP_DECIMAL:
            return '.';
        case KEY_KP_DIVIDE:
            return '/';
        case KEY_KP_MULTIPLY:
            return '*';
        case KEY_KP_SUBTRACT:
            return '-';
        case KEY_KP_ADD:
            return '+';
        case KEY_KP_ENTER:
            return '\n';
        case KEY_KP_EQUAL:
            return '=';
        default:
            return ' ';
        }
}

void text_box_append(dstring_t* text, char c)
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

void text_box_advance(void)
{
    // find currently selceted textbox
    for (size_t i = 0; i < text_boxes.size; ++i) {
        if (text_boxes.text_boxes[i] == selected_text_box) {
            selected_text_box = text_boxes.text_boxes[(i + 1) % text_boxes.size];
            break;
        }
    }
}

void dynamic_string_copy(dstring_t* dst, Clay_String src)
{
    if (dst->capacity < src.length) {
        dst->capacity = src.length;
        void* tmp = (char*) realloc((char*) dst->s.chars, dst->capacity);
        dst->s.chars = (char*) tmp;
    }
    memcpy((char*) dst->s.chars, src.chars, src.length);
    dst->s.length = src.length;
}

void select_text_box(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_text_box = (dstring_t*) user_data;
    }
}

void hover_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME && id.id != dropdown_menu.id.id) {
        dropdown_parent = NULL;
    }
    if (data.right_state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        dropdown_menu.floating.offset.x = data.position.x;
        dropdown_menu.floating.offset.y = data.position.y;
        dropdown_parent = (ui_element_t*) user_data;
    }
}

void toggle_bool(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        *(bool*) user_data = !(*(bool*) user_data);
    }
}

void select_tab(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_tab_info_t* info = (selected_tab_info_t*) user_data;
        *info->selected_tab_ptr = info->index;
    }
}

void initialize_tab_page_info(tab_page_t* page)
{
    for (size_t i = 0; i < page->num_tabs; ++i) {
        page->selected_tab_infos[i].selected_tab_ptr = &page->selected_tab;
        page->selected_tab_infos[i].index = i;
    }
}

void tab_page(tab_page_t* page, void* user_data)
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
            .backgroundColor = BACKGROUND_COLOR })
        {
            for (size_t i = 0; i < page->num_tabs; ++i) {
                bool selected = i == page->selected_tab;
                CLAY({
                    .layout = { .sizing = { .width = CLAY_SIZING_FIT(30) },
                        .padding = CLAY_PADDING_ALL(4),
                        .childGap = 4 },
                    .backgroundColor = selected ? TAB_SELECTED_COLOR
                        : Clay_Hovered()        ? TAB_HOVERED_COLOR
                                                : TAB_COLOR,
                    .cornerRadius = { .topLeft = 8, .topRight = 8 },
                    .border = { .width = { .bottom = 1 } },
                })
                {
                    Clay_OnHover(select_tab, (intptr_t) &page->selected_tab_infos[i]);
                    CLAY_TEXT(page->tab_names[i], BODY_TEXT);
                }
            }
        }
        CLAY(
            { .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
                  .padding = CLAY_PADDING_ALL(4),
                  .childGap = 4,
                  .layoutDirection = CLAY_TOP_TO_BOTTOM },
                .scroll.vertical = true,
                .border = { .color = TEXT_COLOR, .width = CLAY_BORDER_OUTSIDE(1) } })
        {
            page->layout_funcs[page->selected_tab](user_data);
        }
    }
}

void text_box(dstring_t* text, Clay_String label)
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
        .backgroundColor = BACKGROUND_COLOR,
    })
    {
        CLAY_TEXT(label, BODY_TEXT);
        const Clay_BorderElementConfig border
            = { .color = TEXT_BOX_SELECTED_COLOR, .width = CLAY_BORDER_OUTSIDE(1) };
        const Clay_BorderElementConfig no_border
            = { .color = TEXT_BOX_SELECTED_COLOR, .width = CLAY_BORDER_OUTSIDE(0) };
        CLAY({
            .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(32) },
                .padding = CLAY_PADDING_ALL(4) },
            .backgroundColor = TEXT_BOX_COLOR,
            .border = selected ? border : no_border,
        })
        {
            Clay_OnHover(select_text_box, (intptr_t) text);
            CLAY_TEXT(text->s, HEADER_TEXT);
        }
    }
}

void check_box(bool* value, Clay_String label)
{
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } })
    {
        CLAY_TEXT(label, BODY_TEXT);
        CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0) } } }) { }
        // TODO: Dynamic checkbox size?
        CLAY({ .layout
            = { .sizing = { .width = CLAY_SIZING_FIXED(24), .height = CLAY_SIZING_FIXED(24) },
                .padding = CLAY_PADDING_ALL(8) },
            .backgroundColor = *value ? TAB_SELECTED_COLOR
                : Clay_Hovered()      ? TAB_HOVERED_COLOR
                                      : (Clay_Color) { 0 },
            .cornerRadius = DEFAULT_CORNER_RADIUS,
            .border = { .color = TEXT_COLOR, .width = CLAY_BORDER_OUTSIDE(1) } })
        {
            Clay_OnHover(toggle_bool, (intptr_t) value);
            CLAY({ .layout
                = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } },
                .backgroundColor = *value ? TAB_SELECTED_COLOR : (Clay_Color) { 0 } });
        }
    }
}

void selection_menu_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    assert(selection_menu_item);
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        *selection_menu_item = (uint8_t) user_data;
        selection_menu_item = NULL;
    }
}

#define selection_menu(x)                                                                          \
    do {                                                                                           \
        CLAY({ .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM },                                \
            .cornerRadius = DEFAULT_CORNER_RADIUS,                                                 \
            .floating = { .attachTo = CLAY_ATTACH_TO_PARENT },                                     \
            .scroll.vertical = true,                                                               \
            .border = { .color = TEXT_COLOR, .width = CLAY_BORDER_ALL(1) } })                      \
        {                                                                                          \
            for (size_t i = 0; i < CLAY_ENUM_COUNT(x); i++) {                                      \
                button(CLAY_ENUM_VALUE_NAME(x, i), selection_menu_callback, (intptr_t) i);         \
            }                                                                                      \
        }                                                                                          \
    } while (false)

void open_selection_menu(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selection_menu_item = (uint8_t*) user_data;
    }
}

#define selection_item(x, value_ptr)                                                               \
    do {                                                                                           \
        CLAY({ .layout = { .layoutDirection = CLAY_TOP_TO_BOTTOM } })                              \
        {                                                                                          \
            CLAY_TEXT(CLAY_ENUM_NAME(x), BODY_TEXT);                                               \
            button(                                                                                \
                CLAY_ENUM_VALUE_NAME(x, *value_ptr), open_selection_menu, (intptr_t) value_ptr);   \
            if (selection_menu_item == value_ptr) {                                                \
                selection_menu(x);                                                                 \
            }                                                                                      \
        }                                                                                          \
    } while (false)

Clay_LayoutConfig parse_layout(layout_properties_t* layout)
{
    Clay_LayoutConfig ret = { 0 };
    ret.sizing.width.type = layout->sizing_width_type;
    if (layout->sizing_width.s.length)
        ret.sizing.width.size.percent = strtof(layout->sizing_width.s.chars, NULL);
    ret.sizing.height.type = layout->sizing_height_type;
    if (layout->sizing_height.s.length)
        ret.sizing.height.size.percent = strtof(layout->sizing_height.s.chars, NULL);

    if (layout->padding_top.s.length)
        ret.padding.top = (uint16_t) strtoul(layout->padding_top.s.chars, NULL, 0);
    if (layout->padding_bottom.s.length)
        ret.padding.bottom = (uint16_t) strtoul(layout->padding_bottom.s.chars, NULL, 0);
    if (layout->padding_right.s.length)
        ret.padding.right = (uint16_t) strtoul(layout->padding_right.s.chars, NULL, 0);
    if (layout->padding_left.s.length)
        ret.padding.left = (uint16_t) strtoul(layout->padding_left.s.chars, NULL, 0);

    if (layout->child_gap.s.length)
        ret.childGap = (uint16_t) strtoul(layout->child_gap.s.chars, NULL, 0);
    ret.childAlignment.x = layout->child_alignment_x;
    ret.childAlignment.y = layout->child_alignment_y;

    ret.layoutDirection = layout->layout_direction;
    return ret;
}

static void load_float(dstring_t* dst, float src)
{
    if (dst->capacity)
        dst->s.length = snprintf((char*) dst->s.chars, dst->capacity, "%3.3f", src);
}

static void load_uint(dstring_t* dst, uint64_t src)
{
    if (dst->capacity)
        dst->s.length = snprintf((char*) dst->s.chars, dst->capacity, "%" PRIu64, src);
}

void load_layout(layout_properties_t* dst, const Clay_LayoutConfig* src)
{
    dst->sizing_width_type = src->sizing.width.type;
    dst->sizing_height_type = src->sizing.height.type;
    load_float(&dst->sizing_width, src->sizing.width.size.percent);
    load_float(&dst->sizing_height, src->sizing.height.size.percent);

    load_uint(&dst->padding_left, src->padding.left);
    load_uint(&dst->padding_right, src->padding.right);
    load_uint(&dst->padding_top, src->padding.top);
    load_uint(&dst->padding_bottom, src->padding.bottom);

    load_uint(&dst->child_gap, src->childGap);

    dst->child_alignment_x = src->childAlignment.x;
    dst->child_alignment_y = src->childAlignment.y;

    dst->layout_direction = src->layoutDirection;
}

Clay_Color parse_color(const color_string_t* c)
{
    Clay_Color ret = { 0, 0, 0, 255 };
    if (c->r.s.length) {
        ret.r = strtof(c->r.s.chars, NULL);
    }
    if (c->g.s.length) {
        ret.g = strtof(c->g.s.chars, NULL);
    }
    if (c->b.s.length) {
        ret.b = strtof(c->b.s.chars, NULL);
    }
    if (c->a.s.length) {
        ret.a = strtof(c->a.s.chars, NULL);
    }
    return ret;
}

void load_color(color_string_t* dst, Clay_Color src)
{
    load_float(&dst->r, src.r);
    load_float(&dst->g, src.g);
    load_float(&dst->b, src.b);
    load_float(&dst->a, src.a);
}

void parse_text_config(Clay_TextElementConfig* dst, const text_properties_t* src)
{
    if (src->font_id.s.length)
        dst->fontId = (uint16_t) strtoul(src->font_id.s.chars, NULL, 0);
    if (src->font_size.s.length)
        dst->fontSize = (uint16_t) strtoul(src->font_size.s.chars, NULL, 0);
    if (src->letter_spacing.s.length)
        dst->letterSpacing = (uint16_t) strtoul(src->letter_spacing.s.chars, NULL, 0);
    if (src->line_height.s.length)
        dst->lineHeight = (uint16_t) strtoul(src->line_height.s.chars, NULL, 0);
    dst->textColor = parse_color(&src->text_color);
    dst->wrapMode = src->wrap_mode;
    dst->textAlignment = src->text_alignment;
    dst->hashStringContents = src->hash_string_contents;
}

void parse_corner_radius(Clay_CornerRadius* dst, const declaration_properties_t* src)
{
    if (src->corner_radius_top_left.s.length)
        dst->topLeft = strtoul(src->corner_radius_top_left.s.chars, NULL, 0);
    if (src->corner_radius_top_left.s.length)
        dst->topLeft = strtoul(src->corner_radius_top_left.s.chars, NULL, 0);
    if (src->corner_radius_top_left.s.length)
        dst->topLeft = strtoul(src->corner_radius_top_left.s.chars, NULL, 0);
    if (src->corner_radius_top_left.s.length)
        dst->topLeft = strtoul(src->corner_radius_top_left.s.chars, NULL, 0);
}

void save_properties(void)
{
    printf("saving properties\n");
    if (selected_ui_element->type == UI_ELEMENT_DECLARATION) {
        Clay_ElementDeclaration* element = selected_ui_element->ptr;

        static int previous_length = 0;
        if (selected_declaration_properties.id.s.length
            && previous_length != selected_declaration_properties.id.s.length) {
            char* before = (char*) element->id.stringId.chars;
            element->id = Clay__HashString(selected_declaration_properties.id.s, 0, 0);
            void* tmp = realloc(before, element->id.stringId.length);
            element->id.stringId.chars = tmp;
            memcpy((char*) element->id.stringId.chars, selected_declaration_properties.id.s.chars,
                element->id.stringId.length);
            previous_length = selected_declaration_properties.id.s.length;
        }
        element->layout = parse_layout(&selected_declaration_properties.layout);
        element->backgroundColor = parse_color(&selected_declaration_properties.background_color);
        parse_corner_radius(&element->cornerRadius, &selected_declaration_properties);
    } else if (selected_ui_element->type == UI_ELEMENT_TEXT) {
        Clay_TextElementConfig* tp = selected_ui_element->text_config;
        dynamic_string_copy(&selected_ui_element->text, selected_text_properties.text.s);
        parse_text_config(tp, &selected_text_properties);
    }
}

void load_properties(void)
{
    if (selected_ui_element->type == UI_ELEMENT_DECLARATION) {
        Clay_ElementDeclaration* element = selected_ui_element->ptr;
        dynamic_string_copy(&selected_declaration_properties.id, element->id.stringId);
        load_layout(&selected_declaration_properties.layout, &element->layout);
        load_color(&selected_declaration_properties.background_color, element->backgroundColor);

        load_float(
            &selected_declaration_properties.corner_radius_top_left, element->cornerRadius.topLeft);
        load_float(&selected_declaration_properties.corner_radius_top_right,
            element->cornerRadius.topRight);
        load_float(&selected_declaration_properties.corner_radius_bottom_left,
            element->cornerRadius.bottomLeft);
        load_float(&selected_declaration_properties.corner_radius_bottom_right,
            element->cornerRadius.bottomRight);
    } else if (selected_ui_element->type == UI_ELEMENT_TEXT) {
    }
}

void save_default(void) { }

void close_properties_window(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_ui_element = NULL;
    }
}

void general_properties_layout(void* user_data)
{
    declaration_properties_t* p = (declaration_properties_t*) user_data;
    text_box(&p->id, CLAY_STRING("ID"));
    CLAY_TEXT(CLAY_STRING("Background Color"), HEADER_TEXT);
    CLAY({})
    {
        text_box(&p->background_color.r, CLAY_STRING("R"));
        text_box(&p->background_color.g, CLAY_STRING("G"));
        text_box(&p->background_color.b, CLAY_STRING("B"));
        text_box(&p->background_color.a, CLAY_STRING("A"));
    }
    CLAY_TEXT(CLAY_STRING("Corner Radius"), HEADER_TEXT);
    CLAY({})
    {
        text_box(&p->corner_radius_top_left, CLAY_STRING("Top left"));
        text_box(&p->corner_radius_top_right, CLAY_STRING("Top right"));
        text_box(&p->corner_radius_bottom_left, CLAY_STRING("Bottom left"));
        text_box(&p->corner_radius_bottom_right, CLAY_STRING("Bottom right"));
    }
}

void layout_properties_layout(void* user_data)
{
    declaration_properties_t* p = (declaration_properties_t*) user_data;
    selection_item(Clay__SizingType, &p->layout.sizing_width_type);
    text_box(&p->layout.sizing_width, CLAY_STRING("Width"));
    selection_item(Clay__SizingType, &p->layout.sizing_height_type);
    text_box(&p->layout.sizing_height, CLAY_STRING("Height"));
    CLAY_TEXT(CLAY_STRING("Padding"), BODY_TEXT);
    CLAY({})
    {
        text_box(&p->layout.padding_top, CLAY_STRING("Top"));
        text_box(&p->layout.padding_left, CLAY_STRING("Left"));
        text_box(&p->layout.padding_right, CLAY_STRING("Right"));
        text_box(&p->layout.padding_bottom, CLAY_STRING("Bottom"));
    }
    text_box(&p->layout.child_gap, CLAY_STRING("Child gap"));
    CLAY_TEXT(CLAY_STRING("Child alignment"), BODY_TEXT);
    selection_item(Clay_LayoutAlignmentX, &p->layout.child_alignment_x);
    selection_item(Clay_LayoutAlignmentY, &p->layout.child_alignment_y);

    selection_item(Clay_LayoutDirection, &p->layout.layout_direction);
}

void floating_properties_layout(void* user_data)
{
    floating_properties_t* p = &((declaration_properties_t*) user_data)->floating;
    CLAY_TEXT(CLAY_STRING("Offset"), HEADER_TEXT);
    CLAY({}) {
        text_box(&p->offset_x, CLAY_STRING("X"));
        text_box(&p->offset_y, CLAY_STRING("Y"));
    }
    CLAY_TEXT(CLAY_STRING("Expand"), HEADER_TEXT);
    CLAY({}) {
        text_box(&p->expand_width, CLAY_STRING("W"));
        text_box(&p->expand_height, CLAY_STRING("H"));
    }
    text_box(&p->parent_id, CLAY_STRING("Parent ID"));
    text_box(&p->z_index, CLAY_STRING("Z index"));
    selection_item(Clay_FloatingAttachPointType, &p->element_attach_type);
    selection_item(Clay_FloatingAttachPointType, &p->parent_attach_type);
}

void text_properties_layout(text_properties_t* p)
{
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
               .padding = CLAY_PADDING_ALL(4),
               .childGap = 4,
               .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .scroll.vertical = true })
    {
        text_box(&p->text, CLAY_STRING("Text"));
        CLAY({})
        {
            text_box(&p->text_color.r, CLAY_STRING("R"));
            text_box(&p->text_color.g, CLAY_STRING("G"));
            text_box(&p->text_color.b, CLAY_STRING("B"));
            text_box(&p->text_color.a, CLAY_STRING("A"));
        }
        text_box(&p->font_id, CLAY_STRING("Font ID"));
        text_box(&p->font_size, CLAY_STRING("Font size"));
        text_box(&p->letter_spacing, CLAY_STRING("Letter spacing"));
        text_box(&p->line_height, CLAY_STRING("Line height"));
        selection_item(Clay_TextElementConfigWrapMode, &p->wrap_mode);
        selection_item(Clay_TextAlignment, &p->text_alignment);
        check_box(&p->hash_string_contents, CLAY_STRING("Hash string contents"));
    }
}

void properies_window(void)
{
    CLAY({
        .id = CLAY_ID("properties window"),
        .layout = { .sizing = { .width = CLAY_SIZING_FIXED(GetScreenWidth() / 2),
                        .height = CLAY_SIZING_FIXED(GetScreenHeight() / 2) },
            .padding = CLAY_PADDING_ALL(8),
            .childGap = 4,
            .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .backgroundColor = BACKGROUND_COLOR,
        .cornerRadius = DEFAULT_CORNER_RADIUS,
        .floating = { .attachPoints = { .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                          .parent = CLAY_ATTACH_POINT_CENTER_CENTER },
            .attachTo = CLAY_ATTACH_TO_ROOT },
        .border = { .color = TEXT_COLOR, .width = CLAY_BORDER_OUTSIDE(1) },
    })
    {
        CLAY_TEXT(CLAY_STRING("Properties"), TITLE_TEXT);

        if (selected_ui_element->type == UI_ELEMENT_DECLARATION) {
            static Clay_String tab_names[] = {
                CLAY_STRING("General"),
                CLAY_STRING("Layout"),
                CLAY_STRING("Floating"),
            };
            static layout_fn_t tab_funcs[] = {
                general_properties_layout,
                layout_properties_layout,
                floating_properties_layout,
            };
            static selected_tab_info_t infos[numberof(tab_names)];

            static tab_page_t properties_tabs = {
                .tab_names = tab_names,
                .layout_funcs = tab_funcs,
                .selected_tab_infos = infos,
                .num_tabs = numberof(tab_names),
                .selected_tab = 0,
            };
            static bool tabs_initialized = false;
            if (!tabs_initialized) {
                initialize_tab_page_info(&properties_tabs);
                tabs_initialized = true;
            }

            tab_page(&properties_tabs, &selected_declaration_properties);
        } else if (selected_ui_element->type == UI_ELEMENT_TEXT) {
            text_properties_layout(&selected_text_properties);
        }

        CLAY({})
        {
            button(CLAY_STRING("Close"), close_properties_window, 0);
            // button(CLAY_STRING("Save as Default"), save_default, 0);
        }
    }
}

void button(Clay_String text, on_hover_cb_t hover_cb, intptr_t user_data)
{
    CLAY({
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIT(0) },
            .padding = CLAY_PADDING_ALL(4) },
        .backgroundColor = Clay_Hovered() ? BUTTON_HOVERED_COLOR : BUTTON_COLOR,
        .cornerRadius = CLAY_CORNER_RADIUS(4),
    })
    {
        Clay_OnHover(hover_cb, user_data);
        CLAY_TEXT(text, BODY_TEXT);
    }
}

void add_element_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_ui_element = ui_element_add((ui_element_t*) user_data, UI_ELEMENT_DECLARATION);
        load_properties();
        dropdown_parent = NULL;
    }
}

void add_text_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_ui_element = ui_element_add((ui_element_t*) user_data, UI_ELEMENT_TEXT);
        load_properties();
        dropdown_parent = NULL;
    }
}

void remove_element_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        ui_element_remove((ui_element_t*) user_data);
        dropdown_parent = NULL;
    }
}

void properties_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_ui_element = (ui_element_t*) user_data;
        load_properties();
        dropdown_parent = NULL;
    }
}

void dump_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        dump_tree(stdout, &root, 0);
    }
}

void dropdown(ui_element_t* parent)
{
    CLAY(dropdown_menu)
    {
        button(CLAY_STRING("Add element"), add_element_callback, (intptr_t) parent);
        button(
            CLAY_STRING("Insert element after"), add_element_callback, (intptr_t) parent->parent);
        button(CLAY_STRING("Add text"), add_text_callback, (intptr_t) parent);
        button(CLAY_STRING("Remove element"), remove_element_callback, (intptr_t) parent);
        button(CLAY_STRING("Properties"), properties_callback, (intptr_t) parent);
        button(CLAY_STRING("Dump tree"), dump_callback, 0);
    }
}

void configure_element(ui_element_t* me)
{
    if (me == NULL) {
        return;
    }
    if (me->type == UI_ELEMENT_DECLARATION) {
        Clay__OpenElement();
        Clay__ConfigureOpenElement(*me->ptr);
        Clay_OnHover(hover_callback, (intptr_t) me);
        if (dropdown_parent == me) {
            dropdown(me);
        }
        for (size_t i = 0; i < me->num_children; ++i) {
            configure_element(me->children[i]);
        }
        Clay__CloseElement();
    } else if (me->type == UI_ELEMENT_TEXT) {
        Clay__OpenTextElement(me->text.s, me->text_config);
    }
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Clayouter");
    SetTargetFPS(60);
    uint64_t clay_memory_size = Clay_MinMemorySize();
    void* clay_memory = malloc(clay_memory_size);
    Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_memory_size, clay_memory);

    Clay_ErrorHandler err = { .errorHandlerFunction = clay_error, .userData = NULL };
    Clay_Initialize(clay_arena, (Clay_Dimensions) { WINDOW_WIDTH, WINDOW_HEIGHT }, err);

    Font fonts[NUM_FONT_SIZES];
    for (size_t i = 0; i < NUM_FONT_SIZES; ++i) {
        fonts[i] = LoadFontEx("resources/Roboto-Regular.ttf", font_sizes[i], NULL, 128);
    }
    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts);

    root.ptr = (Clay_ElementDeclaration*) malloc(sizeof(Clay_ElementDeclaration));
    *root.ptr = (Clay_ElementDeclaration) {
        .id = CLAY_ID("root"),
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) } },
    };

    dropdown_menu.id = CLAY_ID("dropdown");

    dropdown_menu.layout.padding = CLAY_PADDING_ALL(8);
    dropdown_menu.layout.childGap = 8;
    dropdown_menu.layout.layoutDirection = CLAY_TOP_TO_BOTTOM;
    dropdown_menu.backgroundColor = BACKGROUND_COLOR;
    dropdown_menu.cornerRadius = DEFAULT_CORNER_RADIUS;
    dropdown_menu.floating.attachTo = CLAY_ATTACH_TO_ROOT;
    dropdown_menu.border.color = TEXT_COLOR;
    dropdown_menu.border.width = (Clay_BorderWidth) CLAY_BORDER_OUTSIDE(1);

    while (!WindowShouldClose()) {
        bool left_mouse = IsMouseButtonDown(0);
        bool right_mouse = IsMouseButtonDown(1);
        Clay_SetLayoutDimensions((Clay_Dimensions) { GetScreenWidth(), GetScreenHeight() });
        Clay_SetPointerStateEx(
            RAYLIB_VECTOR_TO_CLAY_VECTOR(GetMousePosition()), left_mouse, right_mouse);
        Clay_UpdateScrollContainers(
            true, RAYLIB_VECTOR_TO_CLAY_VECTOR(GetMouseWheelMoveV()), GetFrameTime());

        int key = GetKeyPressed();
        if (key) {
            if (selected_text_box) {
                if (key == KEY_TAB) {
                    text_box_advance();
                } else if (key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT) {
                    text_box_append(selected_text_box, get_char_from_key(key));
                }
            }
        }
        text_boxes.size = 0;

        Clay_BeginLayout();
        if (selected_ui_element) {
            properies_window();
            if (key || left_mouse || right_mouse)
                save_properties();
        }
        configure_element(&root);

        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(Clay_EndLayout(), fonts);
        DrawFPS(0, 0);
        EndDrawing();
    }

    for (size_t i = 0; i < root.num_children; ++i) {
        ui_element_remove(root.children[i]);
    }
    for (size_t i = 0; i < NUM_FONT_SIZES; ++i) {
        UnloadFont(fonts[i]);
    }
    CloseWindow();
    free(clay_memory);
}
