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
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/lexer.h"

using namespace plnnrc;

static const char* s_token_type_names[] =
{
    #define PLNNRC_TOKEN(TOKEN_TAG) #TOKEN_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN
    "Count",
};

static const char* s_token_group_names[] =
{
    "Unknown",
    #define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG) #GROUP_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN_GROUP
    "Count",
};

static Token_Type s_group_first[] =
{
    Token_Unknown,
    #define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG) Token_##FIRST_TOKEN_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN_GROUP
};

static Token_Type s_group_last[] =
{
    Token_Unknown,
    #define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG) Token_##LAST_TOKEN_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN_GROUP
};

const char* plnnrc::get_type_name(Token_Type token_type)
{
    return s_token_type_names[token_type];
}

const char* plnnrc::get_group_name(Token_Group token_group)
{
    return s_token_group_names[token_group];
}

Token_Type plnnrc::get_group_first(Token_Group token_group)
{
    return s_group_first[token_group];
}

Token_Type plnnrc::get_group_last(Token_Group token_group)
{
    return s_group_last[token_group];
}

void plnnrc::init(Lexer* self, const char* buffer, Memory_Stack* scratch)
{
    memset(self, 0, sizeof(Lexer));

    self->buffer_start = buffer;
    self->buffer_ptr = buffer;
    self->loc.column = 1;
    self->loc.line = 1;
    const uint32_t num_keywords = (uint32_t)(Token_Group_Keyword_Last - Token_Group_Keyword_First + 1);
    init(self->keywords, scratch, num_keywords);
    self->scratch = scratch;

#define PLNNRC_KEYWORD_TOKEN(TOKEN_TAG, TOKEN_STR)      \
    set(self->keywords, TOKEN_STR, Token_##TOKEN_TAG);  \

    #include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_KEYWORD_TOKEN
}

bool plnnrc::equal(Token_Value a, Token_Value b)
{
    return (a.length == b.length) && (strncmp(a.str, b.str, a.length) == 0);
}

static char get_char(const Lexer* self)
{
    return *self->buffer_ptr;
}

static bool is_whitespace(char c)
{
    return c == ' ' || c == '\f' || c == '\t' || c == '\v' || c == '\n' || c == '\r';
}

static bool is_identifier_body(char c)
{
    return \
        c == 'A' || c == 'B' || c == 'C' || c == 'D' || c == 'E' || c == 'F' || c == 'G' || c == 'H' || c == 'I' || c == 'J' || c == 'K' || c == 'L' ||
        c == 'M' || c == 'N' || c == 'O' || c == 'P' || c == 'Q' || c == 'R' || c == 'S' || c == 'T' || c == 'U' || c == 'V' || c == 'W' || c == 'X' || c == 'Y' || c == 'Z' ||
        c == 'a' || c == 'b' || c == 'c' || c == 'd' || c == 'e' || c == 'f' || c == 'g' || c == 'h' || c == 'i' || c == 'j' || c == 'k' || c == 'l' ||
        c == 'm' || c == 'n' || c == 'o' || c == 'p' || c == 'q' || c == 'r' || c == 's' || c == 't' || c == 'u' || c == 'v' || c == 'w' || c == 'x' || c == 'y' || c == 'z' ||
        c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5' || c == '6' || c == '7' || c == '8' || c == '9' || c == '_' || c == '!' || c == '?';
}

static void consume_char(Lexer* self)
{
    self->loc.column++;
    self->buffer_ptr++;
}

static void consume_newline(Lexer* self)
{
    char c1 = get_char(self);
    consume_char(self);
    char c2 = get_char(self);

    if ((c2 != c1) && (c1 == '\n' || c1 == '\r') && (c2 == '\n' || c2 == '\r'))
    {
        consume_char(self);
    }

    self->loc.column = 1;
    self->loc.line++;
}

static void consume_until_whitespace(Lexer* self)
{
    while (char c = get_char(self))
    {
        // whitespace
        if (is_whitespace(c))
        {
            return;
        }

        consume_char(self);
    }
}

static void consume_until_newline(Lexer* self)
{
    while (char c = get_char(self))
    {
        if (c == '\n' || c == '\r')
        {
            return;
        }

        consume_char(self);
    }
}

static Token make_token(const Lexer* self, Token_Type type)
{
    Token tok;
    memset(&tok, 0, sizeof(tok));
    tok.type = type;
    tok.loc = self->loc;
    tok.value.length = 0;
    tok.value.str = 0;
    return tok;
}

static Token begin_token(const Lexer* self)
{
    Token tok;
    memset(&tok, 0, sizeof(tok));
    tok.loc = self->loc;
    tok.value.length = 0;
    tok.value.str = self->buffer_ptr;
    return tok;
}

static void end_token(const Lexer* self, Token* tok, Token_Type type)
{
    tok->type = type;
    plnnrc_assert(self->buffer_ptr > tok->value.str);
    tok->value.length = static_cast<uint32_t>(self->buffer_ptr - tok->value.str);
}

static Token lex_identifier(Lexer* self)
{
    Token tok = begin_token(self);

    while (char c = get_char(self))
    {
        if (!is_identifier_body(c))
        {
            break;
        }

        consume_char(self);
    }

    end_token(self, &tok, Token_Id);

    // test if the token is actually keyword and modify type accordinly
    const Token_Type* keyword_type = get(self->keywords, tok.value.str, tok.value.length);
    if (keyword_type != 0)
    {
        tok.type = *keyword_type;
    }

    return tok;
}

static Token lex_symbol(Lexer* self)
{
    Token tok = begin_token(self);
    plnnrc_assert(get_char(self) == ':');
    consume_char(self); // eat ':'

    while (char c = get_char(self))
    {
        if (!is_identifier_body(c))
        {
            break;
        }

        consume_char(self);
    }

    end_token(self, &tok, Token_Literal_Symbol);

    if (tok.value.length == 1)
    {
        tok.type = Token_Unknown;
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

static Token lex_numeric_literal(Lexer* self)
{
    Token tok = begin_token(self);

    int state = 0;
    while (char c = get_char(self))
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
        consume_char(self);
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

    end_token(self, &tok, type);
    return tok;
}

static Token lex_unknown(Lexer* self)
{
    Token tok = begin_token(self);
    consume_until_whitespace(self);
    end_token(self, &tok, Token_Unknown);
    return tok;
}

Token plnnrc::lex(Lexer* self)
{
    Token tok;
    memset(&tok, 0, sizeof(tok));

    for (char c = get_char(self); c != 0; c = get_char(self))
    {
        switch (c)
        {
        // division or single line C-style comments 
        case '/':
            tok = make_token(self, Token_Div);
            consume_char(self);

            if (get_char(self) == '/')
            {
                consume_char(self);
                consume_until_newline(self);
                break;
            }

            return tok;
        // whitespace
        case ' ': case '\f': case '\t': case '\v':
            consume_char(self);
            break;
        // newline
        case '\n': case '\r':
            consume_newline(self);
            break;

        // single-character punctuators
        case '{':
            tok = make_token(self, Token_L_Curly);
            consume_char(self);
            return tok;
        case '}':
            tok = make_token(self, Token_R_Curly);
            consume_char(self);
            return tok;
        case '(':
            tok = make_token(self, Token_L_Paren);
            consume_char(self);
            return tok;
        case ')':
            tok = make_token(self, Token_R_Paren);
            consume_char(self);
            return tok;
        case '[':
            tok = make_token(self, Token_L_Square);
            consume_char(self);
            return tok;
        case ']':
            tok = make_token(self, Token_R_Square);
            consume_char(self);
            return tok;
        case ',':
            tok = make_token(self, Token_Comma);
            consume_char(self);
            return tok;

        case '&':
            tok = make_token(self, Token_And);
            consume_char(self);
            return tok;
        case '|':
            tok = make_token(self, Token_Or);
            consume_char(self);
            return tok;
        case '.':
            tok = make_token(self, Token_Dot);
            consume_char(self);
            return tok;
        case '+':
            tok = make_token(self, Token_Plus);
            consume_char(self);
            return tok;
        case '*':
            tok = make_token(self, Token_Mul);
            consume_char(self);
            return tok;

        case '-':
            // arrow or minus
            tok = make_token(self, Token_Unknown);
            consume_char(self);
            if (get_char(self) == '>')
            {
                consume_char(self);
                tok.type = Token_Arrow;
                return tok;
            }

            tok.type = Token_Minus;
            return tok;
        case '=':
            // '=' or '=='
            tok = make_token(self, Token_Unknown);
            consume_char(self);
            if (get_char(self) == '=')
            {
                consume_char(self);
                tok.type = Token_Equal;
                return tok;
            }

            tok.type = Token_Assign;
            return tok;
        case '<':
            tok = make_token(self, Token_Less);
            consume_char(self);
            if (get_char(self) == '=')
            {
                consume_char(self);
                tok.type = Token_LessEqual;
                return tok;
            }
            return tok;
        case '>':
            tok = make_token(self, Token_Greater);
            consume_char(self);
            if (get_char(self) == '=')
            {
                consume_char(self);
                tok.type = Token_GreaterEqual;
                return tok;
            }
            return tok;
        case '~':
            tok = make_token(self, Token_Not);
            consume_char(self);
            if (get_char(self) == '=')
            {
                consume_char(self);
                tok.type = Token_NotEqual;
                return tok;
            }
            return tok;

        // symbol
        case ':':
            return lex_symbol(self);

        // identifiers & keywords
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case '?': case '!': case '_':
            return lex_identifier(self);

        // numeric literal
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            return lex_numeric_literal(self);

        default:
            return lex_unknown(self);
        }
    }

    return make_token(self, Token_Eos);
}

void plnnrc::debug_output_tokens(const char* buffer, Writer* output)
{
    Memory_Stack* scratch = Memory_Stack::create(32*1024);
    {
        Lexer lexer;
        init(&lexer, buffer, scratch);

        Formatter fmtr;
        init(fmtr, "  ", "\n", output);

        Token tok = lex(&lexer);
        uint32_t prev_line = tok.loc.line;
        newline(fmtr);

        for (; tok.type != Token_Eos; tok = lex(&lexer))
        {
            if (tok.loc.line > prev_line)
            {
                newline(fmtr);
            }

            if (has_value(tok))
            {
                write(fmtr, "%s[%n] ", get_type_name(tok.type), tok.value);
            }
            else
            {
                write(fmtr, "%s ", get_type_name(tok.type));
            }

            prev_line = tok.loc.line;
        }

        if (tok.type == Token_Eos)
        {
            if (tok.loc.line > prev_line)
            {
                newline(fmtr);
            }

            write(fmtr, "%s", get_type_name(tok.type));
            newline(fmtr);
        }
    }

    Memory_Stack::destroy(scratch);
}
