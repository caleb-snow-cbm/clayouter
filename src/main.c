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
#include "components/clay_components.h"
#include "ui_element.h"
#include "IO/dump_tree.h"
#include "IO/parse_tree.h"

#define WINDOW_WIDTH (1600)
#define WINDOW_HEIGHT (900)

#ifdef _WIN32
#define numberof(x) _countof(x)
#define EMPTY    0
#else
#define numberof(x) (sizeof(x) / sizeof(*x))
#define EMPTY
#endif

typedef struct {
    dstring_t id;
    color_string_t background_color;
    dstring_t corner_radius_top_left;
    dstring_t corner_radius_top_right;
    dstring_t corner_radius_bottom_left;
    dstring_t corner_radius_bottom_right;
    bool scroll_horizontal;
    bool scroll_vertical;
} general_properties_t;

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
    color_string_t color;
    dstring_t left;
    dstring_t right;
    dstring_t top;
    dstring_t bottom;
    dstring_t between_children;
} border_properties_t;

typedef struct {
    bool enable;
    color_string_t color;
    dstring_t callback;
} on_hover_properties_t;

typedef struct {
    dstring_t text;
    color_string_t text_color;
    dstring_t font_name;
    dstring_t font_size;
    dstring_t letter_spacing;
    dstring_t line_height;
    Clay_TextElementConfigWrapMode wrap_mode;
    Clay_TextAlignment text_alignment;
    bool hash_string_contents;
} text_properties_t;

typedef struct {
    general_properties_t general;
    layout_properties_t layout;
    floating_properties_t floating;
    border_properties_t border;
    on_hover_properties_t on_hover;
} declaration_properties_t;

typedef struct {
    Clay_String id;
    uint16_t size;
} font_info_t;

typedef struct {
    font_info_t* info;
    Font* fonts;
    size_t count;
    size_t capacity;
} fonts_t;

typedef enum {
    FSV_NONE,
    FSV_IMPORT,
    FSV_EXPORT
} file_selection_visibility_t;

#define DEFAULT_CORNER_RADIUS CLAY_CORNER_RADIUS(8)

static const theme_t* theme;
#define TITLE_TEXT (&theme->text_types[TT_TITLE])
#define HEADER_TEXT (&theme->text_types[TT_HEADER])
#define BODY_TEXT (&theme->text_types[TT_BODY])

static Clay_ElementDeclaration dropdown_menu;
static ui_element_t* dropdown_parent = NULL;
static declaration_properties_t selected_d_properties;
static text_properties_t selected_t_properties;
static ui_element_t* selected_ui_element = NULL;
static ui_element_t* root;
static Clay_ImageElementConfig color_picker_im;
static file_selection_visibility_t file_selection_visible = FSV_NONE;
static cc_selection_menu_t child_selection_menu = {
    .label = CLAY_STRING_CONST("Child elements")
};
static ui_element_t* selection_box_parent = NULL;

static fonts_t fonts;
static FilePathList font_files;
static cc_selection_menu_t font_selection_menu = {
    .label = CLAY_STRING_CONST("Fonts")
};

void load_properties(void);

#define enum_selection_item(e, value_ptr)                                                          \
    do {                                                                                           \
        const enum_info_t* i = CLAY_ENUM_INFO(e);                                                  \
        cc_selection_item(*i->name, i->values, i->count, (uint8_t*) (value_ptr));                  \
    } while (0)

void clay_error(Clay_ErrorData err)
{
    fprintf(stderr, "CLAY ERROR: %d, %.*s\n", (int) err.errorType, (int) err.errorText.length,
        err.errorText.chars);
}

static char get_char_from_key(int key)
{
    bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
    if (key >= KEY_A && key <= KEY_RIGHT_BRACKET) {
        return shift ? key : key + 0x20;
    } else if (key >= KEY_KP_0 && key <= KEY_KP_9) {
        return key - KEY_KP_0 + '0';
    } else switch (key) {
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

void dynamic_string_copy(dstring_t* dst, Clay_String src)
{
    if (dst->capacity <= src.length) {
        dst->capacity = src.length;
        void* tmp = (char*) realloc((char*) dst->s.chars, dst->capacity + 1);
        assert(tmp);
        dst->s.chars = (char*) tmp;
    }
    if (src.chars)
        memcpy((char*) dst->s.chars, src.chars, src.length);
    ((char*)dst->s.chars)[src.length] = '\0';
    dst->s.length = src.length;
}

void clay_string_copy(Clay_String* dst, dstring_t src)
{
    if (src.s.length) {
        void* tmp = realloc((char*) dst->chars, src.s.length);
        assert(tmp);
        dst->chars = tmp;
        memcpy((char*) dst->chars, src.s.chars, src.s.length);
        dst->length = src.s.length;
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
        bool* x = (bool*) user_data;
        *x = !(*x);
    }
}

void select_font_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) return;
    size_t font_index = (size_t) user_data;
    dynamic_string_copy(&selected_t_properties.font_name, font_selection_menu.options[font_index]);
    font_selection_menu.visible = false;

    assert(selected_ui_element);
    assert(selected_ui_element->type == UI_ELEMENT_TEXT);
    uint16_t current_size = selected_ui_element->text_config->fontSize;
    for (size_t i = 0; i < fonts.count; ++i) {
        if (fonts.info[i].size == current_size &&
            strcmp(fonts.info[i].id.chars, font_selection_menu.options[font_index].chars) == 0)
        {
            selected_ui_element->text_config->fontId = i;
            load_properties();
            return;
        }
    }

    if (fonts.count == fonts.capacity) {
        fonts.capacity *= 2;
        fonts.fonts = realloc(fonts.fonts, sizeof(*fonts.fonts) * fonts.capacity);
        fonts.info = realloc(fonts.info, sizeof(*fonts.info) * fonts.capacity);
        Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts.fonts);
    }
    size_t index = fonts.count;
    fonts.count++;
    fonts.fonts[index] = LoadFontEx(font_files.paths[font_index], current_size, NULL, 400);
    fonts.info[index].id = font_selection_menu.options[font_index];
    fonts.info[index].size = current_size;
    selected_ui_element->text_config->fontId = index;
    load_properties();
}

Clay_LayoutConfig save_layout(layout_properties_t* layout)
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

#define LOAD(dst, src, format)                                                                     \
    do {                                                                                           \
        int necessary_cap = snprintf(NULL, 0, format, (src));                                      \
        if ((dst).capacity < necessary_cap + 1) {                                                  \
            (dst).capacity = necessary_cap + 1;                                                    \
            void* tmp = realloc((char*) (dst).s.chars, (dst).capacity + 1);                        \
            assert(tmp);                                                                           \
            (dst).s.chars = tmp;                                                                   \
        }                                                                                          \
        (dst).s.length = sprintf((char*) (dst).s.chars, format, (src));                            \
    } while (0)

#define LOAD_FLOAT(dst, src) LOAD(dst, src, "%3.1f")
#define LOAD_UINT(dst, src) LOAD(dst, (uint64_t) (src), "%" PRIu64)
#define LOAD_INT(dst, src) LOAD(dst, (int64_t) (src), "%" PRId64)

static void load_color(color_string_t* dst, Clay_Color src)
{
    LOAD_FLOAT(dst->r, src.r);
    LOAD_FLOAT(dst->g, src.g);
    LOAD_FLOAT(dst->b, src.b);
    LOAD_FLOAT(dst->a, src.a);
}

static void load_layout(layout_properties_t* dst, const Clay_LayoutConfig* src)
{
    dst->sizing_width_type = src->sizing.width.type;
    dst->sizing_height_type = src->sizing.height.type;
    LOAD_FLOAT(dst->sizing_width, src->sizing.width.size.percent);
    LOAD_FLOAT(dst->sizing_height, src->sizing.height.size.percent);

    LOAD_UINT(dst->padding_left, src->padding.left);
    LOAD_UINT(dst->padding_right, src->padding.right);
    LOAD_UINT(dst->padding_top, src->padding.top);
    LOAD_UINT(dst->padding_bottom, src->padding.bottom);

    LOAD_UINT(dst->child_gap, src->childGap);

    dst->child_alignment_x = src->childAlignment.x;
    dst->child_alignment_y = src->childAlignment.y;

    dst->layout_direction = src->layoutDirection;
}

static void load_floating(floating_properties_t* dst, const Clay_FloatingElementConfig* src)
{
    LOAD_FLOAT(dst->offset_x, src->offset.x);
    LOAD_FLOAT(dst->offset_y, src->offset.y);
    LOAD_FLOAT(dst->expand_width, src->expand.width);
    LOAD_FLOAT(dst->expand_height, src->expand.height);
    // load_uint(&dst->parent_id, src->parentId); TODO
    LOAD_INT(dst->z_index, src->zIndex);
    dst->element_attach_type = src->attachPoints.element;
    dst->parent_attach_type = src->attachPoints.parent;
    dst->pointer_capture_mode = src->pointerCaptureMode;
    dst->attach_to = src->attachTo;
}

static void load_border(border_properties_t* dst, const Clay_BorderElementConfig* src)
{
    load_color(&dst->color, src->color);
    LOAD_UINT(dst->left, src->width.left);
    LOAD_UINT(dst->right, src->width.right);
    LOAD_UINT(dst->top, src->width.top);
    LOAD_UINT(dst->bottom, src->width.bottom);
    LOAD_UINT(dst->between_children, src->width.betweenChildren);
}

static void load_on_hover(on_hover_properties_t* dst, const on_hover_config_t* src)
{
    load_color(&dst->color, src->hovered_color);
    dst->enable = src->enabled;
    if (src->callback.length)
        dynamic_string_copy(&dst->callback, src->callback);
}

Clay_TextElementConfig save_text_config(const text_properties_t* src)
{
    Clay_TextElementConfig ret = { 0 };
    if (src->font_size.s.length)
        ret.fontSize = (uint16_t) strtoul(src->font_size.s.chars, NULL, 0);
    for (size_t i = 0; i < fonts.count; ++i) {
        if (fonts.info[i].size == ret.fontSize &&
            strcmp(fonts.info[i].id.chars, src->font_name.s.chars) == 0) {
            ret.fontId = (uint16_t) i;
            break;
        }
    }
    if (src->letter_spacing.s.length)
        ret.letterSpacing = (uint16_t) strtoul(src->letter_spacing.s.chars, NULL, 0);
    if (src->line_height.s.length)
        ret.lineHeight = (uint16_t) strtoul(src->line_height.s.chars, NULL, 0);
    ret.textColor = cc_parse_color(&src->text_color);
    ret.wrapMode = src->wrap_mode;
    ret.textAlignment = src->text_alignment;
    ret.hashStringContents = src->hash_string_contents;
    return ret;
}

Clay_CornerRadius save_corner_radius(const general_properties_t* src)
{
    Clay_CornerRadius ret = { 0 };
    if (src->corner_radius_top_left.s.length)
        ret.topLeft = strtof(src->corner_radius_top_left.s.chars, NULL);
    if (src->corner_radius_top_left.s.length)
        ret.topLeft = strtof(src->corner_radius_top_left.s.chars, NULL);
    if (src->corner_radius_top_left.s.length)
        ret.topLeft = strtof(src->corner_radius_top_left.s.chars, NULL);
    if (src->corner_radius_top_left.s.length)
        ret.topLeft = strtof(src->corner_radius_top_left.s.chars, NULL);
    return ret;
}

Clay_FloatingElementConfig save_floating(const floating_properties_t* src)
{
    Clay_FloatingElementConfig ret = { 0 };
    if (src->offset_x.s.length)
        ret.offset.x = strtof(src->offset_x.s.chars, NULL);
    if (src->offset_y.s.length)
        ret.offset.y = strtof(src->offset_y.s.chars, NULL);
    if (src->expand_width.s.length)
        ret.expand.width = strtof(src->expand_width.s.chars, NULL);
    if (src->expand_height.s.length)
        ret.expand.height = strtof(src->expand_height.s.chars, NULL);
    // if (src->parent_id.s.length) TODO
    //     ret.parentId
    if (src->z_index.s.length)
        ret.zIndex = (int16_t) strtol(src->z_index.s.chars, NULL, 0);
    ret.attachPoints.element = src->element_attach_type;
    ret.attachPoints.parent = src->parent_attach_type;
    ret.pointerCaptureMode = src->pointer_capture_mode;
    ret.attachTo = src->attach_to;
    return ret;
}

Clay_BorderElementConfig save_border(const border_properties_t* src)
{
    Clay_BorderElementConfig ret = { 0 };
    ret.color = cc_parse_color(&src->color);
    if (src->left.s.length)
        ret.width.left = (uint16_t) strtoul(src->left.s.chars, NULL, 0);
    if (src->right.s.length)
        ret.width.right = (uint16_t) strtoul(src->right.s.chars, NULL, 0);
    if (src->top.s.length)
        ret.width.top = (uint16_t) strtoul(src->top.s.chars, NULL, 0);
    if (src->bottom.s.length)
        ret.width.bottom = (uint16_t) strtoul(src->bottom.s.chars, NULL, 0);
    if (src->between_children.s.length)
        ret.width.betweenChildren = (uint16_t) strtoul(src->between_children.s.chars, NULL, 0);

    return ret;
}

void save_on_hover(on_hover_config_t* dst, const on_hover_properties_t* src)
{
    clay_string_copy(&dst->callback, src->callback);
    dst->enabled = src->enable;
    dst->hovered_color = cc_parse_color(&src->color);
}

void save_properties(void)
{
    if (selected_ui_element->type == UI_ELEMENT_DECLARATION) {
        Clay_ElementDeclaration* element = selected_ui_element->ptr;

        static int previous_length = 0;
        if (selected_d_properties.general.id.s.length
            && previous_length != selected_d_properties.general.id.s.length) {
            char* before = (char*) element->id.stringId.chars;
            element->id = Clay__HashString(selected_d_properties.general.id.s, 0, 0);
            void* tmp = realloc(before, (size_t) element->id.stringId.length);
            element->id.stringId.chars = tmp;
            memcpy((char*) element->id.stringId.chars, selected_d_properties.general.id.s.chars,
                (size_t) element->id.stringId.length);
            previous_length = selected_d_properties.general.id.s.length;
        }
        element->layout = save_layout(&selected_d_properties.layout);
        element->backgroundColor = cc_parse_color(&selected_d_properties.general.background_color);
        element->cornerRadius = save_corner_radius(&selected_d_properties.general);
        element->floating = save_floating(&selected_d_properties.floating);
        element->scroll.horizontal = selected_d_properties.general.scroll_horizontal;
        element->scroll.vertical = selected_d_properties.general.scroll_vertical;
        element->border = save_border(&selected_d_properties.border);
        save_on_hover(&selected_ui_element->on_hover, &selected_d_properties.on_hover);
        selected_ui_element->on_hover.non_hovered_color = element->backgroundColor;
    } else if (selected_ui_element->type == UI_ELEMENT_TEXT) {
        dynamic_string_copy(&selected_ui_element->text, selected_t_properties.text.s);
        *selected_ui_element->text_config = save_text_config(&selected_t_properties);
    }
}

void load_properties(void)
{
    if (selected_ui_element->type == UI_ELEMENT_DECLARATION) {
        Clay_ElementDeclaration* element = selected_ui_element->ptr;
        dynamic_string_copy(&selected_d_properties.general.id, element->id.stringId);
        load_layout(&selected_d_properties.layout, &element->layout);
        load_color(&selected_d_properties.general.background_color, element->backgroundColor);

        LOAD_FLOAT(
            selected_d_properties.general.corner_radius_top_left, element->cornerRadius.topLeft);
        LOAD_FLOAT(
            selected_d_properties.general.corner_radius_top_right, element->cornerRadius.topRight);
        LOAD_FLOAT(selected_d_properties.general.corner_radius_bottom_left,
            element->cornerRadius.bottomLeft);
        LOAD_FLOAT(selected_d_properties.general.corner_radius_bottom_right,
            element->cornerRadius.bottomRight);

        load_floating(&selected_d_properties.floating, &element->floating);
        selected_d_properties.general.scroll_horizontal = element->scroll.horizontal;
        selected_d_properties.general.scroll_vertical = element->scroll.vertical;
        load_border(&selected_d_properties.border, &element->border);
        load_on_hover(&selected_d_properties.on_hover, &selected_ui_element->on_hover);
    } else if (selected_ui_element->type == UI_ELEMENT_TEXT) {
        Clay_TextElementConfig* src = selected_ui_element->text_config;
        dynamic_string_copy(&selected_t_properties.text, selected_ui_element->text.s);
        load_color(&selected_t_properties.text_color, src->textColor);
        dynamic_string_copy(&selected_t_properties.font_name, fonts.info[src->fontId].id);
        LOAD_UINT(selected_t_properties.font_size, src->fontSize);
        LOAD_UINT(selected_t_properties.letter_spacing, src->letterSpacing);
        LOAD_UINT(selected_t_properties.line_height, src->lineHeight);
        selected_t_properties.wrap_mode = src->wrapMode;
        selected_t_properties.text_alignment = src->textAlignment;
        selected_t_properties.hash_string_contents = src->hashStringContents;
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

void open_parent(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        // if parent is not root
        if (selected_ui_element->parent->parent) {
            selected_ui_element = selected_ui_element->parent;
            load_properties();
        }
    }
}

void open_children(Clay_ElementId id, Clay_PointerData data, intptr_t user_data) {
    (void) id;
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        child_selection_menu.visible = true;
    }
}

void general_properties_layout(void* user_data)
{
    general_properties_t* p = &((declaration_properties_t*) user_data)->general;
    cc_text_box(&p->id, CLAY_STRING("ID"));
    CLAY_TEXT(CLAY_STRING("Background Color"), HEADER_TEXT);
    cc_color_selector(color_picker_im, &p->background_color);
    CLAY_TEXT(CLAY_STRING("Corner Radius"), HEADER_TEXT);
    CLAY({ EMPTY })
    {
        cc_text_box(&p->corner_radius_top_left, CLAY_STRING("Top left"));
        cc_text_box(&p->corner_radius_top_right, CLAY_STRING("Top right"));
        cc_text_box(&p->corner_radius_bottom_left, CLAY_STRING("Bottom left"));
        cc_text_box(&p->corner_radius_bottom_right, CLAY_STRING("Bottom right"));
    }
    cc_check_box(&p->scroll_horizontal, CLAY_STRING("Scroll (horizontal)"));
    cc_check_box(&p->scroll_vertical, CLAY_STRING("Scroll (vertical)"));
}

void layout_properties_layout(void* user_data)
{
    declaration_properties_t* p = (declaration_properties_t*) user_data;
    enum_selection_item(Clay__SizingType, &p->layout.sizing_width_type);
    cc_text_box(&p->layout.sizing_width, CLAY_STRING("Width"));
    enum_selection_item(Clay__SizingType, &p->layout.sizing_height_type);
    cc_text_box(&p->layout.sizing_height, CLAY_STRING("Height"));
    CLAY_TEXT(CLAY_STRING("Padding"), BODY_TEXT);
    CLAY({ EMPTY })
    {
        cc_text_box(&p->layout.padding_top, CLAY_STRING("Top"));
        cc_text_box(&p->layout.padding_left, CLAY_STRING("Left"));
        cc_text_box(&p->layout.padding_right, CLAY_STRING("Right"));
        cc_text_box(&p->layout.padding_bottom, CLAY_STRING("Bottom"));
    }
    cc_text_box(&p->layout.child_gap, CLAY_STRING("Child gap"));
    CLAY_TEXT(CLAY_STRING("Child alignment"), BODY_TEXT);
    enum_selection_item(Clay_LayoutAlignmentX, &p->layout.child_alignment_x);
    enum_selection_item(Clay_LayoutAlignmentY, &p->layout.child_alignment_y);

    enum_selection_item(Clay_LayoutDirection, &p->layout.layout_direction);
}

void floating_properties_layout(void* user_data)
{
    floating_properties_t* p = &((declaration_properties_t*) user_data)->floating;
    CLAY_TEXT(CLAY_STRING("Offset"), HEADER_TEXT);
    CLAY({ EMPTY })
    {
        cc_text_box(&p->offset_x, CLAY_STRING("X"));
        cc_text_box(&p->offset_y, CLAY_STRING("Y"));
    }
    CLAY_TEXT(CLAY_STRING("Expand"), HEADER_TEXT);
    CLAY({ EMPTY })
    {
        cc_text_box(&p->expand_width, CLAY_STRING("W"));
        cc_text_box(&p->expand_height, CLAY_STRING("H"));
    }
    cc_text_box(&p->parent_id, CLAY_STRING("Parent ID"));
    cc_text_box(&p->z_index, CLAY_STRING("Z index"));
    CLAY({ .layout = { .childGap = 4 } })
    {
        enum_selection_item(Clay_FloatingAttachPointType, &p->element_attach_type);
        CLAY_TEXT(CLAY_STRING("Element"), BODY_TEXT);
    }
    CLAY({ .layout = { .childGap = 4 } })
    {
        enum_selection_item(Clay_FloatingAttachPointType, &p->parent_attach_type);
        CLAY_TEXT(CLAY_STRING("Parent"), BODY_TEXT);
    }
    enum_selection_item(Clay_PointerCaptureMode, &p->pointer_capture_mode);
    enum_selection_item(Clay_FloatingAttachToElement, &p->attach_to);
}

void border_properties_layout(void* user_data)
{
    border_properties_t* p = &((declaration_properties_t*) user_data)->border;
    CLAY_TEXT(CLAY_STRING("Color"), HEADER_TEXT);
    cc_color_selector(color_picker_im, &p->color);
    CLAY_TEXT(CLAY_STRING("Width"), HEADER_TEXT);
    cc_text_box(&p->left, CLAY_STRING("Left"));
    cc_text_box(&p->right, CLAY_STRING("Right"));
    cc_text_box(&p->top, CLAY_STRING("Top"));
    cc_text_box(&p->bottom, CLAY_STRING("Bottom"));
    cc_text_box(&p->between_children, CLAY_STRING("Between children"));
}

void on_hover_properties_layout(void* user_data)
{
    on_hover_properties_t* p = &((declaration_properties_t*)user_data)->on_hover;
    cc_check_box(&p->enable, CLAY_STRING("Enable"));
    CLAY_TEXT(CLAY_STRING("Hover color"), HEADER_TEXT);
    cc_color_selector(color_picker_im, &p->color);
    cc_text_box(&p->callback, CLAY_STRING("Callback function"));
}

void text_properties_layout(text_properties_t* p)
{
    CLAY({ .layout = { .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) },
               .padding = CLAY_PADDING_ALL(4),
               .childGap = 4,
               .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .scroll.vertical = true })
    {
        cc_text_box(&p->text, CLAY_STRING("Text"));
        cc_color_selector(color_picker_im, &p->text_color);
        cc_button(p->font_name.s, toggle_bool, (intptr_t) &font_selection_menu.visible);
        if (font_selection_menu.visible) {
            cc_selection_menu(&font_selection_menu);
        }
        cc_text_box(&p->font_size, CLAY_STRING("Font size"));
        cc_text_box(&p->letter_spacing, CLAY_STRING("Letter spacing"));
        cc_text_box(&p->line_height, CLAY_STRING("Line height"));
        enum_selection_item(Clay_TextElementConfigWrapMode, &p->wrap_mode);
        enum_selection_item(Clay_TextAlignment, &p->text_alignment);
        cc_check_box(&p->hash_string_contents, CLAY_STRING("Hash string contents"));
    }
}

void properties_window(void)
{
    CLAY({
        .id = CLAY_ID("properties window"),
        .layout = { .sizing = { .width = CLAY_SIZING_FIXED((float) GetScreenWidth() / 2),
                        .height = CLAY_SIZING_FIXED((float) GetScreenHeight() / 2) },
            .padding = CLAY_PADDING_ALL(8),
            .childGap = 4,
            .layoutDirection = CLAY_TOP_TO_BOTTOM },
        .backgroundColor = theme->background,
        .cornerRadius = DEFAULT_CORNER_RADIUS,
        .floating = { .attachPoints = { .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                          .parent = CLAY_ATTACH_POINT_CENTER_CENTER },
            .attachTo = CLAY_ATTACH_TO_ROOT },
        .border = { .color = theme->highlight, .width = CLAY_BORDER_OUTSIDE(1) },
    })
    {
        CLAY_TEXT(CLAY_STRING("Properties"), TITLE_TEXT);

        if (selected_ui_element->type == UI_ELEMENT_DECLARATION) {
            static Clay_String tab_names[] = {
                CLAY_STRING_CONST("General"),
                CLAY_STRING_CONST("Layout"),
                CLAY_STRING_CONST("Floating"),
                CLAY_STRING_CONST("Border"),
                CLAY_STRING_CONST("On Hover"),
            };
            static layout_fn_t tab_funcs[] = {
                general_properties_layout,
                layout_properties_layout,
                floating_properties_layout,
                border_properties_layout,
                on_hover_properties_layout,
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
                cc_initialize_tab_page(&properties_tabs);
                tabs_initialized = true;
            }

            cc_tab_page(&properties_tabs, &selected_d_properties);
        } else if (selected_ui_element->type == UI_ELEMENT_TEXT) {
            text_properties_layout(&selected_t_properties);
        }

        CLAY({ .layout = { .sizing.width = CLAY_SIZING_GROW(0), .childGap = 4 }})
        {
            cc_button(CLAY_STRING("Close"), close_properties_window, 0);

            CLAY({ .layout.sizing.width = CLAY_SIZING_GROW(0) }); // spacer
            cc_button(CLAY_STRING("Open parent"), open_parent, 0);
            cc_button(CLAY_STRING("Open children"), open_children, 0);
            // cc_button(CLAY_STRING("Save as Default"), save_default, 0);
        }
    }
}

void select_child_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_ui_element = (ui_element_t*) user_data;
        load_properties();
    } else {
        selection_box_parent = (ui_element_t*) user_data;
    }
}

void show_children(ui_element_t* e)
{
    static size_t child_capacity = 0;
    if (e->type == UI_ELEMENT_TEXT) return;
    if (e->num_children > child_capacity) {
        size_t old_cap = child_capacity;
        child_capacity = e->num_children;
        child_selection_menu.options = realloc(child_selection_menu.options, sizeof(Clay_String) * child_capacity);
        memset(child_selection_menu.options + old_cap, 0, sizeof(Clay_String) * (child_capacity - old_cap));
        child_selection_menu.cbs = realloc(child_selection_menu.cbs, sizeof(on_hover_cb_t) * child_capacity);
        child_selection_menu.user_data = realloc(child_selection_menu.user_data, sizeof(intptr_t) * child_capacity);
    }
    for (size_t i = 0; i < e->num_children; ++i) {
        if (child_selection_menu.options[i].length == 0) {
            char* tmp = (char*) malloc(64);
            child_selection_menu.options[i].chars = tmp;
        }
        child_selection_menu.cbs[i] = select_child_callback;
        child_selection_menu.user_data[i] = (intptr_t) e->children[i];
    }
    for (size_t i = 0; i < e->num_children; ++i) {
        // assumes 2 types of elements
        child_selection_menu.options[i].length
            = snprintf((char*) child_selection_menu.options[i].chars, 64, "Child %zu, %s", i,
                e->children[i]->type ? "Text" : "Element");
    }
    child_selection_menu.count = e->num_children;
    cc_selection_menu(&child_selection_menu);
}

void add_element_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        ui_element_t* parent = (ui_element_t*) user_data;
        if (parent == dropdown_parent) {
            selected_ui_element = ui_element_insert_before(parent, NULL, UI_ELEMENT_DECLARATION);
        } else {
            selected_ui_element = ui_element_insert_after((ui_element_t*) user_data, dropdown_parent, UI_ELEMENT_DECLARATION);
        }
        load_properties();
        dropdown_parent = NULL;
    }
}

void insert_before_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_ui_element = ui_element_insert_before((ui_element_t*) user_data, dropdown_parent, UI_ELEMENT_DECLARATION);
        load_properties();
        dropdown_parent = NULL;
    }
}

void add_text_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        selected_ui_element = ui_element_insert_after((ui_element_t*) user_data, NULL, UI_ELEMENT_TEXT);
        load_properties();
        dropdown_parent = NULL;
    }
}

void import_element_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state != CLAY_POINTER_DATA_PRESSED_THIS_FRAME) return;
    dstring_t* path = (dstring_t*) user_data;
    ui_element_t* tmp = parse_tree(path->s.chars);
    if (tmp) {
        ui_element_t* parent = dropdown_parent;
        parent->num_children++;
        parent->children = realloc(parent->children, sizeof(*parent->children) * parent->num_children);
        assert(parent->children);
        parent->children[parent->num_children - 1] = tmp;
        tmp->parent = parent;
        selected_ui_element = tmp;
        load_properties();
        file_selection_visible = FSV_NONE;
        dropdown_parent = NULL;
    }
}

void remove_element_callback(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        if ((ui_element_t*) user_data != root)
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
    dstring_t* path = (dstring_t*) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        dump_tree(path->s.chars, root);
        dropdown_parent = NULL;
        file_selection_visible = FSV_NONE;
    }
}

void open_file_selection(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        file_selection_visible = (file_selection_visibility_t) user_data;
    }
}

void close_file_selection(Clay_ElementId id, Clay_PointerData data, intptr_t user_data)
{
    (void) id;
    (void) user_data;
    if (data.state == CLAY_POINTER_DATA_PRESSED_THIS_FRAME) {
        file_selection_visible = FSV_NONE;
    }
}

static void selection_box(void)
{
    Clay_Color c = theme->selected;
    c.a = 100;
    CLAY({
        .id = CLAY_ID("Selection box"),
        .layout = { .sizing = { .width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0) } },
        .backgroundColor = c,
        .floating = { .pointerCaptureMode = CLAY_POINTER_CAPTURE_MODE_PASSTHROUGH,
                    .attachTo = CLAY_ATTACH_TO_PARENT }
    });
}

void file_selection(on_hover_cb_t select_callback, Clay_String label)
{
    static dstring_t path = { 0 };
    CLAY( { .id = CLAY_ID("import_selection"),
            .layout = { .padding = CLAY_PADDING_ALL(8),
                        .childGap = 8,
                        .layoutDirection = CLAY_TOP_TO_BOTTOM },
            .backgroundColor = theme->background,
            .cornerRadius = DEFAULT_CORNER_RADIUS,
            .border = { .color = theme->highlight, .width = CLAY_BORDER_OUTSIDE(1) },
            .floating = { .zIndex = INT16_MAX,
                          .attachPoints = { .element = CLAY_ATTACH_POINT_CENTER_CENTER,
                                            .parent = CLAY_ATTACH_POINT_CENTER_CENTER },
                          .attachTo = CLAY_ATTACH_TO_ROOT }}) {
        cc_text_box(&path, CLAY_STRING("Enter filename:"));
        CLAY({ .layout = { .childGap = 8 }}) {
            cc_button(label, select_callback, (intptr_t) &path);
            cc_button(CLAY_STRING("Cancel"), close_file_selection, 0);
        }
    }
}

void dropdown(ui_element_t* parent)
{
    CLAY(dropdown_menu)
    {
        cc_button(CLAY_STRING("Add element"), add_element_callback, (intptr_t) parent);
        cc_button(
            CLAY_STRING("Insert element after"), add_element_callback, (intptr_t) parent->parent);
        cc_button(
            CLAY_STRING("Insert element before"), insert_before_callback, (intptr_t) parent->parent);
        cc_button(CLAY_STRING("Add text"), add_text_callback, (intptr_t) parent);
        cc_button(CLAY_STRING("Import element"), open_file_selection, (intptr_t) FSV_IMPORT);
        cc_button(CLAY_STRING("Remove element"), remove_element_callback, (intptr_t) parent);
        cc_button(CLAY_STRING("Properties"), properties_callback, (intptr_t) parent);
        cc_button(CLAY_STRING("Export layout"), open_file_selection, (intptr_t) FSV_EXPORT);
    }
}

void configure_element(ui_element_t* me)
{
    if (me == NULL) {
        return;
    }
    if (me->type == UI_ELEMENT_DECLARATION) {
        Clay__OpenElement();
        if (me->on_hover.enabled && Clay_Hovered()) {
            me->ptr->backgroundColor = me->on_hover.hovered_color;
        } else {
            me->ptr->backgroundColor = me->on_hover.non_hovered_color;
        }
        Clay__ConfigureOpenElement(*me->ptr);
        Clay_OnHover(hover_callback, (intptr_t) me);
        if (dropdown_parent == me) {
            dropdown(me);
        }
        if (selection_box_parent == me) {
            selection_box();
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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Clayouter");
    SetTargetFPS(60);
    uint32_t clay_memory_size = Clay_MinMemorySize();
    void* clay_memory = malloc(clay_memory_size);
    Clay_Arena clay_arena = Clay_CreateArenaWithCapacityAndMemory(clay_memory_size, clay_memory);

    Clay_ErrorHandler err = { .errorHandlerFunction = clay_error, .userData = NULL };
    Clay_Initialize(clay_arena, (Clay_Dimensions) { WINDOW_WIDTH, WINDOW_HEIGHT }, err);

    theme = cc_get_theme();
    font_files = LoadDirectoryFiles("resources");
    const char* default_font = "Roboto-Regular";
    fonts.capacity = theme->text_types_count;
    fonts.count = theme->text_types_count;
    fonts.fonts = (Font*) malloc(sizeof(*fonts.fonts) * fonts.capacity);
    fonts.info = (font_info_t*) malloc(sizeof(*fonts.info) * fonts.capacity);
    for (size_t i = 0; i < font_files.count;) {
        if (strcmp(GetFileExtension(font_files.paths[i]), ".ttf")) {
            font_files.count--;
            font_files.capacity--;
            free(font_files.paths[i]);
            memmove(&font_files.paths[i], &font_files.paths[i + 1], sizeof(void*) * (font_files.count - i));
            continue;
        }
        const char* filename = GetFileNameWithoutExt(font_files.paths[i]);
        if (!strcmp(filename, default_font)) {
            size_t filename_len = strlen(filename);
            char* allocated_filename = malloc(filename_len + 1);
            assert(allocated_filename);
            strcpy(allocated_filename, filename);
            for (size_t j = 0; j < theme->text_types_count; ++j) {
                fonts.fonts[j] = LoadFontEx(font_files.paths[i], theme->text_types[j].fontSize, NULL, 400);
                fonts.info[j].id = (Clay_String) { filename_len, allocated_filename };
                fonts.info[j].size = theme->text_types[j].fontSize;
            }
            break;
        }
        ++i;
    }
    font_selection_menu.count = font_files.count;
    font_selection_menu.options = (Clay_String*) malloc(sizeof(Clay_String) * font_files.count);
    font_selection_menu.cbs = (on_hover_cb_t*) malloc(sizeof(on_hover_cb_t) * font_files.count);
    font_selection_menu.user_data = (intptr_t*) malloc(sizeof(intptr_t)* font_files.count);
    for (size_t i = 0; i < font_files.count; ++i) {
        const char* filename = GetFileNameWithoutExt(font_files.paths[i]);
        int32_t filename_len = (int32_t) strlen(filename);
        char* allocated_filename = malloc(filename_len + 1);
        assert(allocated_filename);
        strcpy(allocated_filename, filename);
        font_selection_menu.options[i] = (Clay_String) { filename_len, allocated_filename };
        font_selection_menu.cbs[i] = select_font_callback;
        font_selection_menu.user_data[i] =  (intptr_t) i;
    }

    Clay_SetMeasureTextFunction(Raylib_MeasureText, fonts.fonts);

    root = ui_element_insert_after(NULL, NULL, UI_ELEMENT_DECLARATION);
    root->ptr->layout.sizing = (Clay_Sizing) { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_GROW(0) };
    root->ptr->id.stringId.chars = malloc(sizeof("root"));
    assert(root->ptr->id.stringId.chars);
    strcpy((char*) root->ptr->id.stringId.chars, "root");
    root->ptr->id.stringId.length = sizeof("root") - 1;

    dropdown_menu.id = CLAY_ID("dropdown");

    dropdown_menu.layout.padding = CLAY_PADDING_ALL(8);
    dropdown_menu.layout.childGap = 8;
    dropdown_menu.layout.layoutDirection = CLAY_TOP_TO_BOTTOM;
    dropdown_menu.backgroundColor = theme->background;
    dropdown_menu.cornerRadius = DEFAULT_CORNER_RADIUS;
    dropdown_menu.floating.attachTo = CLAY_ATTACH_TO_ROOT;
    dropdown_menu.floating.zIndex = INT16_MAX - 10;
    dropdown_menu.border.color = theme->highlight;
    dropdown_menu.border.width = (Clay_BorderWidth) CLAY_BORDER_OUTSIDE(1);
    Texture2D color_picker_texture = LoadTexture("resources/color_picker.png");
    color_picker_im.imageData = &color_picker_texture;
    color_picker_im.sourceDimensions.width = (float) color_picker_texture.width;
    color_picker_im.sourceDimensions.height = (float) color_picker_texture.height;

    while (!WindowShouldClose()) {
        selection_box_parent = NULL;

        bool left_mouse = IsMouseButtonDown(0);
        bool right_mouse = IsMouseButtonDown(1);
        Clay_SetLayoutDimensions((Clay_Dimensions) { (float) GetScreenWidth(),
                                                     (float) GetScreenHeight() });
        Clay_SetPointerStateEx(
            RAYLIB_VECTOR_TO_CLAY_VECTOR(GetMousePosition()), left_mouse, right_mouse);
        Clay_UpdateScrollContainers(
            true, RAYLIB_VECTOR_TO_CLAY_VECTOR(GetMouseWheelMoveV()), GetFrameTime());

        int key = GetKeyPressed();
        if (key && cc_get_selected_text_box()) {
            if (key == KEY_TAB) {
                cc_text_box_advance();
            } else if (key != KEY_LEFT_SHIFT && key != KEY_RIGHT_SHIFT) {
                cc_text_box_append(cc_get_selected_text_box(), get_char_from_key(key));
            }
        }
        cc_begin_layout();

        Clay_BeginLayout();
        if (selected_ui_element) {
            properties_window();
            if (key || left_mouse || right_mouse)
                save_properties();
            if (child_selection_menu.visible) {
                show_children(selected_ui_element);
            }
        }
        configure_element(root);

        if (file_selection_visible == FSV_IMPORT) {
            file_selection(import_element_callback, CLAY_STRING("Import"));
        } else if (file_selection_visible == FSV_EXPORT) {
            file_selection(dump_callback, CLAY_STRING("Export"));
        }

        BeginDrawing();
        ClearBackground(BLACK);
        Clay_Raylib_Render(Clay_EndLayout(), fonts.fonts);
        EndDrawing();
    }

    UnloadDirectoryFiles(font_files);
    cc_free();
    ui_element_remove(root);
    for (size_t i = 0; i < fonts.count; ++i) {
        UnloadFont(fonts.fonts[i]);
    }
    free(fonts.fonts);
    free(fonts.info);
    CloseWindow();
    free(clay_memory);
}
