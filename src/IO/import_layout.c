#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "clay_enum_names.h"
#include "clay_struct_names.h"
#include "ui_element.h"
#include "utilities.h"

#include "stb_c_lexer_config.h"
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#ifndef numberof
#define numberof(x) (sizeof(x) / sizeof(*(x)))
#endif

typedef struct {
    ui_element_t* parent;
    ui_element_t* me;
    stb_lexer* lexer;
    const char* filename;
} parse_ctx_t;

static bool parse_bool(parse_ctx_t* ctx, bool* out);
static bool parse_integral(parse_ctx_t* ctx, uint8_t* out, uint8_t size);
static bool parse_float(parse_ctx_t* ctx, float* out);
static bool parse_string_literal(parse_ctx_t* ctx, dstring_t* s);
static bool parse_enum(parse_ctx_t* ctx, uint8_t* out, const enum_info_t* info);
static bool parse_struct(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info);
static bool parse_struct_by_members(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info);
static bool parse_struct_literal(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info);
static bool parse_struct_member(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info, size_t member_index);
static bool parse_union(parse_ctx_t* ctx, uint8_t* out, const struct_info_t* info);
static bool parse_custom(parse_ctx_t* ctx, uint8_t* out, const struct_info_t* info);
static bool expect_token(parse_ctx_t* ctx, const char* token);

/**
 * @brief Get the next token and compare it with expected values
 * 
 * @param ctx 
 * @param tokens Array of possible tokens, can be single or multi-character
 * @param num_tokens Number of possible tokens
 * @return Index of token found or -1 if none found
 */
static int expect_tokens(parse_ctx_t* ctx, const char** tokens, int num_tokens);

static ui_element_t* parse_tree_r(parse_ctx_t* ctx);

#define REPORT_FAILURE(ctx, expected)                                                              \
    do {                                                                                           \
        stb_lex_location loc;                                                                      \
        stb_c_lexer_get_location((ctx)->lexer, (ctx)->lexer->where_firstchar, &loc);               \
        fprintf(stderr, "Unexpected token at %s:%d:%d\n", (ctx)->filename, loc.line_number,        \
            loc.line_offset);                                                                      \
        fprintf(stderr, "Expected %s\n", expected);                                                \
    } while (0)

#define EXPECT_REQUIRED(ctx, token)                                                                \
    do                                                                                             \
        if (!expect_token(ctx, token)) {                                                           \
            REPORT_FAILURE(ctx, token);                                                            \
            return false;                                                                          \
        }                                                                                          \
    while (0)

/* WRAPPER MACRO HANDLING */
typedef bool (*macro_handler_t)(parse_ctx_t* ctx, uint8_t* out);

static bool clay_sizing_fit(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    Clay_SizingAxis* sizing = (Clay_SizingAxis*) out;
    sizing->type = CLAY__SIZING_TYPE_FIT;
    sizing->size.minMax.max = 0;
    if (ctx->lexer->token == CLEX_floatlit)
        sizing->size.minMax.min = (float) ctx->lexer->real_number;
    else
        sizing->size.minMax.min = (float) ctx->lexer->int_number;
    const char* possible[] = { ",", ")" };
    int next = expect_tokens(ctx, possible, numberof(possible));
    if (next == 1) {
        return true;
    } else if (next == -1) {
        return false;
    }

    ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    if (ctx->lexer->token == CLEX_floatlit)
        sizing->size.minMax.max = (float) ctx->lexer->real_number;
    else
        sizing->size.minMax.max = (float) ctx->lexer->int_number;
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

static bool clay_sizing_grow(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    Clay_SizingAxis* sizing = (Clay_SizingAxis*) out;
    sizing->type = CLAY__SIZING_TYPE_GROW;
    sizing->size.minMax.max = 0;
    if (ctx->lexer->token == CLEX_floatlit)
        sizing->size.minMax.min = (float) ctx->lexer->real_number;
    else
        sizing->size.minMax.min = (float) ctx->lexer->int_number;
    const char* possible[] = { ",", ")" };
    int next = expect_tokens(ctx, possible, numberof(possible));
    if (next == 1) {
        return true;
    } else if (next == -1) {
        return false;
    }

    ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    if (ctx->lexer->token == CLEX_floatlit)
        sizing->size.minMax.max = (float) ctx->lexer->real_number;
    else
        sizing->size.minMax.max = (float) ctx->lexer->int_number;
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

static bool clay_sizing_fixed(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    Clay_SizingAxis* sizing = (Clay_SizingAxis*) out;
    sizing->type = CLAY__SIZING_TYPE_FIXED;
    sizing->size.minMax.max = 0;
    if (ctx->lexer->token == CLEX_floatlit) {
        sizing->size.minMax.min = (float) ctx->lexer->real_number;
        sizing->size.minMax.max = (float) ctx->lexer->real_number;
    } else {
        sizing->size.minMax.min = (float) ctx->lexer->int_number;
        sizing->size.minMax.max = (float) ctx->lexer->int_number;
    }
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

static bool clay_sizing_percent(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    Clay_SizingAxis* sizing = (Clay_SizingAxis*) out;
    sizing->type = CLAY__SIZING_TYPE_PERCENT;
    sizing->size.minMax.max = 0;
    if (ctx->lexer->token == CLEX_floatlit)
        sizing->size.percent = (float) ctx->lexer->real_number;
    else
        sizing->size.percent = (float) ctx->lexer->real_number;
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

static bool clay_corner_radius(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    Clay_CornerRadius* r = (Clay_CornerRadius*) out;
    float value;
    if (ctx->lexer->token == CLEX_floatlit)
        value = (float) ctx->lexer->real_number;
    else
        value = (float) ctx->lexer->int_number;
    r->topLeft = value;
    r->topRight = value;
    r->bottomLeft = value;
    r->bottomRight = value;
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

static bool clay_padding_all(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point or integer literal");
        return false;
    }
    Clay_Padding* padding = (Clay_Padding*) out;
    float value;
    if (ctx->lexer->token == CLEX_floatlit)
        value = (float) ctx->lexer->real_number;
    else
        value = (float) ctx->lexer->int_number;
    padding->left   = (uint16_t) value;
    padding->right  = (uint16_t) value;
    padding->top    = (uint16_t) value;
    padding->bottom = (uint16_t) value;
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

static bool clay_border_outside(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_intlit) {
        REPORT_FAILURE(ctx, "integer literal");
        return false;
    }
    Clay_BorderWidth* border = (Clay_BorderWidth*) out;
    border->top    = (uint16_t) ctx->lexer->int_number;
    border->bottom = (uint16_t) ctx->lexer->int_number;
    border->left   = (uint16_t) ctx->lexer->int_number;
    border->right  = (uint16_t) ctx->lexer->int_number;
    border->betweenChildren = 0;
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

static bool clay_border_all(parse_ctx_t* ctx, uint8_t* out)
{
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_intlit) {
        REPORT_FAILURE(ctx, "integer literal");
        return false;
    }
    Clay_BorderWidth* border = (Clay_BorderWidth*) out;
    border->top             = (uint16_t) ctx->lexer->int_number;
    border->bottom          = (uint16_t) ctx->lexer->int_number;
    border->left            = (uint16_t) ctx->lexer->int_number;
    border->right           = (uint16_t) ctx->lexer->int_number;
    border->betweenChildren = (uint16_t) ctx->lexer->int_number;
    EXPECT_REQUIRED(ctx, ")");
    return true;
}

typedef struct {
    const char* macro;
    macro_handler_t handler;
    struct_info_t* struct_info;
} macro_t;

const macro_t wrapper_macros[] = {
    { .macro = "CLAY_SIZING_FIT",     .handler = clay_sizing_fit,     .struct_info = STRUCT_INFO(Clay_SizingAxis) },
    { .macro = "CLAY_SIZING_GROW",    .handler = clay_sizing_grow,    .struct_info = STRUCT_INFO(Clay_SizingAxis)  },
    { .macro = "CLAY_SIZING_FIXED",   .handler = clay_sizing_fixed,   .struct_info = STRUCT_INFO(Clay_SizingAxis)  },
    { .macro = "CLAY_SIZING_PERCENT", .handler = clay_sizing_percent, .struct_info = STRUCT_INFO(Clay_SizingAxis)  },
    { .macro = "CLAY_PADDING_ALL",    .handler = clay_padding_all,    .struct_info = STRUCT_INFO(Clay_Padding) },
    { .macro = "CLAY_CORNER_RADIUS",  .handler = clay_corner_radius,  .struct_info = STRUCT_INFO(Clay_CornerRadius) },
    { .macro = "CLAY_BORDER_OUTSIDE", .handler = clay_border_outside, .struct_info = STRUCT_INFO(Clay_BorderWidth) },
    { .macro = "CLAY_BORDER_ALL",     .handler = clay_border_all,     .struct_info = STRUCT_INFO(Clay_BorderWidth) },
};

static bool parse_wrapping_macro(parse_ctx_t* ctx, uint8_t* out, const struct_info_t* info)
{
    for (size_t i = 0; i < numberof(wrapper_macros); ++i) {
        if (wrapper_macros[i].struct_info == info &&
            strcmp(ctx->lexer->string, wrapper_macros[i].macro) == 0) {
            return wrapper_macros[i].handler(ctx, out);
        }
    }
    return false;
}

/***************************/

static int expect_tokens(parse_ctx_t* ctx, const char** tokens, int num_tokens)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (ret == 0) {
        return -1;
    }
    char character_buffer[] = " ";
    for (int i = 0; i < num_tokens; ++i) {
        if (ctx->lexer->token < CLEX_eof) {
            character_buffer[0] = (char) ctx->lexer->token;
            if (!strcmp(tokens[i], character_buffer)) {
                return i;
            }
        } else {
            if (!strcmp(tokens[i], ctx->lexer->string)) {
                return i;
            }
        }
    }
    return -1;
}

static bool expect_token(parse_ctx_t* ctx, const char* token)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret) return false;
    if (ctx->lexer->token < CLEX_eof) {
        return token[0] == ctx->lexer->token && token[1] == '\0';
    }
    return strcmp(token, ctx->lexer->string) == 0;
}

static bool parse_bool(parse_ctx_t* ctx, bool* out)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret) return false;
    if (ctx->lexer->token == CLEX_intlit) {
        *out = (bool) ctx->lexer->int_number;
        return true;
    } else if (ctx->lexer->token == CLEX_id) {
        if (!strcmp(ctx->lexer->string, "true")) {
            *out = true;
            return true;
        } else if (!strcmp(ctx->lexer->string, "false")) {
            *out = false;
            return true;
        } else {
            stb_lex_location loc;
            stb_c_lexer_get_location(ctx->lexer, ctx->lexer->where_firstchar, &loc);
            fprintf(stderr, "Unexpected token at %s:%d:%d\n", ctx->filename, loc.line_number,
                loc.line_offset);
            fprintf(stderr, "Expected bool\n");
            return false;
        }
    }
    return false;
}

static bool parse_integral(parse_ctx_t* ctx, uint8_t* out, uint8_t size)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_intlit) {
        REPORT_FAILURE(ctx, "integer literal");
        return false;
    }
    switch (size) {
    case 1:
        *out = (uint8_t) ctx->lexer->int_number;
        break;
    case 2:
        *((uint16_t*) out) = (uint16_t) ctx->lexer->int_number;
        break;
    case 4:
        *((uint32_t*) out) = (uint32_t) ctx->lexer->int_number;
        break;
    case 8:
        *((uint64_t*) out) = (uint64_t) ctx->lexer->int_number;
        break;
    default:
        fprintf(stderr, "Invalid integer size to parse\n");
        return false;
    }
    return true;
}

static bool parse_float(parse_ctx_t* ctx, float* out)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || (ctx->lexer->token != CLEX_floatlit && ctx->lexer->token != CLEX_intlit)) {
        REPORT_FAILURE(ctx, "floating point literal");
        return false;
    }
    if (ctx->lexer->token == CLEX_floatlit)
        *out = (float) ctx->lexer->real_number;
    else
        *out = (float) ctx->lexer->int_number;
    return true;
}

static bool parse_string_literal(parse_ctx_t* ctx, dstring_t* s)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_dqstring) {
        REPORT_FAILURE(ctx, "string literal");
        return false;
    }
    if (s->capacity < ctx->lexer->string_len) {
        s->capacity = ctx->lexer->string_len;
        void* tmp = realloc((char*) s->s.chars, s->capacity);
        assert(tmp);
        s->s.chars = tmp;
    }
    s->s.length = ctx->lexer->string_len;
    memcpy((char*) s->s.chars, ctx->lexer->string, s->capacity);
    return true;
}

static bool parse_enum(parse_ctx_t* ctx, uint8_t* out, const enum_info_t* info)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret) return false;
    if (ctx->lexer->token != CLEX_id) return false;
    for (size_t i = 0; i < info->count; ++i) {
        if (!strcmp(ctx->lexer->string, info->macros[i])) {
            *out = (uint8_t) i;
            return true;
        }
    }
    return false;
}

static bool parse_value(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info, size_t member_index)
{
    if (out == NULL) {
        out = on_hover_out;
    }
    switch (info->info[member_index].type) {
    case TYPE_BOOL:
        if (!parse_bool(ctx, (bool*) out + info->offsets[member_index])) {
            return false;
        }
        break;
    case TYPE_INTEGRAL:
        if (!parse_integral(ctx, out + info->offsets[member_index], info->sizes[member_index])) {
            return false;
        }
        break;
    case TYPE_FLOAT:
        if (!parse_float(ctx, (float*) (out + info->offsets[member_index]))) {
            return false;
        }
        break;
    case TYPE_ENUM:
        if (!parse_enum(ctx, out + info->offsets[member_index], info->info[member_index].enum_info)) {
            return false;
        }
        break;
    case TYPE_STRUCT:
        if (!parse_struct(ctx,
                          out + info->offsets[member_index],
                          on_hover_out ? on_hover_out + info->offsets[member_index] : NULL,
                          info->info[member_index].struct_info)) {
            return false;
        }
        break;
    case TYPE_UNION:
        if (!parse_union(ctx, out + info->offsets[member_index], info->info[member_index].struct_info)) {
            return false;
        }
        break;
    case TYPE_CUSTOM:
        if (!parse_custom(ctx, out + info->offsets[member_index], info->info[member_index].struct_info)) {
            return false;
        }
    }
    return true;
}

static bool parse_struct_member(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info, size_t member_index)
{
    // Check for Clay_Hovered() ternary
    char* prev_parse_point = ctx->lexer->parse_point;
    if (expect_token(ctx, "Clay_Hovered")) {
        ctx->me->on_hover.enabled = true;
        EXPECT_REQUIRED(ctx, "(");
        EXPECT_REQUIRED(ctx, ")");
        EXPECT_REQUIRED(ctx, "?");
        if (!parse_value(ctx, on_hover_out, NULL, info, member_index)) return false;
        EXPECT_REQUIRED(ctx, ":");
        return parse_value(ctx, out, NULL, info, member_index);
    }
    ctx->lexer->parse_point = prev_parse_point;
    if (!parse_value(ctx, out, on_hover_out, info, member_index)) return false;
    if (on_hover_out && info->info[member_index].type != TYPE_STRUCT)
        memcpy(on_hover_out + info->offsets[member_index], out + info->offsets[member_index], info->sizes[member_index]);
    return true;
}

static bool parse_struct_by_members(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info)
{
    for (;;) {
        const char* possible[] = { ".", "}" };
        int token;
next_field:
        token = expect_tokens(ctx, possible, numberof(possible));
        if (token == 0) {
            int ret = stb_c_lexer_get_token(ctx->lexer);
            if (!ret || ctx->lexer->token != CLEX_id) return false;
            for (size_t i = 0; i < info->count; ++i) {
                if (!strcmp(ctx->lexer->string, info->members[i])) {
                    EXPECT_REQUIRED(ctx, "=");
                    if (!parse_struct_member(ctx, out, on_hover_out, info, i)) return false;
                    ret = stb_c_lexer_get_token(ctx->lexer);
                    if (!ret) return false;
                    if (ctx->lexer->token == ',') goto next_field;
                    if (ctx->lexer->token == '}') return true;
                    REPORT_FAILURE(ctx, ", or }");
                }
            }
            return false;
        } else if (token == 1) {
            return true;
        } else {
            return false;
        }
    }
}

static bool parse_struct_literal(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_id) {
        REPORT_FAILURE(ctx, info->name);
        return false;
    }
    EXPECT_REQUIRED(ctx, ")");
    EXPECT_REQUIRED(ctx, "{");
    const char* possible[] = { ",", "}"} ;
    for (size_t i = 0; i < info->count; ++i) {
        parse_struct_member(ctx, out, on_hover_out, info, i);
        int next = expect_tokens(ctx, possible, numberof(possible));
        if (next == -1) return false;
        if (next == 0) continue;
        return true;
    }
    EXPECT_REQUIRED(ctx, "}");
    return true;
}

static bool parse_struct(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info)
{
    // EITHER
    // { .member = ... }
    // OR
    // (struct foo) { ..., ... }
    const char* struct_starters[] = { "{", "(" };
    int token = expect_tokens(ctx, struct_starters, numberof(struct_starters));
    if (token == 0) {
        return parse_struct_by_members(ctx, out, on_hover_out, info);
    } else if (token == 1) {
        return parse_struct_literal(ctx, out, on_hover_out, info);
    } else if (parse_wrapping_macro(ctx, out, info)) {
        if (on_hover_out) {
            size_t total_size = info->offsets[info->count - 1] + info->sizes[info->count - 1];
            memcpy(on_hover_out, out, total_size);
        }
        return true;
    }
    // check for struct wrapping macro
    REPORT_FAILURE(ctx, "{ or (");
    return false;
}

static bool parse_union(parse_ctx_t* ctx, uint8_t* out, const struct_info_t* info)
{
    EXPECT_REQUIRED(ctx, "{");
    assert(0 && "unimplemented");
    (void) out;
    (void) info;
    EXPECT_REQUIRED(ctx, "}");
    return true;
}

static bool parse_custom(parse_ctx_t* ctx, uint8_t* out, const struct_info_t* info)
{
    if (!strcmp(info->name, "Clay_ElementId")) {
        EXPECT_REQUIRED(ctx, "CLAY_ID");
        EXPECT_REQUIRED(ctx, "(");
        dstring_t id = { 0 };
        if (!parse_string_literal(ctx, &id)) return false;
        Clay_String* stringId = (Clay_String*) (out + info->offsets[info->count - 1]);
        *stringId = id.s;
        EXPECT_REQUIRED(ctx, ")");
        return true;
    }
    return false;
}

static ui_element_t* parse_element_declaration(parse_ctx_t* ctx)
{
    EXPECT_REQUIRED(ctx, "(");
    ui_element_t* me = (ui_element_t*) malloc_assert(sizeof *me);
    ctx->me = me;
    memset(me, 0, sizeof *me);
    me->ptr = (Clay_ElementDeclaration*) malloc_assert(sizeof *me->ptr);
    memset(me->ptr, 0, sizeof *me->ptr);
    me->on_hover.ptr = (Clay_ElementDeclaration*) malloc_assert(sizeof *me->on_hover.ptr);
    memset(me->on_hover.ptr, 0, sizeof *me->on_hover.ptr);
    if (!parse_struct(ctx, (uint8_t*) me->ptr, (uint8_t*) me->on_hover.ptr, STRUCT_INFO(Clay_ElementDeclaration))) goto fail;
    EXPECT_REQUIRED(ctx, ")");
    const char* possible_after_decl[] = { "{", ";" };
    int next_token = expect_tokens(ctx, possible_after_decl, numberof(possible_after_decl));
    if (next_token == 1) return me;
    if (next_token == -1) goto fail;
    if (!me->on_hover.enabled) {
        free(me->on_hover.ptr);
        me->on_hover.ptr = NULL;
    }
    ui_element_t* child;
    ctx->parent = me;
    do {
        child = parse_tree_r(ctx);
        if (child && child != (void*) SIZE_MAX) {
            child->parent = me;
            me->num_children++;
            REALLOC_ASSERT(me->children, sizeof(*me->children) * me->num_children);
            me->children[me->num_children - 1] = child;
        }
    } while (child);
    return me;
fail:
    ui_element_remove(me);
    return NULL;
}

static ui_element_t* parse_text(parse_ctx_t* ctx)
{
    ui_element_t* me = NULL;
    EXPECT_REQUIRED(ctx, "CLAY_STRING");
    EXPECT_REQUIRED(ctx, "(");
    me = (ui_element_t*) malloc_assert(sizeof(*me));
    memset(me, 0, sizeof(*me));
    me->type = UI_ELEMENT_TEXT;
    me->text_config = 0;
    me->text_config = (Clay_TextElementConfig*) malloc_assert(sizeof(*me->text_config));
    memset(me->text_config, 0, sizeof(*me->text_config));
    if (!parse_string_literal(ctx, &me->text)) goto fail;
    EXPECT_REQUIRED(ctx, ")");
    EXPECT_REQUIRED(ctx, ",");
    EXPECT_REQUIRED(ctx, "CLAY_TEXT_CONFIG");
    EXPECT_REQUIRED(ctx, "(");
    if (!parse_struct(ctx, (uint8_t*) me->text_config, NULL, STRUCT_INFO(Clay_TextElementConfig))) goto fail;
    EXPECT_REQUIRED(ctx, ")");
    return me;
fail:
    if (me) {
        free(me->text_config);
        free(me);
    }
    return NULL;
}

static bool parse_on_hover(parse_ctx_t* ctx)
{
    if (ctx->parent == NULL) {
        REPORT_FAILURE(ctx, "a parent element before calling Clay_OnHover");
        return false;
    }
    ctx->parent->on_hover.enabled = true;
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_id) {
        REPORT_FAILURE(ctx, "callback function");
        return false;
    }
    ctx->parent->on_hover.callback.length = ctx->lexer->string_len;
    ctx->parent->on_hover.callback.chars = malloc_assert(ctx->lexer->string_len + 1);
    strcpy((char*) ctx->parent->on_hover.callback.chars, ctx->lexer->string);
    EXPECT_REQUIRED(ctx, ",");
    // ignore second argument by ignoring everything until ')'
    // this could obviously accept a lot of invalid syntax
    size_t num_parens = 1;
    do {
        ret = stb_c_lexer_get_token(ctx->lexer);
        if (!ret) {
            REPORT_FAILURE(ctx, ")");
            return false;
        }
        if (ctx->lexer->token == '(') {
            num_parens++;
        } else if (ctx->lexer->token == ')') {
            num_parens--;
        }
    } while (num_parens);
    EXPECT_REQUIRED(ctx, ";");
    return true;
}

static ui_element_t* parse_tree_r(parse_ctx_t* ctx)
{
    ui_element_t* me = NULL;
    const char* possible[] = { "CLAY", "CLAY_TEXT", "Clay_OnHover", "}" };
    int token = expect_tokens(ctx, possible, numberof(possible));
    switch (token) {
    case 0:
        me = parse_element_declaration(ctx);
        break;
    case 1:
        EXPECT_REQUIRED(ctx, "(");
        me = parse_text(ctx);
        if (!me)
            goto fail;
        EXPECT_REQUIRED(ctx, ")");
        EXPECT_REQUIRED(ctx, ";");
        break;
    case 2:
        if (!parse_on_hover(ctx)) goto fail;
        return (void*) SIZE_MAX;
    case 3:
        return me;
    default:
        REPORT_FAILURE(ctx, "CLAY, CLAY_TEXT, Clay_OnHover, or }");
        goto fail;
    }
    return me;
fail:
    if (me)
        ui_element_remove(me);
    return NULL;
}

ui_element_t* import_layout(const char* filename)
{
    ui_element_t* head = NULL;
    char *file_data = NULL, *storage = NULL;
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        goto cleanup;
    }
    fseek(f, 0, SEEK_END);
    int64_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    assert(size >= 0);
    file_data = (char*) malloc(size);
    storage = (char*) malloc(size);
    if (!file_data || !storage) {
        fprintf(stderr, "Unable to allocate memory for file data\n");
        goto cleanup;
    }
    if (fread(file_data, 1, size, f) != (size_t) size) {
        fprintf(stderr, "Unable to read %" PRId64 " bytes from file %s\n", size, filename);
        goto cleanup;
    }
    stb_lexer lexer = { 0 };
    stb_c_lexer_init(&lexer, file_data, file_data + size, storage, size);

    parse_ctx_t ctx = { .parent = NULL, .lexer = &lexer, .filename = filename };
    head = parse_tree_r(&ctx);

cleanup:
    free(file_data);
    free(storage);
    if (f) fclose(f);
    return head;
}
