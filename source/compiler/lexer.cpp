//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <string.h>
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/lexer.h"

using namespace plnnrc;

static const char* token_type_names[] =
{
    "Unknown",
    #define PLNNRC_TOKEN(TOKEN_TAG) #TOKEN_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN
    "None",
};

const char* plnnrc::get_token_name(Token_Type token_type)
{
    return token_type_names[token_type];
}

void plnnrc::init(Lexer& result, const char* buffer)
{
    result.buffer_start = buffer;
    result.buffer_ptr = buffer;
    result.column = 1;
    result.line = 1;
    init(result.keywords, 16);

#define PLNNRC_KEYWORD_TOKEN(TOKEN_TAG, TOKEN_STR)      \
    set(result.keywords, TOKEN_STR, Token_##TOKEN_TAG); \

    #include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_KEYWORD_TOKEN
}

void plnnrc::destroy(Lexer& state)
{
    destroy(state.keywords);
}

static inline char get_char(const Lexer& state)
{
    return *state.buffer_ptr;
}

static inline bool is_whitespace(char c)
{
    return c == ' ' || c == '\f' || c == '\t' || c == '\v' || c == '\n' || c == '\r';
}

static inline bool is_identifier_body(char c)
{
    return \
        c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'G' || c == 'H' || c == 'I' || c == 'J' || c == 'K' || c == 'L' ||
        c == 'M' || c == 'N' || c == 'O' || c == 'P' || c == 'Q' || c == 'R' || c == 'S' || c == 'T' || c == 'U' || c == 'V' || c == 'W' || c == 'X' || c == 'Y' || c == 'Z' ||
        c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'g' || c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' ||
        c == 'm' || c == 'n' || c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u' || c == 'v' || c == 'w' || c == 'x' || c == 'y' || c == 'z' ||
        c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '_' || c == '!' || c == '?';
}

static inline void consume_char(Lexer& state)
{
    state.column++;
    state.buffer_ptr++;
}

static inline void consume_newline(Lexer& state)
{
    char c1 = get_char(state);
    consume_char(state);
    char c2 = get_char(state);

    if ((c2 != c1) && (c1 == '\n' || c1 == '\r') && (c2 == '\n' || c2 == '\r'))
    {
        consume_char(state);
    }

    state.column = 1;
    state.line++;
}

static inline void consume_until_whitespace(Lexer& state)
{
    while (char c = get_char(state))
    {
        // whitespace
        if (is_whitespace(c))
        {
            return;
        }

        consume_char(state);
    }
}

static inline Token make_token(Lexer& state, Token_Type type)
{
    Token tok;
    tok.type = type;
    tok.column = state.column;
    tok.line = state.line;
    tok.value.length = 0;
    tok.value.str = 0;
    return tok;
}

static inline Token begin_token(Lexer& state)
{
    Token tok;
    tok.column = state.column;
    tok.line = state.line;
    tok.value.length = 0;
    tok.value.str = state.buffer_ptr;
    return tok;
}

static inline void end_token(Lexer& state, Token& tok, Token_Type type)
{
    tok.type = type;
    plnnrc_assert(state.buffer_ptr > tok.value.str);
    tok.value.length = static_cast<uint32_t>(state.buffer_ptr - tok.value.str);
}

static inline Token lex_identifier(Lexer& state)
{
    Token tok = begin_token(state);

    while (char c = get_char(state))
    {
        if (!is_identifier_body(c))
        {
            break;
        }

        consume_char(state);
    }

    end_token(state, tok, Token_Identifier);

    // test if the token is actually keyword and modify type accordinly
    const Token_Type* keyword_type_ptr = get(state.keywords, tok.value.str, tok.value.length);
    if (keyword_type_ptr != 0)
    {
        tok.type = *keyword_type_ptr;
    }

    return tok;
}

namespace numeric_literal
{
    int get_char_class(char c)
    {
        switch (c)
        {
        case '+': case '-':
            return 0;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return 1;
        case '.':
            return 2;
        case 'e': case 'E':
            return 3;
        }

        return -1;
    }

    int transitions[] = {
         1,  3,  2, -1,
        -1,  3,  2, -1,
        -1,  4, -1, -1,
        -1,  3,  4,  5,
        -1,  4, -1,  5,
         6,  7, -1, -1,
        -1,  7, -1, -1,
        -1,  7, -1, -1,
    };
}

static inline Token lex_numeric_literal(Lexer& lexer_state)
{
    Token tok = begin_token(lexer_state);

    int state = 0;
    while (char c = get_char(lexer_state))
    {
        int char_class = numeric_literal::get_char_class(c);

        if (char_class < 0)
        {
            break;
        }

        int next_state = numeric_literal::transitions[state * 4 + char_class];

        if (next_state < 0)
        {
            break;
        }

        state = next_state;
        consume_char(lexer_state);
    }

    Token_Type type = Token_Unknown;

    if (state == 3)
    {
        type = Token_Literal_Integer;
    }
    else if (state == 4 || state == 7)
    {
        type = Token_Literal_Float;
    }

    end_token(lexer_state, tok, type);
    return tok;
}

static inline Token lex_unknown(Lexer& state)
{
    Token tok = begin_token(state);
    consume_until_whitespace(state);
    end_token(state, tok, Token_Unknown);
    return tok;
}

Token plnnrc::lex(Lexer& state)
{
    for (char c = get_char(state); c != 0; c = get_char(state))
    {
        switch (c)
        {
        // whitespace
        case ' ': case '\f': case '\t': case '\v':
            consume_char(state);
            break;
        // newline
        case '\n': case '\r':
            consume_newline(state);
            break;

        // single-character punctuators
        case '{':
            consume_char(state);
            return make_token(state, Token_L_Curly);
        case '}':
            consume_char(state);
            return make_token(state, Token_R_Curly);
        case '(':
            consume_char(state);
            return make_token(state, Token_L_Paren);
        case ')':
            consume_char(state);
            return make_token(state, Token_R_Paren);
        case '[':
            consume_char(state);
            return make_token(state, Token_L_Square);
        case ']':
            consume_char(state);
            return make_token(state, Token_R_Square);
        case ',':
            consume_char(state);
            return make_token(state, Token_Comma);
        case ':':
            consume_char(state);
            return make_token(state, Token_Colon);

        // single-character operators
        case '&':
            consume_char(state);
            return make_token(state, Token_And);
        case '|':
            consume_char(state);
            return make_token(state, Token_Or);
        case '~':
            consume_char(state);
            return make_token(state, Token_Not);

        // arrow or numeric
        case '-':
            consume_char(state);
            if (get_char(state) == '>')
            {
                consume_char(state);
                return make_token(state, Token_Arrow);
            }

            return make_token(state, Token_Minus);

        case '+':
            consume_char(state);
            return make_token(state, Token_Plus);

        // identifiers & keywords
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case '!': case '?': case '_':
            return lex_identifier(state);

        // numeric literal
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return lex_numeric_literal(state);

        default:
            return lex_unknown(state);
        }
    }

    return make_token(state, Token_Eof);
}
