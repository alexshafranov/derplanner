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
#include <derplanner/compiler/assert.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include "compiler/tree_tools.h"
#include "compiler/ast_logical_expression.h"

using namespace plnnrc;

namespace
{
    std::string to_string(ast::Node* root)
    {
        std::string result;

        switch (root->type)
        {
        case ast::node_op_and:
            result += "(and";

            if (root->first_child)
            {
                result += " ";
            }

            break;
        case ast::node_op_or:
            result += "(or";

            if (root->first_child)
            {
                result += " ";
            }

            break;
        case ast::node_op_not:
            result += "(not";

            if (root->first_child)
            {
                result += " ";
            }

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
            for (ast::Node* n = root->first_child; n != 0; n = n->next_sibling)
            {
                result += to_string(n);

                if (!plnnrc::is_last(n))
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
        sexpr::Tree expr;
        char buffer[] = "((or (a) (b) (c)))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        const char* expected = "(and (or (a) (b) (c)))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(build_2)
    {
        sexpr::Tree expr;
        char buffer[] = "((not (not (x))))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        const char* expected = "(and (not (not (x))))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(nnf_conversion_trivial)
    {
        {
            sexpr::Tree expr;
            char buffer[] = "()";
            expr.parse(buffer);
            ast::Tree tree;
            ast::Node* actual = ast::convert_to_nnf(tree, ast::build_logical_expression(tree, expr.root()->first_child));
            const char* expected = "(and)";
            CHECK_EQUAL(expected, to_string(actual).c_str());
        }

        {
            sexpr::Tree expr;
            char buffer[] = "((or (x) (y)))";
            expr.parse(buffer);
            ast::Tree Tree;
            ast::Node* actual = ast::convert_to_nnf(Tree, ast::build_logical_expression(Tree, expr.root()->first_child));
            const char* expected = "(and (or (x) (y)))";
            CHECK_EQUAL(expected, to_string(actual).c_str());
        }
    }

    TEST(nnf_conversion_double_negative)
    {
        sexpr::Tree expr;
        char buffer[] = "((not (not (x))))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        actual = ast::convert_to_nnf(tree, actual);
        const char* expected = "(and (x))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(nnf_conversion_non_root_node)
    {
        sexpr::Tree expr;
        char buffer[] = "((not (not (x))))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        // build_logical_expression always returns (and ...) as a root
        // move to the first child of (and ...) 
        // to test 'convert_to_nnf' working on a non root nodes of Tree.
        actual = actual->first_child;
        actual = ast::convert_to_nnf(tree, actual);
        const char* expected = "(x)";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(nnf_conversion_de_morgans)
    {
        sexpr::Tree expr;
        char buffer[] = "((not (and (x) (or (y) (not (z))))))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        actual = ast::convert_to_nnf(tree, actual->first_child);
        // !(x && (y || !z)) -> !x || !(y || z) -> !x || (!y && z)
        const char* expected = "(or (not (x)) (and (not (y)) (z)))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(flatten_op_chains)
    {
        sexpr::Tree expr;
        char buffer[] = "((not (and (a) (and (b) (or (c) (d) (or (e) (f))) (and (g) (h))))))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        ast::flatten(actual);
        const char* expected = "(and (not (and (a) (b) (or (c) (d) (e) (f)) (g) (h))))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(empty_is_dnf)
    {
        sexpr::Tree expr;
        char buffer[] = "()";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        actual = ast::convert_to_dnf(tree, actual);
        const char* expected = "(or (and))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(literal_is_dnf)
    {
        {
            sexpr::Tree expr;
            char buffer[] = "((x))";
            expr.parse(buffer);
            ast::Tree tree;
            ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
            actual = ast::convert_to_dnf(tree, actual);
            const char* expected = "(or (and (x)))";
            CHECK_EQUAL(expected, to_string(actual).c_str());
        }

        {
            sexpr::Tree expr;
            char buffer[] = "((not (x)))";
            expr.parse(buffer);
            ast::Tree tree;
            ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
            actual = ast::convert_to_dnf(tree, actual);
            const char* expected = "(or (and (not (x))))";
            CHECK_EQUAL(expected, to_string(actual).c_str());
        }
    }

    TEST(dnf_conversion_1)
    {
        sexpr::Tree expr;
        char buffer[] = "((and (q1) (or (r1) (r2)) (q2) (or (r3) (r4)) (q3)))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        actual = ast::convert_to_dnf(tree, actual);
        const char* expected = "(or (and (q1) (r1) (q2) (r3) (q3)) (and (q1) (r1) (q2) (r4) (q3)) (and (q1) (r2) (q2) (r3) (q3)) (and (q1) (r2) (q2) (r4) (q3)))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(dnf_conversion_2)
    {
        sexpr::Tree expr;
        char buffer[] = "((a) (not (or (b) (c))))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        actual = ast::convert_to_dnf(tree, actual);
        const char* expected = "(or (and (a) (not (b)) (not (c))))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(dnf_conversion_3)
    {
        sexpr::Tree expr;
        char buffer[] = "((or (a) (b)))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        actual = ast::convert_to_dnf(tree, actual);
        const char* expected = "(or (and (a)) (and (b)))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }

    TEST(dnf_conversion_4)
    {
        sexpr::Tree expr;
        char buffer[] = "((t1) (or (not (t2)) (and (t2) (t3) (t4))))";
        expr.parse(buffer);
        ast::Tree tree;
        ast::Node* actual = ast::build_logical_expression(tree, expr.root()->first_child);
        actual = ast::convert_to_dnf(tree, actual);
        const char* expected = "(or (and (t1) (not (t2))) (and (t1) (t2) (t3) (t4)))";
        CHECK_EQUAL(expected, to_string(actual).c_str());
    }
}
