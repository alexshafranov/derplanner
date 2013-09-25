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

#ifndef DERPLANNER_COMPILER_AST_TOOLS_H_
#define DERPLANNER_COMPILER_AST_TOOLS_H_

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/tree_ops.h"

namespace plnnrc {
namespace ast {

inline bool is_bound(node* var)
{
    plnnrc_assert(var->type == node_term_variable);
    return annotation<term_ann>(var)->var_def != 0;
}

inline node* definition(node* var)
{
    plnnrc_assert(var->type == node_term_variable);
    return annotation<term_ann>(var)->var_def;
}

inline bool is_parameter(node* var)
{
    plnnrc_assert(var->type == node_term_variable);
    plnnrc_assert(var->parent && var->parent->parent);
    return var->parent->parent->type == node_method || var->parent->parent->type == node_operator;
}

inline bool has_parameters(node* task)
{
    node* atom = task->first_child;
    plnnrc_assert(atom);
    plnnrc_assert(atom->type == node_atom);
    return atom->first_child != 0;
}

inline int type_tag(node* node)
{
    plnnrc_assert(is_term(node));
    return annotation<term_ann>(node)->type_tag;
}

inline void type_tag(node* node, int new_type_tag)
{
    plnnrc_assert(is_term(node));
    annotation<term_ann>(node)->type_tag = new_type_tag;
}

inline node* first_parameter_usage(node* parameter, node* precondition)
{
    for (node* var = precondition; var != 0; var = preorder_traversal_next(precondition, var))
    {
        if (var->type == node_term_variable && parameter == definition(var))
        {
            return var;
        }
    }

    return 0;
}

inline bool is_operator(tree& ast, node* atom)
{
    plnnrc_assert(atom->type == node_atom);
    return !ast.methods.find(atom->s_expr->token);
}

}
}

#endif
