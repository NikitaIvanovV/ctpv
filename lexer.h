#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Lexer Lexer;

typedef struct {
    enum TokenType {
        TOK_NULL,
        TOK_EOF,
        TOK_ERR,
        TOK_END,
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
        size_t sp;
    } val;
} Token;

Lexer *lexer_init(FILE *f);
void lexer_free(Lexer *ctx);
Token lexer_get_token(Lexer *ctx);
char *lexer_get_string(Lexer *ctx, Token tok);
char *lexer_token_type_str(enum TokenType type);

#endif
