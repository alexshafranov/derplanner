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

#include <unittestpp.h>
#include <derplanner/compiler/ast.h>
#include "compiler/token.h"

using namespace plnnrc;

namespace
{
    TEST(symbol_to_id_conversion)
    {
        {
            CHECK(!valid_id("!?"));
            CHECK(valid_id("!?a"));
            CHECK(id_len("!?1") == 2);
        }

        {
            ast::tree ast;
            char* actual = to_id(ast, "abcd0123");
            CHECK_EQUAL("abcd0123", actual);
        }

        {
            ast::tree ast;
            CHECK_EQUAL(5, id_len("!ax-by?"));
            char* actual = to_id(ast, "!ax-by?");
            CHECK_EQUAL("ax_by", actual);
        }

        {
            ast::tree ast;
            char* actual = to_id(ast, "!23a");
            CHECK_EQUAL("_23a", actual);
        }
    }
}
