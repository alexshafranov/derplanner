//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
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

#ifndef DERPLANNER_COMPILER_TOKENS_H_
#define DERPLANNER_COMPILER_TOKENS_H_

#include <string.h>
#include "derplanner/compiler/s_expression.h"

namespace plnnrc
{
    struct Str_Ref
    {
        const char* str;
        size_t len;
    };

    inline bool is_token(::plnnrc::sexpr::Node* s_expr, ::plnnrc::Str_Ref token)
    {
        return s_expr && s_expr->token && strncmp(s_expr->token, token.str, token.len) == 0;
    }
}

#define PLNNRC_DECLARE_TOKEN(NAME, STR) extern const ::plnnrc::Str_Ref NAME
#define PLNNRC_DEFINE_TOKEN(NAME, STR) const ::plnnrc::Str_Ref NAME = { STR, sizeof(STR) }

namespace plnnrc {

#define PLNNRC_TOKEN(NAME, STR) PLNNRC_DECLARE_TOKEN(NAME, STR);
#include "token_tags.inl"
#undef PLNNRC_TOKEN

}

#endif
