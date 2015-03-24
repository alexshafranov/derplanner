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

inline Token make_token()
{
    Token tok;
    memset(&tok, 0, sizeof(Token));
    return tok;
}

inline Token make_token(Lexer_State& state, Token_Type type, const char* str, uint32_t size)
{
    Token tok;
    tok.type = type;
    tok.column = state.column;
    tok.line = state.line;
    tok.size = size;
    tok.str = str;
    return tok;
}

Token plnnrc::lex(Lexer_State& state)
{
    for (;;)
    {
        char c = get_char(state);
        if (c == 0)
        {
            break;
        }

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
        // punctuators
        case '-':
            consume_char(state);
            if (get_char(state) == '>')
            {
                consume_char(state);
            }
            break;
        }
    }

    return make_token();
}
