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

#ifndef DERPLANNER_COMPILER_LEXER_H_
#define DERPLANNER_COMPILER_LEXER_H_

#include "derplanner/compiler/types.h"

namespace plnnrc {

/// Lexer

// create lexer state.
void init(Lexer& state, const char* buffer, Memory* mem);
// destroy lexer state.
void destroy(Lexer& state);

// lex next token from the input buffer.
Token lex(Lexer& state);

// writes the nicely formatted token stream to `output`.
void debug_output_tokens(const char* buffer, Writer* output);

/// Token

// checks if the given token has a string value attached.
bool has_value(const Token& tok);

// gets token type name as a string to aid debugging.
const char* get_type_name(Token_Type token_type);

// compute FNV-1a hash for the token value.
uint32_t hash(Token_Value token_value);
// check equality by comparing actual strings.
bool     equal(Token_Value a, Token_Value b);

// is_<Token_Type>
#define PLNNRC_TOKEN(TAG)                    \
    bool is_##TAG(Token_Type token_type);    \
    bool is_##TAG(const Token& token);       \

#include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_TOKEN

// is_<Token_Group>
#define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG)   \
    bool is_##GROUP_TAG(Token_Type token_type);                          \
    bool is_##GROUP_TAG(const Token& token);                             \

#include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_TOKEN_GROUP

}

/// Inline Code.

inline plnnrc::Token::Token() : type(plnnrc::Token_Unknown), line(0), column(0) {}

inline bool plnnrc::has_value(const plnnrc::Token& tok) { return tok.value.length > 0; }

// is_<Token_Type>
#define PLNNRC_TOKEN(TAG)                                                                                       \
    inline bool plnnrc::is_##TAG(plnnrc::Token_Type token_type) { return token_type == plnnrc::Token_##TAG; }   \
    inline bool plnnrc::is_##TAG(const plnnrc::Token& token) { return is_##TAG(token.type); }                   \

#include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_TOKEN

// is_<Token_Group>
#define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG)                                                                                              \
    inline bool plnnrc::is_##GROUP_TAG(plnnrc::Token_Type token_type) { return token_type >= Token_##FIRST_TOKEN_TAG && token_type <= Token_##LAST_TOKEN_TAG; }     \
    inline bool plnnrc::is_##GROUP_TAG(const plnnrc::Token& token) { return plnnrc::is_##GROUP_TAG(token.type); }                                                   \

#include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_TOKEN_GROUP

// FNV-1a
inline uint32_t plnnrc::hash(plnnrc::Token_Value token_value)
{
    uint32_t result = 0x811c9dc5;

    for (uint32_t i = 0; i < token_value.length; ++i)
    {
        char c = token_value.str[i];
        result ^= c;
        result *= 0x01000193;
    }

    return result;
}

#endif
