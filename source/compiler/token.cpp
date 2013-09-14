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

#include <stdio.h>
#include <ctype.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/ast.h"
#include "token.h"

namespace plnnrc {

int id_len(const char* symbol)
{
    const char* c;

    for (c = symbol; *c != 0; ++c)
    {
        if (*c != '?' && *c != '!')
        {
            break;
        }
    }

    if (*c == 0)
    {
        return 0;
    }

    int length = 0;

    if (isdigit(*c))
    {
        ++length;
    }

    for (; *c != 0; ++c)
    {
        if (isalnum(*c) || *c == '_' || *c == '-')
        {
            ++length;
            continue;
        }

        if (*c == '!' || *c == '?')
        {
            continue;
        }

        return -1;
    }

    return length;
}

char* to_id(ast::tree& ast, const char* symbol)
{
    int length = id_len(symbol);

    plnnrc_assert(length > 0);

    char* token = ast.make_token(length);

    if (!token)
    {
        return 0;
    }

    char* t = token;
    const char* c = symbol;

    for (; *c != 0; ++c)
    {
        if (*c != '?' && *c != '!')
        {
            break;
        }
    }

    if (isdigit(*c))
    {
        *t++ = '_';
    }

    for (; *c != 0; ++c)
    {
        if (*c == '?' || *c == '!')
        {
            continue;
        }

        if (isalnum(*c) || *c == '_')
        {
            *t++ = *c;
            continue;
        }

        if (*c == '-')
        {
            *t++ = '_';
            continue;
        }
    }

    return token;
}

}
