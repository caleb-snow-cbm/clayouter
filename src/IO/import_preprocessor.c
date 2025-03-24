#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "stb_c_lexer.h"

#include "import_preprocessor.h"
#include "utilities.h"

#define MAX_ARGS    (8)

typedef struct {
    char* ptr;
    size_t cap;
    size_t size;
} arena_t;

typedef struct {
    char* expression;
    char* arguments[MAX_ARGS];
    int num_args;
    bool variadic;
} macro_definition_t;

typedef struct {
    char* key;
    macro_definition_t value;
} macro_t;

typedef struct {
    arena_t* a;
    stb_lexer* lex;
    const char* filename;
    macro_t** map;
} ctx_t;

typedef struct {
    char* ptr;
    size_t size;
    size_t capacity;
} buffer_t;

static bool replace_macro(ctx_t ctx, buffer_t* output, macro_definition_t* macro);

static arena_t arena_init(size_t capacity)
{
    return (arena_t) { .ptr = malloc_assert(capacity), .cap = capacity, .size = 0 };
}

static char* arena_alloc(arena_t* a, size_t num_bytes)
{
    assert(a->size + num_bytes < a->cap);
    char* ret = a->ptr + a->size;
    a->size += num_bytes;
    a->ptr[a->size++] = '\0';
    return ret;
}

static char* arena_copy_string(arena_t* a, const char* s)
{
    size_t len = strlen(s);
    assert(len + a->size <= a->cap);
    char* ret = a->ptr + a->size;
    strcpy(ret, s);
    a->size += len + 1;
    return ret;
}

static char* arena_copy_string_n(arena_t* a, const char* s, size_t n)
{
    assert(a->size + n + 1 <= a->cap);
    char* ret = a->ptr + a->size;
    memcpy(a->ptr + a->size, s, n);
    a->size += n;
    a->ptr[a->size] = '\0';
    return ret;
}

static void arena_append_char(arena_t* a, char c)
{
    assert(a->size + 1 < a->cap);
    a->ptr[a->size++] = c;
    a->ptr[a->size] = '\0';
}

static void arena_destory(arena_t a)
{
    free(a.ptr);
}

static char* arena_append_point(arena_t* a)
{
    return a->ptr + a->size;
}

static void report_failure(const char* filename, stb_lexer* lex, const char* expected)
{
    stb_lex_location loc;
    stb_c_lexer_get_location(lex, lex->where_firstchar, &loc);
    fprintf(stderr, "Unexpected token at %s:%d:%d\n", filename, loc.line_number, loc.line_offset);
    fprintf(stderr, "Expected %s\n", expected);
}

static void append_bytes_to_buffer(buffer_t* buffer, const char* bytes, size_t num_bytes)
{
    while (buffer->size + num_bytes > buffer->capacity) {
        buffer->capacity *= 2;
        REALLOC_ASSERT(buffer->ptr, buffer->capacity);
    }
    memcpy(buffer->ptr + buffer->size, bytes, num_bytes);
    buffer->size += num_bytes;
}

static bool read_file_data(const char* filename, char** file_data, int64_t* size)
{
    bool ret = false;
    *file_data = NULL;
    FILE* f = fopen(filename, "rb");
    if (f == NULL) {
        fprintf(stderr, "Unable to open %s\n", filename);
        return false;
    }
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, SEEK_SET);
    assert(*size >= 0);
    *file_data = (char*) malloc(*size);
    if (!file_data) {
        fprintf(stderr, "Unable to allocate memory for file data\n");
        goto cleanup;
    }
    if (fread(*file_data, 1, *size, f) != (size_t) *size) {
        fprintf(stderr, "Unable to read %" PRId64 " bytes from file %s\n", *size, filename);
        free(*file_data);
        goto cleanup;
    }
    ret = true;
cleanup:
    if (f) fclose(f);
    return ret;
}

static int expand_arguments(ctx_t ctx, macro_definition_t* macro, char* args[MAX_ARGS])
{
    stb_lexer* lex = ctx.lex;
    if (!stb_c_lexer_get_token(lex) || lex->token != '(') {
        report_failure(ctx.filename, lex, "macro arguments");
        return -1;
    }
    int num_parens = 0;
    for (int i = 0; i < MAX_ARGS; ++i) {
        args[i] = arena_append_point(ctx.a);
        for (;;) {
            char* prev = lex->parse_point;
            if (!stb_c_lexer_get_token(lex)) {
                report_failure(ctx.filename, lex, "macro arguments");
                return -1;
            }
            if (num_parens == 0 && lex->token == ',') {
                if (!macro->variadic && i == macro->num_args - 1) {
                    report_failure(ctx.filename, lex, "), too many macro arguments given");
                    return -1;
                }
                arena_append_char(ctx.a, '\0');
                break;
            } else if (num_parens == 0 && lex->token == ')') {
                if (!macro->variadic && i < macro->num_args - 1) {
                    report_failure(ctx.filename, lex, "more macro arguments");
                    fprintf(stderr, "%d given, %d expected\n", i + 1, macro->num_args);
                    return -1;
                }
                arena_append_char(ctx.a, '\0');
                return i + 1;
            }
            if (lex->token == ')' || lex->token == '}') {
                --num_parens;
            } else if (lex->token == '{' || lex->token == '(') {
                ++num_parens;
            }
            int index = shgeti(*ctx.map, lex->string);
            if (lex->token != CLEX_id || index == -1) {
                arena_copy_string_n(ctx.a, prev, lex->parse_point - prev);
                continue;
            }
            buffer_t idk = { .ptr = malloc(64), .capacity = 64, .size = 0 };
            replace_macro(ctx, &idk, &(*ctx.map)[index].value);
            arena_append_char(ctx.a, '\0');
            arena_copy_string_n(ctx.a, idk.ptr, idk.size);
            free(idk.ptr);
        }
    }
    report_failure(ctx.filename, lex, "), too many macro arguments, max is 8");
    return -1;
}

static bool replace_macro(ctx_t ctx, buffer_t* output, macro_definition_t* macro)
{
    if (macro->num_args == 0 && !macro->variadic) {
        append_bytes_to_buffer(output,
                               macro->expression,
                               strlen(macro->expression));
        return true;
    }
    // get arguments
    char* args[MAX_ARGS];
    int args_given = expand_arguments(ctx, macro, args);
    if (args_given == -1) {
        return false;
    }
    // replace arguments
    char storage[64] = { 0 };
    stb_lexer macro_lexer = { 0 };
    char* exp = macro->expression;
    size_t exp_len = strlen(exp);
    stb_c_lexer_init(&macro_lexer, exp, exp + exp_len, storage, 64);
    buffer_t argument_buffer = { 0 };
    stb_lexer argument_lexer = { 0 };

    ctx.lex = &macro_lexer;

    while (true) {
        char* before = ctx.lex->parse_point;
        if (!stb_c_lexer_get_token(ctx.lex)) {
            if (ctx.lex == &argument_lexer) {
                free(argument_buffer.ptr);
                ctx.lex = &macro_lexer;
                continue;
            }
            break;
        }
        if (ctx.lex->token == CLEX_id) {
            if (macro->variadic && !strncmp("__VA_ARGS__", ctx.lex->string, 11)) {
                for (int i = macro->num_args; i < args_given; ++i) {
                    append_bytes_to_buffer(output, args[i], strlen(args[i]));
                    if (i != args_given - 1)
                        append_bytes_to_buffer(output, ",", 1);
                }
                continue;
            }
            bool found = false;
            int index = shgeti(*ctx.map, ctx.lex->string);
            if (index != -1) {
                argument_buffer.ptr = malloc(128);
                argument_buffer.capacity = 128;
                replace_macro(ctx, &argument_buffer, &(*ctx.map)[index].value);
                ctx.lex = &argument_lexer;
                stb_c_lexer_init(ctx.lex,
                                 argument_buffer.ptr,
                                 argument_buffer.ptr + argument_buffer.size,
                                 arena_alloc(ctx.a, 64),
                                 64);
                continue;
            }
            for (int i = 0; i < macro->num_args; ++i) {
                if (!strcmp(ctx.lex->string, macro->arguments[i])) {
                    found = true;
                    append_bytes_to_buffer(output, args[i], strlen(args[i]));
                    break;
                }
            }
            if (!found)
                append_bytes_to_buffer(output, before, ctx.lex->parse_point - before);
        } else {
            append_bytes_to_buffer(output, before, ctx.lex->parse_point - before);
        }
    }
    return true;
}

static bool gather_macro_definitions(ctx_t* ctx)
{
    stb_lexer* lex = ctx->lex;
    while (lex->parse_point + 1 < lex->eof) {
        if (*lex->parse_point != '\n' || *(lex->parse_point + 1) != '#') {
            ++lex->parse_point;
            continue;
        }
        lex->parse_point += 2; // \n#
        if (!stb_c_lexer_get_token(lex) || lex->token != CLEX_id) {
            report_failure(ctx->filename, lex, "preprocessor directive");
            return false;
        }
        if (strcmp("define", lex->string)) {
            continue;
        }
        if (!stb_c_lexer_get_token(lex) || lex->token != CLEX_id) {
            report_failure(ctx->filename, lex, "macro");
            return false;
        }
        macro_definition_t m = { 0 };
        char* macro = arena_copy_string(ctx->a, lex->string);
        switch (*lex->parse_point) {
        case '(':
            stb_c_lexer_get_token(lex); // (
            do {
                while (lex->parse_point != lex->eof && isspace(*lex->parse_point))
                    ++lex->parse_point;
                if (!strncmp("...", lex->parse_point, 3)) {
                    m.variadic = true;
                    lex->parse_point += 3;
                    if (!stb_c_lexer_get_token(lex) || lex->token != ')') {
                        report_failure(ctx->filename, lex, ") after variadic macro definition");
                        return false;
                    }
                    break;
                }
                if (!stb_c_lexer_get_token(lex) || lex->token != CLEX_id) {
                    report_failure(ctx->filename, lex, "macro argument");
                    return false;
                }
                m.arguments[m.num_args++] = arena_copy_string(ctx->a, lex->string);
            } while (stb_c_lexer_get_token(lex) && lex->token == ',');

            if (lex->token != ')') {
                report_failure(ctx->filename, lex, "macro arguments");
                return false;
            }
            // fallthrough
        case ' ':
        case '\t':
        case '\r':
        case '\n':
        case '\\':
            m.expression = arena_append_point(ctx->a);
            while (lex->parse_point < lex->eof) {
                if (*lex->parse_point == '\r' || *lex->parse_point == '\n') {
                    arena_append_char(ctx->a, '\0');
                    break;
                }
                if (*lex->parse_point == '\\') {
                    if (*(lex->parse_point + 1) == '\r') {
                        lex->parse_point += 1;
                    }
                    if (*(lex->parse_point + 1) == '\n') {
                        lex->parse_point += 2;
                    }
                    continue;
                }
                arena_append_char(ctx->a, *lex->parse_point);
                lex->parse_point++;
            }
            ctx->a->size++;
            shput(*ctx->map, macro, m);
            break;
        default:
            report_failure(ctx->filename, lex, "macro definition");
            return false;
        }
    }
    return true;
}

bool replace_macros(const char* filename, stb_lexer* lex, char** file_data, int64_t* size)
{
    bool ret = false;
    macro_t* map = NULL;

    char* clay_h_data = NULL;
    int64_t clay_h_size;
    arena_t clay_arena = { 0 };
    if (!read_file_data("clay/clay.h", &clay_h_data, &clay_h_size)) {
        fprintf(stderr, "Unable to open or read clay.h, macros will not be available\n");
    } else {
        stb_lexer clay_lexer;
        char* storage = malloc(256);
        stb_c_lexer_init(&clay_lexer, clay_h_data, clay_h_data + clay_h_size, storage, 256);
        clay_arena = arena_init(clay_h_size);
        ctx_t clay_h_ctx = {
            .filename = "clay.h",
            .lex = &clay_lexer,
            .map = &map,
            .a = &clay_arena
        };
        if (!gather_macro_definitions(&clay_h_ctx)) {
            fprintf(stderr, "Error collecting clay.h definitions\n");
            shfree(map);
            map = NULL;
        }
        // These macros are easier to handle in a custom manner
        shdel(map, "CLAY");
        shdel(map, "CLAY_TEXT");
        shdel(map, "CLAY_TEXT_CONFIG");
        shdel(map, "CLAY_STRING");
        shdel(map, "CLAY_ID");
        int index = shgeti(map, "CLAY__CONFIG_WRAPPER");
        map[index].value.expression = arena_copy_string(&clay_arena, "(type) { __VA_ARGS__ }");
        free(storage);
    }

    buffer_t output_buffer = { 0 };
    if (!read_file_data(filename, file_data, size)) return false;
    char* storage = (char*) malloc_assert(*size);
    arena_t arena = arena_init(*size);
    stb_c_lexer_init(lex, *file_data, (*file_data) + *size, storage, *size);

    ctx_t ctx = {
        .a = &arena,
        .lex = lex,
        .filename = filename,
        .map = &map
    };

    // gather all macro definitions first
    if (!gather_macro_definitions(&ctx)) goto cleanup;

    output_buffer.ptr = (char*) malloc(*size);
    output_buffer.capacity = *size;
    output_buffer.size = 0;
    stb_c_lexer_init(lex, *file_data, (*file_data) + *size, storage, *size);


    while (true) {
        char* before = lex->parse_point;
        if (!stb_c_lexer_get_token(lex)) {
            break;
        }
        ptrdiff_t index = shgeti(map, lex->string);
        if (lex->token != CLEX_id || index == -1) {
            append_bytes_to_buffer(&output_buffer, before, lex->parse_point - before);
            continue;
        }
        replace_macro(ctx, &output_buffer, &map[index].value);
    }

    char* tmp = *file_data;
    *file_data = output_buffer.ptr;
    output_buffer.ptr = tmp;
    *size = output_buffer.size;
    ret = true;
    stb_c_lexer_init(lex, *file_data, *file_data + *size, malloc(128), 128);
cleanup:
    shfree(map);
    free(storage);
    free(output_buffer.ptr);
    free(clay_h_data);
    arena_destory(arena);
    arena_destory(clay_arena);
    return ret;
}
