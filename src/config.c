#include "ctpv.h"
#include "lexer.h"
#include "error.h"
#include "preview.h"

#define CHECK(f, cond) \
    do {               \
        int x = (f);   \
        if (!(cond))   \
            return x;  \
    } while (0)

#define CHECK_OK(f)   CHECK(f, x == STAT_OK)
#define CHECK_NULL(f) CHECK(f, x == STAT_NULL)

#define EXPECT(x)     CHECK_OK(expect(x))
#define ACCEPT(x)     CHECK_OK(accept(x))
#define NOT_ACCEPT(x) CHECK_NULL(accept(x))

#define DEF_OPTION(name, type, val) { (#name), (type), { .val = &ctpv.opts.name } }
#define DEF_OPTION_BOOL(name)       DEF_OPTION(name, OPTION_BOOL, i)
#define DEF_OPTION_INT(name)        DEF_OPTION(name, OPTION_INT, i)
#define DEF_OPTION_STR(name)        DEF_OPTION(name, OPTION_STR, s)

#define TYPE_SET_EMPTY (struct TypeSet){ NULL, NULL, NULL }

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

enum {
    STAT_OK,
    STAT_ERR,
    STAT_NULL,
};

static Lexer *lexer;
static Token token;
static VectorPreview *previews;

static struct Option options[] = {
    DEF_OPTION_BOOL(forcekitty),
    DEF_OPTION_BOOL(forcekittyanim),
    DEF_OPTION_BOOL(forcechafa),
    DEF_OPTION_BOOL(noimages),
    DEF_OPTION_BOOL(nosymlinkinfo),
    DEF_OPTION_BOOL(showgpg),
};

static void any_type_null(char **s)
{
    if (*s && strcmp(*s, any_type) == 0)
        *s = NULL;
}

static void add_preview(char *name, char *script, struct TypeSet *set,
                        unsigned int set_len)
{
    if (!previews)
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

        vectorPreview_append(previews, p);
    }
}

static int add_priority(char *name, int priority)
{
    if (!previews)
        return OK;

    int found = 0;

    for (size_t i = 0; i < previews->len; i++) {
        if (strcmp(previews->buf[i].name, name) != 0)
            continue;

        previews->buf[i].priority = priority;
        found = 1;
    }

    return found ? OK : ERR;
}

static int remove_preview(char *name)
{
    if (!previews)
        return OK;

    int found = 0;

    for (ssize_t i = previews->len - 1; i >= 0; i--) {
        if (strcmp(previews->buf[i].name, name) != 0)
            continue;

        vectorPreview_remove(previews, i);
        found = 1;
    }

    return found ? OK : ERR;
}

static inline void next_token(void)
{
    token = lexer_get_token(lexer);
}

static int accept(enum TokenType type)
{
    if (token.type == type) {
        next_token();
        return STAT_OK;
    }

    return STAT_NULL;
}

static int expect(enum TokenType type)
{
    if (accept(type) == STAT_OK)
        return STAT_OK;

    if (token.type == TOK_ERR)
        return STAT_ERR;

    PARSEERROR(token, "unexpected token: %s, expected: %s",
               lexer_token_type_str(token.type),
               lexer_token_type_str(type));
    return STAT_ERR;
}

static int preview_type_ext(char **ext)
{
    ACCEPT(TOK_DOT);

    Token tok = token;
    EXPECT(TOK_STR);
    *ext = tok.val.s;

    return STAT_OK;
}

static int preview_type_mime_part(char **s)
{
    NOT_ACCEPT(TOK_STAR);

    Token tok = token;
    EXPECT(TOK_STR);
    *s = tok.val.s;

    return STAT_OK;
}

static int preview_type_mime(char **type, char **subtype)
{
    CHECK_OK(preview_type_mime_part(type));
    EXPECT(TOK_SLASH);
    CHECK_OK(preview_type_mime_part(subtype));

    return STAT_OK;
}

static inline void num_is_text(void)
{
    lexer_set_opts(lexer, LEX_OPT_NUMISTEXT);
}

static inline void reset_lexer_opts(void)
{
    lexer_set_opts(lexer, LEX_OPT_NONE);
}

static int preview_type(struct TypeSet *set)
{
    int ret;

    *set = TYPE_SET_EMPTY;

    num_is_text();

    if ((ret = preview_type_ext(&set->ext)) != STAT_NULL)
        goto exit;

    ret = preview_type_mime(&set->type, &set->subtype);

exit:
    reset_lexer_opts();
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

static int cmd_set(void)
{
    Token name = token;
    EXPECT(TOK_STR);

    struct Option *opt = get_option(name.val.s);
    if (!opt) {
        PARSEERROR(name, "option '%s' does not exist", name.val.s);
        return STAT_ERR;
    }

    Token value = token;

    switch (opt->arg_type) {
    case OPTION_BOOL:
        *opt->arg_val.i = accept(TOK_INT) == STAT_OK ? value.val.i : 1;
        break;
    case OPTION_INT:
        EXPECT(TOK_INT);
        *opt->arg_val.i = value.val.i;
        break;
    case OPTION_STR:
        EXPECT(TOK_STR);
        *opt->arg_val.s = value.val.s;
        break;
    default:
        PRINTINTERR("unknowm type: %d", opt->arg_type);
        abort();
    }

    return STAT_OK;
}

static int cmd_preview(void)
{
    Token name = token;
    EXPECT(TOK_STR);

    struct TypeSet types[16];
    unsigned int types_len = 0;

    while (accept(TOK_BLK_OPEN) == STAT_NULL) {
        if (types_len >= LEN(types)) {
            PARSEERROR(name, "a preview can only have up through %lu types",
                       LEN(types));
            return STAT_ERR;
        }

        CHECK_OK(preview_type(types + types_len++));
    }

    Token script = token;
    EXPECT(TOK_STR);

    EXPECT(TOK_BLK_CLS);

    add_preview(name.val.s, script.val.s, types, types_len);
    return STAT_OK;
}

static int cmd_priority(Token tok)
{
    Token name = token;
    EXPECT(TOK_STR);

    Token number = token;
    int i = accept(TOK_INT) == STAT_OK ? number.val.i : 1;

    if (add_priority(name.val.s, i) != OK) {
        PARSEERROR(name, "preview '%s' not found", name.val.s);
        return STAT_ERR;
    }

    return STAT_OK;
}

static int cmd_remove(Token tok)
{
    Token name = token;
    EXPECT(TOK_STR);

    if (remove_preview(name.val.s) != OK) {
        PARSEERROR(name, "preview '%s' not found", name.val.s);
        return STAT_ERR;
    }

    return STAT_OK;
}

static int command(void)
{
    Token cmd = token;
    EXPECT(TOK_STR);

    if (strcmp(cmd.val.s, "set") == 0)
        return cmd_set();
    else if (strcmp(cmd.val.s, "preview") == 0)
        return cmd_preview();
    else if (strcmp(cmd.val.s, "priority") == 0)
        return cmd_priority(cmd);
    else if (strcmp(cmd.val.s, "remove") == 0)
        return cmd_remove(cmd);

    PARSEERROR(cmd, "unknown command: %s", cmd.val.s);
    return STAT_ERR;
}

static void newlines(void)
{
    while (1) {
        if (accept(TOK_NEW_LN) != STAT_OK)
            break;
    }
}

static int end(void)
{
    NOT_ACCEPT(TOK_EOF);
    EXPECT(TOK_NEW_LN);

    newlines();

    return STAT_OK;
}

static int commands(void)
{
    newlines();

    while (1) {
        NOT_ACCEPT(TOK_EOF);
        CHECK_OK(command());
        CHECK_OK(end());
    }
}

static int parse(void)
{
#ifdef DEBUG_LEXER
    while (1) {
        next_token();
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
    next_token();
    if (commands() == STAT_ERR)
        return ERR;
#endif

    return OK;
}

int config_load(VectorPreview *prevs, char *filename)
{
    int ret = OK;

    FILE *f;
    ERRCHK_GOTO_ERN(!(f = fopen(filename, "r")), ret, exit);

    lexer = lexer_init(f);
    previews = prevs;

    ERRCHK_GOTO_OK(parse(), ret, file);

file:
    fclose(f);

exit:
    return ret;
}

void config_cleanup(void)
{
    if (!lexer)
        return;

    lexer_free(lexer);
    lexer = NULL;
}
