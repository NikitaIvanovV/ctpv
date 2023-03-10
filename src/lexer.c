#include <ctype.h>
#include <assert.h>

#include "error.h"
#include "lexer.h"
#include "ulist.h"

#define READ_PUNCT(c, t, s) read_punct((c), (t), (s), LEN(s) - 1)

#define EOF_CHAR (-1)

typedef int (*Predicate)(int);

typedef struct {
    unsigned int pos, len, eof;
    FILE *f;
    char buf[1024];
} InputBuffer;

typedef struct {
    unsigned int back, front;
    Token toks[16];
} TokenQueue;

struct Lexer {
    enum LexerOpts opts;
    unsigned line, col;
    struct {
        unsigned int line, col;
    } tok_pos;
    InputBuffer input_buf;
    TokenQueue tok_queue;
    UList *text_buf;
};

static char block_open[] = "{{",
            block_close[] = "}}",
            slash[] = "/",
            star[] = "*",
            dot[] = ".";

static void add_token_queue(Lexer *ctx, Token tok)
{
    ctx->tok_queue.toks[ctx->tok_queue.back] = tok;
    ctx->tok_queue.back = (ctx->tok_queue.back + 1) % LEN(ctx->tok_queue.toks);
}

static Token remove_token_queue(Lexer *ctx)
{
    Token tok = ctx->tok_queue.toks[ctx->tok_queue.front];
    ctx->tok_queue.front = (ctx->tok_queue.front + 1) % LEN(ctx->tok_queue.toks);
    return tok;
}

static inline int is_empty_token_queue(Lexer *ctx)
{
    return ctx->tok_queue.back == ctx->tok_queue.front;
}

static void init_input_buf(InputBuffer *b, FILE *f)
{
    b->pos = 0;
    b->len = 0;
    b->eof = 0;
    b->f = f;
}

static int peekn_char(Lexer *ctx, unsigned int i)
{
    InputBuffer *b = &ctx->input_buf;

    if (b->pos + i < b->len)
        goto exit;

    if (b->eof || (i > 0 && i >= b->len))
        return EOF_CHAR;

    if (i > 0) {
        assert(i < LEN(b->buf));
        memmove(b->buf, b->buf + (b->len - i), i * sizeof(*b->buf));
    }

    b->len = i + fread(b->buf + i, sizeof(*b->buf), LEN(b->buf) - i, b->f);
    b->pos = 0;

    if (b->len != LEN(b->buf)) {
        if (feof(b->f))
            b->eof = 1;
        else if (ferror(b->f))
            PRINTINTERR("fread() failed");

        if (b->len == 0)
            return EOF_CHAR;
    }

exit:
    return b->buf[b->pos + i];
}

static inline char peek_char(Lexer *ctx)
{
    return peekn_char(ctx, 0);
}

static char nextn_char(Lexer *ctx, unsigned int i)
{
    char c = peekn_char(ctx, i);

    ctx->col++;

    if (c == '\n') {
        ctx->col = 1;
        ctx->line++;
    }

    ctx->input_buf.pos++;

    return c;
}

static inline char next_char(Lexer *ctx)
{
    return nextn_char(ctx, 0);
}

static void skipn_char(Lexer *ctx, int n)
{
    for (int i = 0; i < n; i++)
        next_char(ctx);
}

static inline void add_text_buf(Lexer *ctx, char c)
{
    ulist_append(ctx->text_buf, &c);
}

static inline void record_text(Lexer *ctx)
{
    ulist_lock(ctx->text_buf);
}

static inline char *get_text(Lexer *ctx)
{
    return ulist_unlock(ctx->text_buf);
}

Lexer *lexer_init(FILE *f)
{
    Lexer *ctx;

    if (!(ctx = malloc(sizeof(*ctx)))) {
        FUNCFAILED("malloc", strerror(errno));
        abort();
    }

    init_input_buf(&ctx->input_buf, f);
    lexer_set_opts(ctx, LEX_OPT_NONE);
    ctx->text_buf = ulist_new(sizeof(char), 1024);
    ctx->line = ctx->col = 1;
    ctx->tok_queue.back = ctx->tok_queue.front = 0;

    return ctx;
}

void lexer_set_opts(Lexer *ctx, enum LexerOpts flags)
{
    ctx->opts = flags;
}

void lexer_free(Lexer *ctx)
{
    ulist_free(ctx->text_buf);
    free(ctx);
}

static int cmp_nextn(Lexer *ctx, int n, char *s)
{
    char c;
    int i = 0;

    while (1) {
        c = peekn_char(ctx, i);
        if (i >= n || *s == '\0' || c != *s)
            break;

        s++;
        i++;
    }

    return i == n ? 0 : ((unsigned char)c - *(unsigned char *)s);
}

static void ignore_comments(Lexer *ctx)
{
    char c;

    while (peek_char(ctx) == '#') {
        do {
            c = next_char(ctx);
        } while (c != '\n');
    }
}

static void read_while(Lexer *ctx, Predicate p, int add)
{
    char c;

    while ((c = peek_char(ctx)) >= 0 && p(c)) {
        if (add)
            add_text_buf(ctx, c);

        next_char(ctx);
    }

    if (add)
        add_text_buf(ctx, '\0');
}

static inline Token get_tok(Lexer *ctx, enum TokenType type)
{
    return (Token){ .type = type,
                    .line = ctx->tok_pos.line,
                    .col = ctx->tok_pos.col };
}

static inline Token read_new_line(Lexer *ctx)
{
    Token tok = get_tok(ctx, TOK_NULL);

    while (peek_char(ctx) == '\n') {
        next_char(ctx);
        tok.type = TOK_NEW_LN;
    }

    return tok;
}

static inline int issymbol(int c)
{
    return isalnum(c) || c == '_' || c == '-';
}

static inline int isnotquote(int c)
{
    return (c != '"');
}

static inline Token read_symbol(Lexer *ctx)
{
    char c = peek_char(ctx);

    if (!isalpha(c))
        return get_tok(ctx, TOK_NULL);

    record_text(ctx);
    read_while(ctx, issymbol, 1);

    Token tok = get_tok(ctx, TOK_STR);
    tok.val.s = get_text(ctx);

    return tok;
}

static inline Token read_string(Lexer *ctx)
{
    char c = next_char(ctx);

    if (isnotquote(c))
        return get_tok(ctx, TOK_NULL);

    record_text(ctx);
    read_while(ctx, isnotquote, 1);

    Token tok = get_tok(ctx, TOK_STR);
    tok.val.s = get_text(ctx);

    // Skip ending quote
    next_char(ctx);

    return tok;
}

static inline Token read_int(Lexer *ctx)
{
    int positive = 1;

    if (peek_char(ctx) == '-') {
        positive = 0;
        next_char(ctx);
    }

    if (!isdigit(peek_char(ctx)))
        return get_tok(ctx, TOK_NULL);

    record_text(ctx);
    read_while(ctx, isdigit, 1);

    Token tok;
    char *text = get_text(ctx);

    /* If NUMISTEXT option is set, do not convert string to integer */
    if (ctx->opts & LEX_OPT_NUMISTEXT) {
        tok = get_tok(ctx, TOK_STR);
        tok.val.s = text;
        return tok;
    }

    int i = atoi(text);

    if (!positive)
        i *= -1;

    tok = get_tok(ctx, TOK_INT);
    tok.val.i = i;

    return tok;
}

static Token read_punct(Lexer *ctx, int type, char *s, int n)
{
    Token tok;

    if (peek_char(ctx) == EOF_CHAR)
        return get_tok(ctx, TOK_EOF);

    int ret = cmp_nextn(ctx, n, s);

    if (ret == 0)
        tok = get_tok(ctx, type);
    else
        return get_tok(ctx, TOK_NULL);

    skipn_char(ctx, n);

    return tok;
}

static inline Token read_block_open(Lexer *ctx)
{
    return READ_PUNCT(ctx, TOK_BLK_OPEN, block_open);
}

static inline Token read_block_close(Lexer *ctx)
{
    return READ_PUNCT(ctx, TOK_BLK_CLS, block_close);
}

static Token read_block(Lexer *ctx)
{
    Token open_tok, body_tok, close_tok;

    if ((open_tok = read_block_open(ctx)).type == TOK_NULL)
        return get_tok(ctx, TOK_NULL);

    record_text(ctx);

    while (1) {
        close_tok = read_block_close(ctx);

        if (close_tok.type == TOK_EOF) {
            PARSEERROR(*ctx, "unclosed block");
            return get_tok(ctx, TOK_ERR);
        } else if (close_tok.type != TOK_NULL) {
            break;
        }

        add_text_buf(ctx, next_char(ctx));
    }

    add_text_buf(ctx, '\0');

    body_tok = get_tok(ctx, TOK_STR);
    body_tok.val.s = get_text(ctx);

    add_token_queue(ctx, body_tok);

    if (close_tok.type != TOK_NULL)
        add_token_queue(ctx, close_tok);

    return open_tok;
}

#define ATTEMPT_READ(c, func)   \
    do {                        \
        Token t = (func)(c);    \
        if (t.type != TOK_NULL) \
            return t;           \
    } while (0)

#define ATTEMPT_READ_CHAR(ctx, tok, ch, type_) \
    do {                                       \
        char c = peek_char(ctx);               \
        if (c == (ch)) {                       \
            (tok).type = (type_);              \
            next_char(ctx);                    \
            return (tok);                      \
        }                                      \
    } while (0)

Token lexer_get_token(Lexer *ctx)
{
    if (!is_empty_token_queue(ctx))
        return remove_token_queue(ctx);

    read_while(ctx, isblank, 0);
    ignore_comments(ctx);

    ctx->tok_pos.line = ctx->line;
    ctx->tok_pos.col = ctx->col;

    Token tok = get_tok(ctx, TOK_NULL);

    ATTEMPT_READ_CHAR(ctx, tok, EOF_CHAR, TOK_EOF);
    ATTEMPT_READ_CHAR(ctx, tok, '/', TOK_SLASH);
    ATTEMPT_READ_CHAR(ctx, tok, '*', TOK_STAR);
    ATTEMPT_READ_CHAR(ctx, tok, '.', TOK_DOT);

    ATTEMPT_READ(ctx, read_new_line);
    ATTEMPT_READ(ctx, read_symbol);
    ATTEMPT_READ(ctx, read_int);
    ATTEMPT_READ(ctx, read_block);
    ATTEMPT_READ(ctx, read_string);

    PARSEERROR((*ctx), "cannot handle character: %c", peek_char(ctx));
    return get_tok(ctx, TOK_ERR);
}

char *lexer_token_type_str(enum TokenType type)
{
    switch (type) {
    case TOK_NULL:
        return "<null>";
    case TOK_EOF:
        return "<end of file>";
    case TOK_ERR:
        return "<TOKEN ERROR>";
    case TOK_NEW_LN:
        return "<newline>";
    case TOK_BLK_OPEN:
        return block_open;
    case TOK_BLK_CLS:
        return block_close;
    case TOK_SLASH:
        return slash;
    case TOK_STAR:
        return star;
    case TOK_DOT:
        return dot;
    case TOK_INT:
        return "<integer>";
    case TOK_STR:
        return "<string>";
    }

    PRINTINTERR("unknown type: %d", type);
    abort();
}
