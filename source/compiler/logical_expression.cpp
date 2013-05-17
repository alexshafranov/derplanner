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

#include <string.h>
#include "derplanner/compiler/derplanner_assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/logical_expression.h"

namespace plnnrc {
namespace ast {

namespace
{
    const char token_and[] = "and";
    const char token_or[]  = "or";
    const char token_not[] = "not";

    // forward
    node* build_recursive(tree& t, sexpr::node* s_expr);

    node* build_logical_op(tree& t, sexpr::node* s_expr, node_type op_type)
    {
        node* root = t.make_node(op_type, s_expr);
        plnnrc_assert(root != 0);
        plnnrc_assert(s_expr->first_child != 0);

        for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
        {
            node* child = build_recursive(t, c_expr);
            append_child(root, child);
        }

        return root;
    }

    node* build_atom(tree& t, sexpr::node* s_expr)
    {
        node* root = t.make_node(node_atom, s_expr->first_child);
        plnnrc_assert(root != 0);
        return root;
    }

    node* build_recursive(tree& t, sexpr::node* s_expr)
    {
        plnnrc_assert(s_expr->type == sexpr::node_list);
        sexpr::node* c_expr = s_expr->first_child;
        plnnrc_assert(c_expr && c_expr->type == sexpr::node_symbol);

        if (strncmp(c_expr->token, token_and, sizeof(token_and)) == 0)
        {
            return build_logical_op(t, s_expr, node_op_and);
        }

        if (strncmp(c_expr->token, token_or, sizeof(token_or)) == 0)
        {
            return build_logical_op(t, s_expr, node_op_or);
        }

        if (strncmp(c_expr->token, token_not, sizeof(token_not)) == 0)
        {
            return build_logical_op(t, s_expr, node_op_not);
        }

        return build_atom(t, s_expr);
    }
}

node* build_logical_expression(tree& t, sexpr::node* s_expr)
{
    plnnrc_assert(s_expr->type == sexpr::node_list);

    node* root = t.make_node(node_op_and, s_expr);
    plnnrc_assert(root != 0);

    for (sexpr::node* c_expr = s_expr->first_child; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        node* child = build_recursive(t, c_expr);
        append_child(root, child);
    }

    return root;
}

node* convert_to_nnf(tree& t, node* root)
{
    if (root->type == node_atom)
    {
        return root;
    }

    if (is_logical_op(root))
    {
        if (root->type == node_op_not)
        {
            return root;
        }
        else
        {
            return root;
        }
    }

    plnnrc_assert(false);
    return 0;
}

}
}
