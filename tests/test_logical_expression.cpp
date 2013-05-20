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

#include <string>
#include <unittestpp.h>
#include <derplanner/compiler/derplanner_assert.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/logical_expression.h>

using namespace plnnrc;

namespace
{
    std::string to_string(ast::node* root)
    {
        std::string result;

        switch (root->type)
        {
        case ast::node_op_and:
            result += "(and ";
            break;
        case ast::node_op_or:
            result += "(or ";
            break;
        case ast::node_op_not:
            result += "(not ";
            break;
        case ast::node_atom:
            result += "(";
            plnnrc_assert(root->s_expr && root->s_expr->token);
            result += root->s_expr->token;
            result += ")";
            break;
        default:
            break;
        }

        if (is_logical_op(root))
        {
            for (ast::node* n = root->first_child; n != 0; n = n->next_sibling)
            {
                result += to_string(n);

                if (n != root->first_child->prev_sibling_cyclic)
                {
                    result += " ";
                }
            }

            result += ")";
        }

        return result;
    }

    TEST(build_1)
    {
        sexpr::tree expr;
        char buffer[] = "((or (a) (b) (c)))";
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::build_logical_expression(tree, expr.root());
        const char* expected = "(and (or (a) (b) (c)))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(build_2)
    {
        sexpr::tree expr;
        char buffer[] = "((not (not (x))))";
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::build_logical_expression(tree, expr.root());
        const char* expected = "(and (not (not (x))))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(nnf_conversion_trivial)
    {
        sexpr::tree expr;
        char buffer[] = "((or (x) (y)))";
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::convert_to_nnf(tree, ast::build_logical_expression(tree, expr.root()));
        const char* expected = "(and (or (x) (y)))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(nnf_conversion_double_negative)
    {
        sexpr::tree expr;
        char buffer[] = "((not (not (x))))";
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::convert_to_nnf(tree, ast::build_logical_expression(tree, expr.root()));
        const char* expected = "(and (x))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(nnf_conversion_de_morgans)
    {
        sexpr::tree expr;
        char buffer[] = "((not (and (x) (or (y) (not (z))))))";
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::convert_to_nnf(tree, ast::build_logical_expression(tree, expr.root()));
        // !(x && (y || !z)) -> !x || !(y || z) -> !x || (!y && z)
        const char* expected = "(and (or (not (x)) (and (not (y)) (z))))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(flatten_op_chains)
    {
        sexpr::tree expr;
        char buffer[] = "((not (and (a) (and (b) (or (c) (d) (or (e) (f))) (and (g) (h))))))";
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::build_logical_expression(tree, expr.root());
        ast::flatten(actual);
        const char* expected = "(and (not (and (a) (b) (or (c) (d) (e) (f)) (g) (h))))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }
}
