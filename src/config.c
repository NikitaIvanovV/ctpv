#include "ctpv.h"
#include "lexer.h"
#include "error.h"
#include "config.h"
#include "preview.h"

#define CHECK(f, cond)       \
    do {                     \
        enum Status x = (f); \
        if (!(cond))         \
            return x;        \
    } while (0)

#define CHECK_OK(f)   CHECK(f, x == STAT_OK)
#define CHECK_NULL(f) CHECK(f, x == STAT_NULL)

#define EXPECT(c, x)     CHECK_OK(expect(c, x))
#define ACCEPT(c, x)     CHECK_OK(accept(c, x))
#define NOT_ACCEPT(c, x) CHECK_NULL(accept(c, x))

#define DEF_OPTION(name, type, val) { (#name), (type), { .val = &ctpv.opts.name } }
#define DEF_OPTION_BOOL(name)       DEF_OPTION(name, OPTION_BOOL, i)
#define DEF_OPTION_INT(name)        DEF_OPTION(name, OPTION_INT, i)
#define DEF_OPTION_STR(name)        DEF_OPTION(name, OPTION_STR, s)

#define TYPE_SET_EMPTY (struct TypeSet){ NULL, NULL, NULL }

struct Parser {
    Lexer *lexer;
    Token token;
    VectorPreview *previews;
};

struct Option {
    char *name;
    enum {
        OPTION_BOOL,
        OPTION_INT,
        OPTION_STR,
    } arg_type;
    union {
        int *i;
        char **s;
    } arg_val;
};

struct TypeSet {
    char *type, *subtype, *ext;
};

enum Status {
    STAT_OK,
    STAT_ERR,
    STAT_NULL,
};

static struct Option options[] = {
    DEF_OPTION_BOOL(forcekitty),
    DEF_OPTION_BOOL(forcekittyanim),
    DEF_OPTION_BOOL(forcechafa),
    DEF_OPTION_BOOL(noimages),
    DEF_OPTION_BOOL(nosymlinkinfo),
    DEF_OPTION_BOOL(autochafa),
    DEF_OPTION_BOOL(showgpg),
};

static void any_type_null(char **s)
{
    if (*s && strcmp(*s, any_type) == 0)
        *s = NULL;
}

static void add_preview(Parser *ctx, char *name, char *script, struct TypeSet *set,
                        unsigned int set_len)
{
    if (!ctx->previews)
        return;

    size_t script_len = strlen(script) + 1;

    for (unsigned int i = 0; i < set_len; i++) {
        any_type_null(&set[i].type);
        any_type_null(&set[i].subtype);

        Preview p = (Preview){
            .name = name,
            .script = script,
            .script_len = script_len,
            .ext = set[i].ext,
            .type = set[i].type,
            .subtype = set[i].subtype,
            .attrs = PREV_ATTR_NONE,
            .order = 1, /* custom previews are always prioritized */
            .priority = 0
        };

        vectorPreview_append(ctx->previews, p);
    }
}

static RESULT add_priority(Parser *ctx, char *name, int priority)
{
    if (!ctx->previews)
        return OK;

    int found = 0;

    for (size_t i = 0; i < ctx->previews->len; i++) {
        if (strcmp(ctx->previews->buf[i].name, name) != 0)
            continue;

        ctx->previews->buf[i].priority = priority;
        found = 1;
    }

    return found ? OK : ERR;
}

static RESULT remove_preview(Parser *ctx, char *name)
{
    if (!ctx->previews)
        return OK;

    int found = 0;

    for (ssize_t i = ctx->previews->len - 1; i >= 0; i--) {
        if (strcmp(ctx->previews->buf[i].name, name) != 0)
            continue;

        vectorPreview_remove(ctx->previews, i);
        found = 1;
    }

    return found ? OK : ERR;
}

static inline void next_token(Parser *ctx)
{
    ctx->token = lexer_get_token(ctx->lexer);
}

static enum Status accept(Parser *ctx, enum TokenType type)
{
    if (ctx->token.type == type) {
        next_token(ctx);
        return STAT_OK;
    }

    return STAT_NULL;
}

static enum Status expect(Parser *ctx, enum TokenType type)
{
    if (accept(ctx, type) == STAT_OK)
        return STAT_OK;

    if (ctx->token.type == TOK_ERR)
        return STAT_ERR;

    PARSEERROR(ctx->token, "unexpected token: %s, expected: %s",
               lexer_token_type_str(ctx->token.type),
               lexer_token_type_str(type));
    return STAT_ERR;
}

static enum Status preview_type_ext(Parser *ctx, char **ext)
{
    ACCEPT(ctx, TOK_DOT);

    Token tok = ctx->token;
    EXPECT(ctx, TOK_STR);
    *ext = tok.val.s;

    return STAT_OK;
}

static enum Status preview_type_mime_part(Parser *ctx, char **s)
{
    NOT_ACCEPT(ctx, TOK_STAR);

    Token tok = ctx->token;
    EXPECT(ctx, TOK_STR);
    *s = tok.val.s;

    return STAT_OK;
}

static enum Status preview_type_mime(Parser *ctx, char **type, char **subtype)
{
    CHECK_OK(preview_type_mime_part(ctx, type));
    EXPECT(ctx, TOK_SLASH);
    CHECK_OK(preview_type_mime_part(ctx, subtype));

    return STAT_OK;
}

static inline void num_is_text(Parser *ctx)
{
    lexer_set_opts(ctx->lexer, LEX_OPT_NUMISTEXT);
}

static inline void reset_lexer_opts(Parser *ctx)
{
    lexer_set_opts(ctx->lexer, LEX_OPT_NONE);
}

static enum Status preview_type(Parser *ctx, struct TypeSet *set)
{
    enum Status ret;

    *set = TYPE_SET_EMPTY;

    num_is_text(ctx);

    if ((ret = preview_type_ext(ctx, &set->ext)) != STAT_NULL)
        goto exit;

    ret = preview_type_mime(ctx, &set->type, &set->subtype);

exit:
    reset_lexer_opts(ctx);
    return ret;
}

static struct Option *get_option(char *name)
{
    for (size_t i = 0; i < LEN(options); i++) {
        if (strcmp(name, options[i].name) == 0)
            return options + i;
    }

    return NULL;
}

static enum Status cmd_set(Parser *ctx)
{
    Token name = ctx->token;
    EXPECT(ctx, TOK_STR);

    struct Option *opt = get_option(name.val.s);
    if (!opt) {
        PARSEERROR(name, "option '%s' does not exist", name.val.s);
        return STAT_ERR;
    }

    Token value = ctx->token;

    switch (opt->arg_type) {
    case OPTION_BOOL:
        *opt->arg_val.i = accept(ctx, TOK_INT) == STAT_OK ? value.val.i : 1;
        break;
    case OPTION_INT:
        EXPECT(ctx, TOK_INT);
        *opt->arg_val.i = value.val.i;
        break;
    case OPTION_STR:
        EXPECT(ctx, TOK_STR);
        *opt->arg_val.s = value.val.s;
        break;
    default:
        PRINTINTERR("unknown type: %d", opt->arg_type);
        abort();
    }

    return STAT_OK;
}

static enum Status cmd_preview(Parser *ctx)
{
    Token name = ctx->token;
    EXPECT(ctx, TOK_STR);

    struct TypeSet types[16];
    unsigned int types_len = 0;

    while (accept(ctx, TOK_BLK_OPEN) == STAT_NULL) {
        if (types_len >= LEN(types)) {
            PARSEERROR(name, "a preview can only have up through %lu types",
                       LEN(types));
            return STAT_ERR;
        }

        CHECK_OK(preview_type(ctx, types + types_len++));
    }

    Token script = ctx->token;
    EXPECT(ctx, TOK_STR);

    EXPECT(ctx, TOK_BLK_CLS);

    add_preview(ctx, name.val.s, script.val.s, types, types_len);
    return STAT_OK;
}

static enum Status cmd_priority(Parser *ctx)
{
    Token name = ctx->token;
    EXPECT(ctx, TOK_STR);

    Token number = ctx->token;
    enum Status i = accept(ctx, TOK_INT) == STAT_OK ? number.val.i : 1;

    if (add_priority(ctx, name.val.s, i) != OK) {
        PARSEERROR(name, "preview '%s' not found", name.val.s);
        return STAT_ERR;
    }

    return STAT_OK;
}

static enum Status cmd_remove(Parser *ctx)
{
    Token name = ctx->token;
    EXPECT(ctx, TOK_STR);

    if (remove_preview(ctx, name.val.s) != OK) {
        PARSEERROR(name, "preview '%s' not found", name.val.s);
        return STAT_ERR;
    }

    return STAT_OK;
}

static enum Status command(Parser *ctx)
{
    Token cmd = ctx->token;
    EXPECT(ctx, TOK_STR);

    if (strcmp(cmd.val.s, "set") == 0)
        return cmd_set(ctx);
    else if (strcmp(cmd.val.s, "preview") == 0)
        return cmd_preview(ctx);
    else if (strcmp(cmd.val.s, "priority") == 0)
        return cmd_priority(ctx);
    else if (strcmp(cmd.val.s, "remove") == 0)
        return cmd_remove(ctx);

    PARSEERROR(cmd, "unknown command: %s", cmd.val.s);
    return STAT_ERR;
}

static void newlines(Parser *ctx)
{
    while (1) {
        if (accept(ctx, TOK_NEW_LN) != STAT_OK)
            break;
    }
}

static enum Status end(Parser *ctx)
{
    NOT_ACCEPT(ctx, TOK_EOF);
    EXPECT(ctx, TOK_NEW_LN);

    newlines(ctx);

    return STAT_OK;
}

static enum Status commands(Parser *ctx)
{
    newlines(ctx);

    while (1) {
        NOT_ACCEPT(ctx, TOK_EOF);
        CHECK_OK(command(ctx));
        CHECK_OK(end(ctx));
    }
}

static RESULT parse(Parser *ctx)
{
#ifdef DEBUG_LEXER
    while (1) {
        next_token(ctx);
        if (token.type == TOK_EOF)
            break;
        printf("%s", lexer_token_type_str(token.type));
        switch (token.type) {
        case TOK_INT:
            printf(": %d\n", token.val.i);
            break;
        case TOK_STR:
            printf(": %s\n", token.val.s);
            break;
        default:
            puts("");
            break;
        }
    }
#endif
#ifndef DEBUG_LEXER
    next_token(ctx);
    if (commands(ctx) == STAT_ERR)
        return ERR;
#endif

    return OK;
}

RESULT config_load(Parser **ctx, VectorPreview *prevs, char *filename)
{
    enum Result ret = OK;

    FILE *f;
    ERRCHK_GOTO_ERN(!(f = fopen(filename, "r")), ret, exit);

    if (!(*ctx = malloc(sizeof(**ctx)))) {
        FUNCFAILED("malloc", strerror(errno));
        abort();
    }

    (*ctx)->lexer = lexer_init(f);
    (*ctx)->previews = prevs;

    ERRCHK_GOTO_OK(parse(*ctx), ret, file);

file:
    fclose(f);

exit:
    return ret;
}

void config_cleanup(Parser *ctx)
{
    if (!ctx->lexer)
        return;

    lexer_free(ctx->lexer);
    ctx->lexer = NULL;

    free(ctx);
}
