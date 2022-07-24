#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>

#define PARSEERROR(c, format, ...)                  \
    print_errorf("config:%u:%u: " format, (c).line, \
                 (c).col __VA_OPT__(, ) __VA_ARGS__)

typedef struct Lexer Lexer;

enum LexerOpts {
    LEX_OPT_NONE = 0,
    LEX_OPT_NUMISTEXT = 1 << 0,
};

typedef struct {
    unsigned int line, col;
    enum TokenType {
        TOK_NULL,
        TOK_EOF,
        TOK_ERR,
        TOK_NEW_LN,
        TOK_BLK_OPEN,
        TOK_BLK_CLS,
        TOK_SLASH,
        TOK_STAR,
        TOK_DOT,
        TOK_INT,
        TOK_STR,
    } type;
    union {
        int i;
        char *s;
    } val;
} Token;

Lexer *lexer_init(FILE *f);
void lexer_set_opts(Lexer *ctx, enum LexerOpts flags);
void lexer_free(Lexer *ctx);
Token lexer_get_token(Lexer *ctx);
char *lexer_token_type_str(enum TokenType type);

#endif
