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

#include <string>
#include "unittestpp.h"
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/parser.h"

// bring in parser implementation details.
namespace plnnrc
{
    extern ast::World* parse_world(Parser& state);
}

namespace
{
    std::string to_string(const plnnrc::ast::World* world)
    {
        std::string output;

        for (plnnrc::ast::Fact_Type* fact = world->facts; fact != 0; fact = fact->next)
        {
            output.append(fact->name.str, fact->name.length);
            output.append("[");

            for (plnnrc::ast::Fact_Param* param = fact->params; param != 0; param = param->next)
            {
                const char* token_name = plnnrc::get_token_name(param->type);
                output.append(token_name);

                if (param->next != 0)
                {
                    output.append(", ");
                }
            }

            output.append("]");

            if (fact->next != 0)
            {
                output.append(" ");
            }
        }

        return output;
    }

    TEST(world_parsing)
    {
        const char* str = "{ f1(int32) f2(float, int32) f3() }";
        const char* expected = "f1[Int32] f2[Float, Int32] f3[]";

        plnnrc::Lexer   lexer;
        plnnrc::Parser  parser;
        plnnrc::init(lexer, str);
        plnnrc::init(parser, &lexer);

        parser.token = plnnrc::lex(lexer);
        plnnrc::ast::World* world = plnnrc::parse_world(parser);
        std::string world_str = to_string(world);

        CHECK_EQUAL(expected, world_str.c_str());
    }
}
