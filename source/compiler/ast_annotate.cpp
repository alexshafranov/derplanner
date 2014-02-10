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

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/ast.h"
#include "tree_tools.h"
#include "ast_tools.h"
#include "ast_infer.h"

namespace plnnrc {
namespace ast {

void annotate_precondition(node* precondition)
{
    for (node* n = precondition; n != 0; n = preorder_traversal_next(precondition, n))
    {
        if (is_term_variable(n))
        {
            node* def = definition(n);

            if (def)
            {
                annotation<term_ann>(def)->var_index = -1;
            }
        }
    }

    int var_index = 0;

    for (node* n = precondition; n != 0; n = preorder_traversal_next(precondition, n))
    {
        if (is_term_variable(n))
        {
            node* def = definition(n);

            if (!def || annotation<term_ann>(def)->var_index == -1)
            {
                if (def)
                {
                    annotation<term_ann>(def)->var_index = var_index;
                    annotation<term_ann>(n)->var_index = var_index;
                }
                else
                {
                    annotation<term_ann>(n)->var_index = var_index;
                }

                ++var_index;
            }
            else
            {
                annotation<term_ann>(n)->var_index = annotation<term_ann>(def)->var_index;
            }
        }
    }

    int atom_index = 0;

    for (node* n = precondition; n != 0; n = preorder_traversal_next(precondition, n))
    {
        if (is_atom(n))
        {
            annotation<atom_ann>(n)->index = atom_index;
            ++atom_index;
        }
    }
}

void annotate_params(node* task)
{
    node* atom = task->first_child;

    int param_index = 0;

    for (node* param = atom->first_child; param != 0; param = param->next_sibling)
    {
        annotation<term_ann>(param)->var_index = param_index;
        ++param_index;
    }
}

void annotate(tree& ast)
{
    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        node* method = methods.value();

        for (node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            plnnrc_assert(is_branch(branch));
            annotate_precondition(branch->first_child);
        }
    }

    for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
    {
        annotate_params(operators.value());
    }

    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        annotate_params(methods.value());
    }
}

}
}
