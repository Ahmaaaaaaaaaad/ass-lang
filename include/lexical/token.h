#ifndef __LEXICAL_TOKEN_H__
#define __LEXICAL_TOKEN_H__

#include <stdint.h>

#include "core/string/slice.h"

typedef enum : uint8_t
{
    TOK_IDENT,
    TOK_STR,
    TOK_CHAR,
    TOK_INT,
    TOK_FLOAT,
    TOK_BOOL,

    // Operators

    TOK_PLUS,    // +
    TOK_MINUS,   // -
    TOK_STAR,    // * - Asterisk
    TOK_SLASH,   // /
    TOK_PERCENT, // %

    // Keywords
    ____tok_kw_start,

    TOK_KW_IF,
    TOK_KW_ELSE,
    TOK_KW_WHILE,

    TOK_KW_AND,
    TOK_KW_OR,
    TOK_KW_NOT,

    TOK_KW_TRUE,
    TOK_KW_FALSE,

    ____tok_kw_end,

    // Delimeters
    TOK_DOT,        // .
    TOK_COMMA,      // ,
    TOK_SEMI_COLON, // ;

    TOK_LBRACE,   // {
    TOK_RBRACE,   // }
    TOK_LPAREN,   // (
    TOK_RPAREN,   // )
    TOK_LBRACKET, // [
    TOK_RBRACKET, // ]

} token_kind_t;

extern char const *TOKEN_KIND_STRS[];

char const *token_kind_str_repr(token_kind_t kind);

typedef struct
{
    string_slice_t lexeme;
    token_kind_t kind;
} token_t;

#endif
