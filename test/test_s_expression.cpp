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
#include <string.h>
#include <unittestpp.h>
#include "compiler/tree_tools.h"
#include <derplanner/compiler/s_expression.h>

using namespace plnnrc::sexpr;

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

                if (!plnnrc::is_last(n))
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

    TEST(empty)
    {
        tree s_exp;
        char buffer[] = "";
        s_exp.parse(buffer);
        std::string actual = to_string(s_exp.root());
        CHECK_EQUAL("()", actual.c_str());
    }

    TEST(symbol_delimeters)
    {
        tree s_exp;
        char buffer[] = "(hello world  hello\nplanner)";
        s_exp.parse(buffer);
        std::string actual = to_string(s_exp.root());
        CHECK_EQUAL("((hello world hello planner))", actual.c_str());
    }

    TEST(simple_hierarchy)
    {
        tree s_exp;
        char buffer[] = "(hello (world) (hello planner))";
        s_exp.parse(buffer);
        std::string actual = to_string(s_exp.root());
        CHECK_EQUAL("((hello (world) (hello planner)))", actual.c_str());
    }

    TEST(line_number)
    {
        tree s_exp;
        char buffer[] = "(hello\nworld)";
        s_exp.parse(buffer);
        CHECK_EQUAL(1, s_exp.root()->first_child->first_child->line);
        CHECK_EQUAL(2, s_exp.root()->first_child->first_child->next_sibling->line);
    }

    TEST(column_number)
    {
        tree s_exp;
        char buffer[] = "(hello world)";
        s_exp.parse(buffer);
        CHECK_EQUAL(2, s_exp.root()->first_child->first_child->column);
        CHECK_EQUAL(8, s_exp.root()->first_child->first_child->next_sibling->column);
    }

    TEST(column_number_is_reset_on_newline)
    {
        tree s_exp;
        char buffer[] = "(hello\nworld)";
        s_exp.parse(buffer);
        CHECK_EQUAL(2, s_exp.root()->first_child->first_child->column);
        CHECK_EQUAL(1, s_exp.root()->first_child->first_child->next_sibling->column);
    }

    TEST(line_comment)
    {
        tree s_exp;
        // children:       n1                           n2
        char buffer[] = "(hello ; world \n ; hello \n planner)";
        s_exp.parse(buffer);
        std::string actual = to_string(s_exp.root());
        CHECK_EQUAL("((hello planner))", actual.c_str());
        node* n1 = s_exp.root()->first_child->first_child;
        node* n2 = n1->next_sibling;
        CHECK_EQUAL(1, n1->line);
        CHECK_EQUAL(2, n1->column);
        CHECK_EQUAL(3, n2->line);
        CHECK_EQUAL(2, n2->column);
    }

    TEST(symbol_end_location)
    {
        tree s_exp;
        char buffer[] = "(hello)";
        s_exp.parse(buffer);
        node* n = s_exp.root()->first_child->first_child;
        CHECK_EQUAL(7, n->column_end);
    }

    TEST(list_end_location)
    {
        tree s_exp;
        char buffer[] = "(hello\n(world\nhello (planner)))";
        s_exp.parse(buffer);
        node* n = s_exp.root()->first_child->first_child->next_sibling;
        CHECK_EQUAL(3, n->line_end);
        CHECK_EQUAL(16, n->column_end);
    }

    TEST(multiple_top_level_lists)
    {
        tree s_exp;
        char buffer[] = "(hello) (world)";
        s_exp.parse(buffer);
        std::string actual = to_string(s_exp.root());
        CHECK_EQUAL("((hello) (world))", actual.c_str());
    }

    void check_number(const char* str, node_type expected)
    {
        char buffer[128];
        for (size_t i=0; i<strlen(str)+1 && i<128; ++i) { buffer[i] = str[i]; }
        tree s_exp;
        s_exp.parse(buffer);
        CHECK_EQUAL(expected, s_exp.root()->first_child->first_child->type);
    }

    void check_number(const char* str, float expected)
    {
        char buffer[128];
        for (size_t i=0; i<strlen(str)+1 && i<128; ++i) { buffer[i] = str[i]; }
        tree s_exp;
        s_exp.parse(buffer);
        CHECK_EQUAL(expected, as_float(s_exp.root()->first_child->first_child));
    }

    void check_number(const char* str, int expected)
    {
        char buffer[128];
        for (size_t i=0; i<strlen(str)+1 && i<128; ++i) { buffer[i] = str[i]; }
        tree s_exp;
        s_exp.parse(buffer);
        CHECK_EQUAL(expected, as_int(s_exp.root()->first_child->first_child));
    }

    TEST(numbers)
    {
        check_number("(123)", node_int);
        check_number("(+123)", node_int);
        check_number("(-123)", node_int);
        check_number("(123.)", node_float);
        check_number("(.123)", node_float);
        check_number("(+.123)", node_float);
        check_number("(-.123)", node_float);
        check_number("(+123.)", node_float);
        check_number("(-123.)", node_float);
        check_number("(1.23)", node_float);
        check_number("(+1.23)", node_float);
        check_number("(-1.23)", node_float);
        check_number("(.1e+23)", node_float);
        check_number("(+.1e+23)", node_float);
        check_number("(-.1e+23)", node_float);
        check_number("(1e23)", node_float);
        check_number("(1e-23)", node_float);
        check_number("(1e+23)", node_float);
        check_number("(+123x)", node_symbol);
        check_number("(e+10)", node_symbol);
        check_number("(123)", 123);
        check_number("(1.23)", 1.23f);
    }

    TEST(mismatching_parentheses)
    {
        {
            char buffer[] = "(hello world))";
            tree s_exp;
            parse_result actual = s_exp.parse(buffer);
            CHECK_EQUAL(parse_excess_close, actual.status);
            CHECK_EQUAL(14, actual.column);
        }

        {
            char buffer[] = "(hello (world (world (world)";
            tree s_exp;
            parse_result actual = s_exp.parse(buffer);
            CHECK_EQUAL(parse_excess_open, actual.status);
            CHECK_EQUAL(15, actual.column);
        }

        {
            char buffer[] = "(hello) (world)";
            tree s_exp;
            parse_result actual = s_exp.parse(buffer);
            CHECK_EQUAL(parse_ok, actual.status);
        }
    }

    TEST(glue_tokens)
    {
        char buffer[] = "(hello world)";
        tree s_exp;
        s_exp.parse(buffer);
        glue_tokens(s_exp.root()->first_child);
        CHECK_EQUAL("hello world", s_exp.root()->first_child->first_child->token);
        CHECK(plnnrc::is_last(s_exp.root()->first_child->first_child));
    }

    TEST(scan_number_rollback)
    {
        char buffer[] = "(-123>)";
        tree s_exp;
        s_exp.parse(buffer);
        CHECK_EQUAL(node_symbol, s_exp.root()->first_child->first_child->type);
        CHECK_EQUAL("-123>", s_exp.root()->first_child->first_child->token);
    }
}
