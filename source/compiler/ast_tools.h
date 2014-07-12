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

#include "tree_tools.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"

namespace plnnrc {
namespace ast {

inline bool is_bound(Node* var)
{
    plnnrc_assert(is_term_variable(var));
    return annotation<Term_Ann>(var)->var_def != 0;
}

inline bool all_bound(Node* atom)
{
    for (Node* arg = atom->first_child; arg != 0; arg = arg->next_sibling)
    {
        if (!is_term_variable(arg))
        {
            continue;
        }

        if (!is_bound(arg))
        {
            return false;
        }
    }

    return true;
}

inline bool all_unbound(Node* atom)
{
    for (Node* arg = atom->first_child; arg != 0; arg = arg->next_sibling)
    {
        if (!is_term_variable(arg) || is_bound(arg))
        {
            return false;
        }
    }

    return true;
}

inline Node* definition(Node* var)
{
    plnnrc_assert(is_term_variable(var));
    return annotation<Term_Ann>(var)->var_def;
}

inline bool is_method_parameter(Node* var)
{
    plnnrc_assert(is_term_variable(var));
    plnnrc_assert(var->parent && var->parent->parent);
    return is_method(var->parent->parent);
}

inline bool is_operator_parameter(Node* var)
{
    plnnrc_assert(is_term_variable(var));
    plnnrc_assert(var->parent && var->parent->parent);
    return is_operator(var->parent->parent);
}

inline bool is_parameter(Node* var)
{
    return is_method_parameter(var) || is_operator_parameter(var);
}

inline bool has_parameters(Node* task)
{
    Node* atom = task->first_child;
    plnnrc_assert(atom && is_atom(atom));
    return atom->first_child != 0;
}

inline int type_tag(Node* Node)
{
    plnnrc_assert(is_term(Node));
    return annotation<Term_Ann>(Node)->type_tag;
}

inline void type_tag(Node* Node, int new_type_tag)
{
    plnnrc_assert(is_term(Node));
    annotation<Term_Ann>(Node)->type_tag = new_type_tag;
}

inline Node* first_parameter_usage(Node* parameter, Node* precondition)
{
    for (Node* var = precondition; var != 0; var = preorder_traversal_next(precondition, var))
    {
        if (is_term_variable(var) && parameter == definition(var))
        {
            return var;
        }
    }

    return 0;
}

inline bool is_lazy(Node* atom)
{
    return is_atom(atom) && annotation<Atom_Ann>(atom)->lazy;
}

inline bool is_operator(Tree& ast, Node* atom)
{
    return is_atom(atom) && ast.operators.find(atom->s_expr->token);
}

inline bool is_method(Tree& ast, Node* atom)
{
    return is_atom(atom) && ast.methods.find(atom->s_expr->token);
}

inline Node* find_child(Node* parent, Node_Type type)
{
    for (Node* child = parent->first_child; child != 0; child = child->next_sibling)
    {
        if (child->type == type)
        {
            return child;
        }
    }

    return 0;
}

inline Node* find_descendant(Node* parent, Node_Type type)
{
    for (Node* n = parent; n != 0; n = preorder_traversal_next(parent, n))
    {
        if (n->type == type)
        {
            return n;
        }
    }

    return 0;
}

}
}

#endif
