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
#include "build_tools.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/tree_ops.h"
#include "derplanner/compiler/term.h"

namespace plnnrc {
namespace ast {

namespace
{
    PLNNRC_DEFINE_TOKEN(token_eq, "==");
}

node* build_atom(tree& ast, sexpr::node* s_expr)
{
    node_type atom_type = node_atom;

    if (is_token(s_expr->first_child, token_eq))
    {
        atom_type = node_atom_eq;
    }

    node* atom = ast.make_node(atom_type, s_expr->first_child);
    PLNNRC_CHECK(atom);

    for (sexpr::node* v_expr = s_expr->first_child->next_sibling; v_expr != 0; v_expr = v_expr->next_sibling)
    {
        plnnrc_assert(v_expr->type = sexpr::node_symbol);

        node* argument = ast.make_node(node_term_variable, v_expr);
        PLNNRC_CHECK(argument);
        append_child(atom, argument);
    }

    return atom;
}

node* build_atom_list(tree& ast, sexpr::node* s_expr)
{
    node* atom_list = ast.make_node(node_atomlist, s_expr);
    PLNNRC_CHECK(atom_list);

    for (sexpr::node* t_expr = s_expr->first_child; t_expr != 0; t_expr = t_expr->next_sibling)
    {
        node* atom = build_atom(ast, t_expr);
        PLNNRC_CHECK(atom);
        append_child(atom_list, atom);
    }

    return atom_list;
}

}
}
