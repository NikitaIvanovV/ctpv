#include "lexer.h"
#include "error.h"
#include "preview.h"

#define CHECK(f, cond) \
    do {               \
        int x = (f);   \
        if (cond)      \
            return x;  \
    } while (0)

#define CHECK_OK(f)      CHECK(f, x != STAT_OK)
#define CHECK_NULL(f)    CHECK(f, x != STAT_NULL)
#define CHECK_OK_NULL(f) CHECK(f, x != STAT_OK || x != STAT_NULL)

enum {
    STAT_OK,
    STAT_ERR,
    STAT_NULL,
};

static Lexer *lexer;
static Token token;
static VectorPreview *previews;
static char *any_type;

static void add_preview(char *name, char *script, char *type, char *subtype,
                        char *ext)
{
    if (type && strcmp(type, any_type) == 0)
        type = NULL;

    if (subtype && strcmp(subtype, any_type) == 0)
        subtype = NULL;

    Preview p = (Preview){
        .name = name,
        .script = script,
        .script_len = strlen(script),
        .type = type,
        .subtype = subtype,
        .ext = ext,
        .priority = 1 /* custom previews are always prioritized */
    };

    vectorPreview_append(previews, p);
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

static inline char *get_str(Token tok)
{
    return lexer_get_string(lexer, tok);
}

static int preview_type_ext(char **ext)
{
    CHECK_OK(accept(TOK_DOT));

    Token tok = token;
    CHECK_OK(expect(TOK_STR));
    *ext = get_str(tok);

    return STAT_OK;
}

static int preview_type_mime_part(char **s)
{
    *s = NULL;
    CHECK_NULL(accept(TOK_STAR));

    Token t = token;
    CHECK_OK(expect(TOK_STR));
    *s = get_str(t);

    return STAT_OK;
}

static int preview_type_mime(char **type, char **subtype)
{
    CHECK_OK(preview_type_mime_part(type));
    CHECK_OK(expect(TOK_SLASH));
    CHECK_OK(preview_type_mime_part(subtype));

    return STAT_OK;
}

static int preview_type(char **type, char **subtype, char **ext)
{
    CHECK_NULL(preview_type_ext(ext));
    return preview_type_mime(type, subtype);
}

static int new_preview(void)
{
    Token name = token;
    CHECK_OK(expect(TOK_STR));

    char *type = NULL;
    char *subtype = NULL;
    char *ext = NULL;
    CHECK_OK(preview_type(&type, &subtype, &ext));

    CHECK_OK(expect(TOK_BLK_OPEN));

    Token script = token;
    CHECK_OK(expect(TOK_STR));

    CHECK_OK(expect(TOK_BLK_CLS));

    add_preview(get_str(name), get_str(script), type, subtype, ext);
    return STAT_OK;
}

static int priority(Token tok)
{
    PARSEERROR(tok, "priority is not supported yet");
    return STAT_ERR;
}

static int command(void)
{
    Token cmd = token;
    CHECK_OK(expect(TOK_STR));

    char *cmd_str = get_str(cmd);
    if (strcmp(cmd_str, "preview") == 0)
        return new_preview();
    else if (strcmp(cmd_str, "priority") == 0)
        return priority(cmd);

    PARSEERROR(cmd, "unknown command: %s", cmd_str);
    return STAT_ERR;
}

static int commands(void)
{
    accept(TOK_NEW_LN);

    while (1) {
        CHECK_NULL(accept(TOK_EOF));
        CHECK_OK(command());
        CHECK_OK(accept(TOK_NEW_LN));
    }
}

static int parse(void)
{
    next_token();

    if (commands() == STAT_ERR)
        return ERR;

    return OK;
}

int config_load(VectorPreview *prevs, char *filename, char *any_type_)
{
    int ret = OK;

    FILE *f = fopen(filename, "r");
    ERRCHK_GOTO(!f, ret, exit, FUNCFAILED("fopen"), ERRNOS);

    lexer = lexer_init(f);
    previews = prevs;
    any_type = any_type_;

    ERRCHK_GOTO_OK(parse(), ret, file);

file:
    fclose(f);

exit:
    return ret;
}

void config_cleanup(void)
{
    if (lexer)
        lexer_free(lexer);
}
