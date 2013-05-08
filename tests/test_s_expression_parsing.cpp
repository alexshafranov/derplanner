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
#include <string>
#include <unittestpp.h>
#include <derplanner/compiler/s_expression.h>

using namespace derplanner::s_expression;

namespace
{
    std::string to_string(node* root)
    {
        std::string result;

        if (root->type == node_list)
        {
            result += "(";

            for (node* n = root->first_child; n != 0; n = n->next_sibling)
            {
                result += to_string(n);

                if (n != root->first_child->prev_sibling_cyclic)
                {
                    result += " ";
                }
            }

            result += ")";
        }
        else
        {
            result = root->token;
        }

        return result;
    }

    TEST(symbol_delimeters)
    {
        tree s_exp;
        char buffer[] = "(hello world  hello\nplanner)";
        s_exp.parse(buffer);
        std::string actual = to_string(s_exp.root);
        CHECK_EQUAL("(hello world hello planner)", actual.c_str());
    }

    TEST(simple_hierarchy)
    {
        tree s_exp;
        char buffer[] = "(hello (world) (hello planner))";
        s_exp.parse(buffer);
        std::string actual = to_string(s_exp.root);
        CHECK_EQUAL("(hello (world) (hello planner))", actual.c_str());
    }

    TEST(line_number)
    {
        tree s_exp;
        char buffer[] = "(hello\nworld)";
        s_exp.parse(buffer);
        CHECK_EQUAL(1, s_exp.root->first_child->line);
        CHECK_EQUAL(2, s_exp.root->first_child->next_sibling->line);
    }
}
