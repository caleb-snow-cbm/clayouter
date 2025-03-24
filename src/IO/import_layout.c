#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "clay_enum_names.h"
#include "clay_struct_names.h"
#include "import_preprocessor.h"
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

/*
static bool parse_bool(parse_ctx_t* ctx, bool* out);
static bool parse_integral(parse_ctx_t* ctx, uint8_t* out, uint8_t size);
static bool parse_float(parse_ctx_t* ctx, float* out);
*/
static bool parse_string_literal(parse_ctx_t* ctx, dstring_t* s);
static bool parse_enum(parse_ctx_t* ctx, uint8_t* out, const enum_info_t* info);
static bool parse_struct(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info);
static bool parse_struct_members(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info);
static bool parse_struct_member(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info, size_t member_index);
static bool parse_union(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info);
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

void report_failure(parse_ctx_t* ctx, const char* expected)
{
    stb_lex_location loc;
    stb_c_lexer_get_location((ctx)->lexer, (ctx)->lexer->where_firstchar, &loc);
    fprintf(stderr, "Unexpected token at %s:%d:%d\n", (ctx)->filename, loc.line_number,
        loc.line_offset);
    fprintf(stderr, "Expected %s\n", expected);
}

#define EXPECT_REQUIRED(ctx, token)                                                                \
    do                                                                                             \
        if (!expect_token(ctx, token)) {                                                           \
            report_failure(ctx, token);                                                            \
            assert(0);\
            return false;                                                                          \
        }                                                                                          \
    while (0)

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

typedef enum {
    VALUE_TYPE_BOOL,
    VALUE_TYPE_INT,
    VALUE_TYPE_FLOAT,
} value_type_t;

typedef enum {
    OP_NONE,
    OP_ADD,
    OP_SUB,
    OP_MULT,
    OP_DIV
} operation_t;

typedef struct value_t {
    value_type_t type;
    union {
        float    f;
        int64_t  i;
        bool     b;
    };
} value_t;

void promote_to_float(value_t* v)
{
    switch (v->type) {
        case VALUE_TYPE_BOOL:
        v->f = (float) v->b;
        break;
        case VALUE_TYPE_INT:
        v->f = (float) v->i;
        break;
        default: break;
    }
    v->type = VALUE_TYPE_FLOAT;
}

void value_invert_sign(value_t* v)
{
    switch (v->type) {
        case VALUE_TYPE_BOOL:
        break;
        case VALUE_TYPE_INT:
        v->i = -v->i;
        break;
        case VALUE_TYPE_FLOAT:
        v->f = -v->f;
    }
}

value_t evaluate_simple_expression(value_t lhs, value_t rhs, operation_t op)
{
    value_t ret = { 0 };
    if (lhs.type == VALUE_TYPE_FLOAT) {
        promote_to_float(&rhs);
    } else if (rhs.type == VALUE_TYPE_FLOAT) {
        promote_to_float(&lhs);
    }
    switch (lhs.type) {
        case VALUE_TYPE_BOOL:
        case VALUE_TYPE_INT:
        ret.type = VALUE_TYPE_INT;
        switch (op) {
            case OP_ADD:
            ret.i = lhs.i + rhs.i;
            break;
            case OP_SUB:
            ret.i = lhs.i - rhs.i;
            break;
            case OP_MULT:
            ret.i = lhs.i * rhs.i;
            break;
            case OP_DIV:
            ret.i = lhs.i / rhs.i;
            break;
            default: break;
        }
        break;
        case VALUE_TYPE_FLOAT:
        ret.type = VALUE_TYPE_FLOAT;
        switch (op) {
            case OP_ADD:
            ret.f = lhs.f + rhs.f;
            break;
            case OP_SUB:
            ret.f = lhs.f - rhs.f;
            break;
            case OP_MULT:
            ret.f = lhs.f * rhs.f;
            break;
            case OP_DIV:
            ret.f = lhs.f / rhs.f;
            break;
            default: break;
        }
        break;
    }
    return ret;
}

value_t evaluate_expression(parse_ctx_t* ctx, bool* done)
{
    value_t lhs = { 0 };
    bool lhs_valid = false;
    value_t rhs;
    operation_t op = OP_NONE;
    while (*done == false) {
        char* before = ctx->lexer->parse_point;
        if (!stb_c_lexer_get_token(ctx->lexer)) {
            report_failure(ctx, "constant expression");
            return lhs;
        }
        switch (ctx->lexer->token) {
            case CLEX_intlit:
                if (op == OP_NONE) {
                    lhs.type = VALUE_TYPE_INT;
                    lhs.i = ctx->lexer->int_number;
                } else {
                    rhs.type = VALUE_TYPE_INT;
                    rhs.i = ctx->lexer->int_number;
                    lhs = evaluate_simple_expression(lhs, rhs, op);
                }
                lhs_valid = true;
                break;
            case CLEX_floatlit:
                if (op == OP_NONE) {
                    lhs.type = VALUE_TYPE_FLOAT;
                    lhs.f = (float) ctx->lexer->real_number;
                } else {
                    rhs.type = VALUE_TYPE_FLOAT;
                    rhs.f = (float) ctx->lexer->real_number;
                    lhs = evaluate_simple_expression(lhs, rhs, op);
                }
                lhs_valid = true;
                break;
            case '(':
                if (op == OP_NONE) {
                    if (lhs_valid) {
                        report_failure(ctx, "math operator");
                        return lhs;
                    }
                    lhs = evaluate_expression(ctx, done);
                    break;
                }
                rhs = evaluate_expression(ctx, done);
                if (ctx->lexer->token != ')') {
                    report_failure(ctx, ")");
                    return lhs;
                }
                lhs = evaluate_simple_expression(lhs, rhs, op);
                lhs_valid = true;
                break;
            case ')':
                if (!lhs_valid) {
                    report_failure(ctx, "expression before )");
                }
                return lhs;
            case '*':
                op = OP_MULT;
                break;
            case '/':
                op = OP_DIV;
                break;
            case '+':
                op = OP_ADD;
                rhs = evaluate_expression(ctx, done);
                lhs = evaluate_simple_expression(lhs, rhs, op);
                if (ctx->lexer->token == ')') return lhs;
                lhs_valid = true;
                break;
            case '-':
                if (!lhs_valid) {
                    lhs.i = -1;
                    lhs.type = VALUE_TYPE_INT;
                    lhs_valid = true;
                    op = OP_MULT;
                    break;
                }
                rhs = evaluate_expression(ctx, done);
                value_invert_sign(&rhs);
                lhs = evaluate_simple_expression(lhs, rhs, OP_ADD);
                lhs_valid = true;
                break;
            default:
                *done = true;
                ctx->lexer->parse_point = before;
                return lhs;
        }
    }
    return lhs;
}

static void write_bool(bool* out, value_t v)
{
    switch (v.type) {
    case VALUE_TYPE_BOOL:
        *out = v.b;
        break;
    case VALUE_TYPE_INT:
        *out = (bool) v.i;
        break;
    case VALUE_TYPE_FLOAT:
        *out = (bool) v.f;
        break;
    }
}

static void write_integral(uint8_t* out, size_t size, value_t v)
{
    int64_t tmp;
    switch (v.type) {
    case VALUE_TYPE_BOOL:
        tmp = v.b;
        break;
    case VALUE_TYPE_INT:
        tmp = v.i;
        break;
    case VALUE_TYPE_FLOAT:
        tmp = (int64_t) v.f;
        break;
    }
    switch (size) {
    case 1:
        *out = (uint8_t) tmp;
        break;
    case 2:
        *(uint16_t*) out = (uint16_t) tmp;
        break;
    case 4:
        *(uint32_t*) out = (uint32_t) tmp;
        break;
    case 8:
        *(uint64_t*) out = tmp;
        break;
    default:
        fprintf(stderr, "Invalid integer size %zu\n", size);
        break;
    }
}

static void write_float(float* out, value_t v)
{
    switch (v.type) {
    case VALUE_TYPE_BOOL:
        *out = (float) v.b;
        break;
    case VALUE_TYPE_INT:
        *out = (float) v.i;
        break;
    case VALUE_TYPE_FLOAT:
        *out = v.f;
        break;
    }
}

static void write_value(uint8_t* out, type_t expected_type, size_t size, value_t v)
{
    switch (expected_type) {
    case TYPE_BOOL:
        write_bool((bool*) out, v);
        break;
    case TYPE_INTEGRAL:
        write_integral(out, size, v);
        break;
    case TYPE_FLOAT:
        write_float((float*) out, v);
        break;
    default:
        fprintf(stderr, "unexpecteded type in write_value()\n");
    }
}

static bool parse_string_literal(parse_ctx_t* ctx, dstring_t* s)
{
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_dqstring) {
        report_failure(ctx, "string literal");
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
    value_t v;
    bool done = false;
    switch (info->info[member_index].type) {
    case TYPE_BOOL:
    case TYPE_INTEGRAL:
    case TYPE_FLOAT:
        v = evaluate_expression(ctx, &done);
        write_value(out + info->offsets[member_index], info->info[member_index].type, info->sizes[member_index], v);
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
        if (!parse_union(ctx,
                         out + info->offsets[member_index],
                         on_hover_out ? on_hover_out + info->offsets[member_index] : NULL,
                         info->info[member_index].struct_info)) {
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
    if (!parse_value(ctx, out, on_hover_out, info, member_index)) {
        return false;
    }
    if (on_hover_out && info->info[member_index].type != TYPE_STRUCT)
        memcpy(on_hover_out + info->offsets[member_index], out + info->offsets[member_index], info->sizes[member_index]);
    return true;
}

static bool parse_struct_members(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info)
{
    int braces = 1;
    for (size_t i = 0; i < info->count; ++i) {
        char* before = ctx->lexer->parse_point;
        for (stb_c_lexer_get_token(ctx->lexer); ctx->lexer->token == '{'; ++braces) {
            before = ctx->lexer->parse_point;
            if (!stb_c_lexer_get_token(ctx->lexer)) {
                report_failure(ctx, info->members[i]);
                return false;
            }
        }
        if (ctx->lexer->token == '}') {
            --braces;
            break;
        }
        if (ctx->lexer->token == '.') {
            // adjust i to member given
            if (!stb_c_lexer_get_token(ctx->lexer)) {
                report_failure(ctx, "struct member before EOF");
                return false;
            }
            bool found = false;
            for (size_t j = 0; j < info->count; ++j) {
                if (!strcmp(info->members[j], ctx->lexer->string)) {
                    found = true;
                    i = j;
                    break;
                }
            }
            if (!found) {
                report_failure(ctx, "struct member name");
            }
            EXPECT_REQUIRED(ctx, "=");
        } else {
            ctx->lexer->parse_point = before;
        }
        if (!parse_struct_member(ctx, out, on_hover_out, info, i)) {
            return false;
        }
        if (!stb_c_lexer_get_token(ctx->lexer)) return false;
        if (ctx->lexer->token == ',') continue;
        if (ctx->lexer->token == '}') {
            --braces;
            break;
        }
        report_failure(ctx, ", or } in struct definition");
    }
    assert(braces >= 0);
    for (; braces; --braces)
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
        return parse_struct_members(ctx, out, on_hover_out, info);
    } else if (token == 1) {
        int parens = 0;
        while (ctx->lexer->token == '(') {
            int ret = stb_c_lexer_get_token(ctx->lexer);
            if (!ret || (ctx->lexer->token == CLEX_id &&
                        strcmp(info->name, ctx->lexer->string))) {
                report_failure(ctx, info->name);
                return false;
            } else if (ctx->lexer->token == '(') {
                ++parens;
            }
        }
        EXPECT_REQUIRED(ctx, ")");
        EXPECT_REQUIRED(ctx, "{");
        if (!parse_struct_members(ctx, out, on_hover_out, info)) return false;
        for (; parens; --parens)
            EXPECT_REQUIRED(ctx, ")");
        return true;
    }
    report_failure(ctx, "{ or ( for struct definition");
    return false;
}

static bool parse_union_by_members(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info)
{
    char* prev = ctx->lexer->parse_point;
    if (!stb_c_lexer_get_token(ctx->lexer)) {
        report_failure(ctx, "union member");
        return false;
    }
    if (ctx->lexer->token == '.') {
        int ret = stb_c_lexer_get_token(ctx->lexer);
        if (!ret || ctx->lexer->token != CLEX_id) return false;
        for (size_t i = 0; i < info->count; ++i) {
            if (!strcmp(ctx->lexer->string, info->members[i])) {
                EXPECT_REQUIRED(ctx, "=");
                if (!parse_struct_member(ctx, out, on_hover_out, info, i)) return false;
                ret = stb_c_lexer_get_token(ctx->lexer);
                if (!ret) return false;
                if (ctx->lexer->token == ',') {
                    report_failure(ctx, "}, initialing subobjects of unions is not supported");
                    return false;
                }
                if (ctx->lexer->token == '}') return true;
                report_failure(ctx, "}");
            }
        }
        return false;
    }
    ctx->lexer->parse_point = prev;
    // do something else
    return false;
}

static bool parse_union_literal(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info)
{
    char* prev = ctx->lexer->parse_point;
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_id) {
        report_failure(ctx, "type of union member");
        return false;
    }
    for (size_t i = 0; i < info->count; ++i) {
        if (!strcmp(ctx->lexer->string, info->members[i])) {
            ctx->lexer->parse_point = prev;
            return parse_struct(ctx, out, on_hover_out, info->info[i].struct_info);
        }
    }
    report_failure(ctx, "type of union member");
    return false;
}

static bool parse_union(parse_ctx_t* ctx, uint8_t* out, uint8_t* on_hover_out, const struct_info_t* info)
{
    const char* possible[] = { "{", "(" };
    int token = expect_tokens(ctx, possible, numberof(possible));
    if (token == 0) {
        return parse_union_by_members(ctx, out, on_hover_out, info);
    } else if (token == 1) {
        return parse_union_literal(ctx, out, on_hover_out, info);
    }
    report_failure(ctx, "{ or ( for union definition");
    return false;
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
        report_failure(ctx, "a parent element before calling Clay_OnHover");
        return false;
    }
    ctx->parent->on_hover.enabled = true;
    EXPECT_REQUIRED(ctx, "(");
    int ret = stb_c_lexer_get_token(ctx->lexer);
    if (!ret || ctx->lexer->token != CLEX_id) {
        report_failure(ctx, "callback function");
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
            report_failure(ctx, ")");
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
    case 0: // CLAY
        me = parse_element_declaration(ctx);
        break;
    case 1: // CLAY_TEXT
        EXPECT_REQUIRED(ctx, "(");
        me = parse_text(ctx);
        if (!me)
            goto fail;
        EXPECT_REQUIRED(ctx, ")");
        EXPECT_REQUIRED(ctx, ";");
        break;
    case 2: // Clay_OnHover
        if (!parse_on_hover(ctx)) goto fail;
        return (void*) SIZE_MAX;
    case 3: // }
        return me;
    default:
        report_failure(ctx, "CLAY, CLAY_TEXT, Clay_OnHover, }, or #");
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
    char *file_data;
    stb_lexer lexer = { 0 };
    int64_t size;

    parse_ctx_t ctx = { .parent = NULL, .lexer = &lexer, .filename = filename };
    // First pass replaces macros
    if (!replace_macros(filename, &lexer, &file_data, &size)) return head;
    head = parse_tree_r(&ctx);

    free(file_data);
    free(lexer.string_storage);
    return head;
}
