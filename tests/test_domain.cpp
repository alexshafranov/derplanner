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
#include <derplanner/compiler/domain.h>

using namespace plnnrc;

#define NODE_TO_STRING_CASE(NODE_TYPE, NODE, INCLUDE_TOKEN)             \
case NODE_TYPE:                                                         \
    if ((INCLUDE_TOKEN))                                                \
    {                                                                   \
        return std::string(#NODE_TYPE) + " " + (NODE)->s_expr->token;   \
    }                                                                   \
    else                                                                \
    {                                                                   \
        return std::string(#NODE_TYPE);                                 \
    }                                                                   \

std::string node_to_string(ast::node* node)
{
    switch (node->type)
    {
    NODE_TO_STRING_CASE(ast::node_domain, node, false)
    NODE_TO_STRING_CASE(ast::node_method, node, false)
    NODE_TO_STRING_CASE(ast::node_branch, node, false)
    NODE_TO_STRING_CASE(ast::node_tasklist, node, false)
    NODE_TO_STRING_CASE(ast::node_operator, node, false)
    NODE_TO_STRING_CASE(ast::node_op_and, node, false)
    NODE_TO_STRING_CASE(ast::node_op_or, node, false)
    NODE_TO_STRING_CASE(ast::node_op_not, node, false)
    NODE_TO_STRING_CASE(ast::node_atom, node, true)
    NODE_TO_STRING_CASE(ast::node_term_variable, node, true)
    NODE_TO_STRING_CASE(ast::node_term_int, node, true)
    NODE_TO_STRING_CASE(ast::node_term_float, node, true)
    NODE_TO_STRING_CASE(ast::node_term_call, node, true)
    NODE_TO_STRING_CASE(ast::node_error, node, true)
    default:
        plnnrc_assert(false);
        return std::string();
    }
}

#undef NODE_TO_STRING_CASE

std::string to_string(ast::node* root, int level=0)
{
    std::string result;

    for (int i = 0; i < level; ++i)
    {
        result += "    ";
    }

    result += node_to_string(root);

    for (ast::node* child = root->first_child; child != 0; child = child->next_sibling)
    {
        result += "\n" + to_string(child, level+1);
    }

    return result;
}

namespace
{
    TEST(empty_domain)
    {
        char buffer[] = "(:domain)";
        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual);
        CHECK(!actual->first_child);
    }

    TEST(domain_ast_structure)
    {
        char buffer[] = \
"(:domain                         "
"    (:method (root)              "
"        ((start ?s) (finish ?f)) "
"        ((travel ?s ?f))         "
"    )                            "
")                                ";

        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual_tree = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual_tree);
        std::string actual_str = to_string(actual_tree);

        const char* expected = \
"ast::node_domain\n"
"    ast::node_method\n"
"        ast::node_atom root\n"
"        ast::node_branch\n"
"            ast::node_op_or\n"
"                ast::node_op_and\n"
"                    ast::node_atom start\n"
"                    ast::node_atom finish\n"
"            ast::node_tasklist\n"
"                ast::node_atom travel\n"
"                    ast::node_term_variable ?s\n"
"                    ast::node_term_variable ?f";

        CHECK_EQUAL(expected, actual_str.c_str());
    }
}
