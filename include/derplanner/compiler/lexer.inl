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

#ifndef DERPLANNER_COMPILER_LEXER_INL_
#define DERPLANNER_COMPILER_LEXER_INL_

inline plnnrc::Location plnnrc::get_loc(const plnnrc::Lexer& state)
{
    return state.loc;
}

inline bool plnnrc::has_value(const plnnrc::Token& tok) { return tok.value.length > 0; }

inline bool plnnrc::is_Error(const Token& tok) { return tok.error; }

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
