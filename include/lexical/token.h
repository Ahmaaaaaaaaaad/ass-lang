#ifndef __LEXICAL_TOKEN_H__
#define __LEXICAL_TOKEN_H__

#include <stdint.h>

#include "core/collection/vec.h"
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

    // Special
    TOK_WHITESPACE, // Skipped by the lexer, never emitted
    TOK_UNKNOWN,    // Lexical error

    ____tok_count,
} token_kind_t;

#define is_keyword(kind) ((kind) > ____tok_kw_start && (kind) < ____tok_kw_end)

extern char const *TOKEN_KIND_STRS[];
extern char const *TOKEN_KIND_NAMES[];

/// Source spelling of keywords, operators and delimiters (e.g. `"if"`, `"+"`),
/// or `""` for token kinds without a fixed spelling.
char const *token_kind_str_repr(token_kind_t kind);
/// Display name of a token kind (e.g. `"KW_IF"`, `"PLUS"`).
char const *token_kind_name(token_kind_t kind);

typedef struct
{
    string_slice_t lexeme;
    token_kind_t kind;
    uint32_t line, col;
} token_t;

declare_vec(token_t);

#endif
