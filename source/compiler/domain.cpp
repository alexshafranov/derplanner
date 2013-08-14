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
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/term.h"
#include "derplanner/compiler/logical_expression.h"
#include "derplanner/compiler/domain.h"

namespace plnnrc {
namespace ast {

namespace
{
    const char token_worldstate[]   = ":worldstate";
    const char token_domain[]       = ":domain";
    const char token_method[]       = ":method";
}

node* build_domain(tree& ast, sexpr::node* s_expr)
{
    plnnrc_assert(s_expr->type == sexpr::node_list);
    plnnrc_assert(s_expr->first_child);
    plnnrc_assert(s_expr->first_child->type == sexpr::node_symbol);
    plnnrc_assert(strncmp(s_expr->first_child->token, token_domain, sizeof(token_domain)) == 0);

    node* domain = ast.make_node(node_domain, s_expr);

    if (!domain)
    {
        return 0;
    }

    unsigned method_count = 0;

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        method_count++;
    }

    if (!ast.methods.init(method_count))
    {
        return 0;
    }

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        node* method = build_method(ast, c_expr);

        if (!method)
        {
            return 0;
        }

        append_child(domain, method);
    }

    return domain;
}

node* build_method(tree& ast, sexpr::node* s_expr)
{
    plnnrc_assert(s_expr->type == sexpr::node_list);
    plnnrc_assert(s_expr->first_child);
    plnnrc_assert(s_expr->first_child->type == sexpr::node_symbol);
    plnnrc_assert(strncmp(s_expr->first_child->token, token_method, sizeof(token_method)) == 0);

    node* method = ast.make_node(node_method, s_expr);

    if (!method)
    {
        return 0;
    }

    sexpr::node* task_atom_expr = s_expr->first_child->next_sibling;
    plnnrc_assert(task_atom_expr);
    plnnrc_assert(task_atom_expr->type == sexpr::node_list);

    node* task_atom = build_atom(ast, task_atom_expr);

    if (!task_atom)
    {
        return 0;
    }

    append_child(method, task_atom);

    ast.methods.insert(task_atom->s_expr->token, method);

    sexpr::node* branch_precond_expr = task_atom_expr->next_sibling;

    while (branch_precond_expr)
    {
        node* branch = build_branch(ast, branch_precond_expr);

        if (!branch)
        {
            return 0;
        }

        append_child(method, branch);

        branch_precond_expr = branch_precond_expr->next_sibling->next_sibling;
    }

    return method;
}

node* build_branch(tree& ast, sexpr::node* s_expr)
{
    node* branch = ast.make_node(node_branch, s_expr);

    if (!branch)
    {
        return 0;
    }

    node* precondition = build_logical_expression(ast, s_expr);

    if (!precondition)
    {
        return 0;
    }

    node* precondition_dnf = convert_to_dnf(ast, precondition);

    if (!precondition_dnf)
    {
        return 0;
    }

    append_child(branch, precondition_dnf);

    sexpr::node* tasklist_expr = s_expr->next_sibling;

    node* task_list = ast.make_node(node_tasklist, tasklist_expr);

    if (!task_list)
    {
        return 0;
    }

    for (sexpr::node* t_expr = tasklist_expr->first_child; t_expr != 0; t_expr = t_expr->next_sibling)
    {
        node* task_atom = build_atom(ast, t_expr);

        if (!task_atom)
        {
            return 0;
        }

        append_child(task_list, task_atom);
    }

    append_child(branch, task_list);

    return branch;
}

node* build_worldstate(tree& ast, sexpr::node* s_expr)
{
    plnnrc_assert(s_expr->type == sexpr::node_list);
    plnnrc_assert(s_expr->first_child);
    plnnrc_assert(s_expr->first_child->type == sexpr::node_symbol);
    plnnrc_assert(strncmp(s_expr->first_child->token, token_worldstate, sizeof(token_worldstate)) == 0);

    node* worldstate = ast.make_node(node_worldstate, s_expr);

    if (!worldstate)
    {
        return 0;
    }

    unsigned total_atom_count = 0;
    unsigned total_term_count = 0;

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        total_atom_count++;

        for (sexpr::node* t_expr = c_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
        {
            total_term_count++;
        }
    }

    if (!ast.ws_atoms.init(total_atom_count))
    {
        return 0;
    }

    if (!ast.ws_types.init(total_term_count))
    {
        return 0;
    }

    int type_tag = 1;

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        node* atom = ast.make_node(node_atom, c_expr->first_child);

        if (!atom)
        {
            return 0;
        }

        ast.ws_atoms.insert(c_expr->first_child->token, atom);

        for (sexpr::node* t_expr = c_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
        {
            node* type = ast.make_node(node_worldstate_type, t_expr);

            if (!type)
            {
                return 0;
            }

            node* type_proto = ast.ws_types.find(t_expr->first_child->token);

            if (!type_proto)
            {
                annotation<worldstate_type>(type)->type_tag = type_tag;
                ast.ws_types.insert(t_expr->first_child->token, type);
                type_tag++;
            }
            else
            {
                annotation<worldstate_type>(type)->type_tag = annotation<worldstate_type>(type_proto)->type_tag;
            }

            append_child(atom, type);
        }

        append_child(worldstate, atom);
    }

    return worldstate;
}

}
}
