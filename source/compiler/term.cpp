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

#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/term.h"

namespace plnnrc {
namespace ast {

node* build_atom(tree& ast, sexpr::node* s_expr)
{
    node* atom = ast.make_node(node_atom, s_expr->first_child);

    if (!atom)
    {
        return 0;
    }

    for (sexpr::node* v_expr = s_expr->first_child->next_sibling; v_expr != 0; v_expr = v_expr->next_sibling)
    {
        node* argument = ast.make_node(node_term_variable, v_expr);

        if (!argument)
        {
            return 0;
        }

        append_child(atom, argument);
    }

    return atom;
}

}
}
