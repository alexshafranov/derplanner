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

const char token_and[] = "and";
const char token_or[]  = "or";
const char token_not[] = "not";

node* build_logical_expression(tree& t, sexpr::node* s_expr)
{
    plnnrc_assert(s_expr->type == sexpr::node_list);

    node* root = t.make_node(node_op_and);

    if (!root)
    {
        return 0;
    }

    node* parent = root;
    sexpr::node* pexpr = s_expr;
    sexpr::node* cexpr = s_expr->first_child;

    while (cexpr != 0)
    {
        node* node = 0;

        if (strncmp(token_and, cexpr->token, sizeof(token_and)) == 0)
        {
            node = t.make_node(node_op_and);
        }
        else if (strncmp(token_or, cexpr->token, sizeof(token_or)) == 0)
        {
            node = t.make_node(node_op_or);
        }
        else if (strncmp(token_not, cexpr->token, sizeof(token_not)) == 0)
        {
            node = t.make_node(node_op_not);
        }
        else
        {
            node = t.make_node(node_atom);
        }

        if (!node)
        {
            return 0;
        }

        append_child(parent, node);
        node->s_expr = cexpr;

        if (cexpr->first_child)
        {
            pexpr = cexpr;
            cexpr = cexpr->first_child;
            parent = node;
        }
        else
        {
            if (cexpr->next_sibling)
            {
                cexpr = cexpr->next_sibling;
            }
            else
            {
                cexpr = (pexpr != s_expr) ? pexpr->next_sibling : 0;
                pexpr = pexpr->parent;
                parent = parent->parent;
            }
        }
    }

    return root;
}

}
}
