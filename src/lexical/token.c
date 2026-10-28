#include "lexical/token.h"

define_vec(token_t);

char const *TOKEN_KIND_STRS[____tok_count] = {
    [TOK_PLUS] = "+",       [TOK_MINUS] = "-",      [TOK_STAR] = "*",
    [TOK_SLASH] = "/",      [TOK_PERCENT] = "%",

    [TOK_KW_IF] = "if",     [TOK_KW_ELSE] = "else", [TOK_KW_WHILE] = "while",
    [TOK_KW_AND] = "and",   [TOK_KW_OR] = "or",     [TOK_KW_NOT] = "not",
    [TOK_KW_TRUE] = "true", [TOK_KW_FALSE] = "false",

    [TOK_DOT] = ".",        [TOK_COMMA] = ",",      [TOK_SEMI_COLON] = ";",
    [TOK_LBRACE] = "{",     [TOK_RBRACE] = "}",     [TOK_LPAREN] = "(",
    [TOK_RPAREN] = ")",     [TOK_LBRACKET] = "[",   [TOK_RBRACKET] = "]",
};

char const *TOKEN_KIND_NAMES[____tok_count] = {
    [TOK_IDENT] = "IDENT",
    [TOK_STR] = "STR",
    [TOK_CHAR] = "CHAR",
    [TOK_INT] = "INT",
    [TOK_FLOAT] = "FLOAT",
    [TOK_BOOL] = "BOOL",

    [TOK_PLUS] = "PLUS",
    [TOK_MINUS] = "MINUS",
    [TOK_STAR] = "STAR",
    [TOK_SLASH] = "SLASH",
    [TOK_PERCENT] = "PERCENT",

    [TOK_KW_IF] = "KW_IF",
    [TOK_KW_ELSE] = "KW_ELSE",
    [TOK_KW_WHILE] = "KW_WHILE",
    [TOK_KW_AND] = "KW_AND",
    [TOK_KW_OR] = "KW_OR",
    [TOK_KW_NOT] = "KW_NOT",
    [TOK_KW_TRUE] = "KW_TRUE",
    [TOK_KW_FALSE] = "KW_FALSE",

    [TOK_DOT] = "DOT",
    [TOK_COMMA] = "COMMA",
    [TOK_SEMI_COLON] = "SEMI_COLON",
    [TOK_LBRACE] = "LBRACE",
    [TOK_RBRACE] = "RBRACE",
    [TOK_LPAREN] = "LPAREN",
    [TOK_RPAREN] = "RPAREN",
    [TOK_LBRACKET] = "LBRACKET",
    [TOK_RBRACKET] = "RBRACKET",

    [TOK_WHITESPACE] = "WHITESPACE",
    [TOK_UNKNOWN] = "UNKNOWN",
};

char const *token_kind_str_repr(token_kind_t kind)
{
    char const *repr = kind < ____tok_count ? TOKEN_KIND_STRS[kind] : NULL;
    return repr != NULL ? repr : "";
}

char const *token_kind_name(token_kind_t kind)
{
    char const *name = kind < ____tok_count ? TOKEN_KIND_NAMES[kind] : NULL;
    return name != NULL ? name : "";
}
