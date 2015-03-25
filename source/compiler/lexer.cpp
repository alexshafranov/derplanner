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
#include "derplanner/compiler/lexer.h"

using namespace plnnrc;

void plnnrc::init(Lexer_State& result, const char* buffer)
{
    result.buffer_start = buffer;
    result.buffer_ptr = buffer;
    result.column = 0;
    result.line = 0;
    init(result.errors, 32);
}

void plnnrc::destroy(Lexer_State& state)
{
    destroy(state.errors);
}

inline char get_char(const Lexer_State& state)
{
    return *state.buffer_ptr;
}

inline void consume_char(Lexer_State& state)
{
    state.column++;
    state.buffer_ptr++;
}

inline void consume_newline(Lexer_State& state)
{
    char c1 = get_char(state);
    consume_char(state);
    char c2 = get_char(state);

    if ((c2 != c1) && (c1 == '\n' || c1 == '\r'))
    {
        consume_char(state);
    }

    state.column = 0;
    state.line++;
}

inline void consume_until_whitespace(Lexer_State& state)
{
    char c = get_char(state);
    while (c != 0)
    {
        // whitespace
        if (c == ' ' || c == '\f' || c == '\t' || c == '\v' || c == '\n' || c == '\r')
        {
            return;
        }

        consume_char(state);
    }
}

inline Token make_token(Lexer_State& state, Token_Type type)
{
    Token tok;
    tok.type = type;
    tok.column = state.column;
    tok.line = state.line;
    tok.length = 0;
    tok.str = 0;
    return tok;
}

inline Token begin_token(Lexer_State& state)
{
    Token tok;
    tok.column = state.column;
    tok.line = state.line;
    tok.str = state.buffer_ptr - 1;
    tok.length = 0;
    return tok;
}

inline void end_token(Lexer_State& state, Token& tok, Token_Type type)
{
    tok.type = type;
    tok.length = static_cast<uint32_t>(state.buffer_ptr - tok.str);
}

inline Token lex_id(Lexer_State& state)
{
    Token tok = begin_token(state);

    for (;;)
    {
        switch (get_char(state))
        {
            case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
            case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
            case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            case '!': case '?': case '_':
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
                consume_char(state);
                break;
            default:
                end_token(state, tok, Token_Identifier);
                return tok;
        }
    }
}

inline Token lex_unknown(Lexer_State& state)
{
    Token tok = begin_token(state);
    consume_until_whitespace(state);
    end_token(state, tok, Token_Unknown);
    return tok;
}

Token plnnrc::lex(Lexer_State& state)
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

        // arrow
        case '-':
            consume_char(state);
            if (get_char(state) == '>')
            {
                consume_char(state);
                return make_token(state, Token_Arrow);
            }

            return lex_unknown(state);

        // identifiers & keywords
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case '!': case '?': case '_':
            return lex_id(state);

        default:
            return lex_unknown(state);
        }
    }

    return make_token(state, Token_Eof);
}
