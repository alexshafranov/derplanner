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

#ifndef DERPLANNER_COMPILER_TOKEN_H_
#define DERPLANNER_COMPILER_TOKEN_H_

#include "derplanner/compiler/lexer_types.h"

namespace plnnrc {

// is_<Token_Type>
#define PLNNRC_TOKEN(TAG)                           \
    inline bool is_##TAG(Token_Type token_type)     \
    {                                               \
        return token_type == Token_##TAG;           \
    }                                               \

#include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_TOKEN

// is_<Token_Group>
#define PLNNRC_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG)                                \
    inline bool is_##GROUP_TAG(Token_Type token_type)                                           \
    {                                                                                           \
        return token_type >= Token_##FIRST_TOKEN_TAG && token_type <= Token_##LAST_TOKEN_TAG;   \
    }                                                                                           \

#include "derplanner/compiler/token_tags.inl"
#undef PLNNRC_GROUP

}

#endif
