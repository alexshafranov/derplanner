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
#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "ast_tools.h"
#include "error_tools.h"
#include "tree_tools.h"
#include "tokens.h"
#include "ast_logical_expression.h"
#include "ast_term.h"
#include "ast_worldstate.h"
#include "ast_infer.h"
#include "ast_annotate.h"
#include "ast_domain.h"

namespace plnnrc {
namespace ast {

namespace
{
    sexpr::node* next_branch_expr(sexpr::node* branch_expr)
    {
        if (is_token(branch_expr->first_child, token_foreach))
        {
            return branch_expr->next_sibling;
        }

        // error condition - handled in build_branch.
        if (!branch_expr->next_sibling)
        {
            return 0;
        }

        return branch_expr->next_sibling->next_sibling;
    }

    int count_elements(sexpr::node* root, str_ref token)
    {
        int result = 0;

        for (sexpr::node* c = root->first_child; c != 0; c = c->next_sibling)
        {
            if (c->type == sexpr::node_list)
            {
                if (c->first_child && c->first_child->type == sexpr::node_symbol)
                {
                    if (is_token(c->first_child, token))
                    {
                        ++result;
                    }
                }
            }
        }

        return result;
    }

    void link_to_parameter(node* parameter, node* root)
    {
        plnnrc_assert(parameter->type == node_term_variable);
        const char* id = parameter->s_expr->token;
        plnnrc_assert(id);

        for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == node_term_variable)
            {
                if (strcmp(n->s_expr->token, id) == 0)
                {
                    annotation<term_ann>(n)->var_def = parameter;
                }
            }
        }
    }

    void link_to_variable(node* variable, node* root, node* first)
    {
        plnnrc_assert(variable->type == node_term_variable);
        const char* id = variable->s_expr->token;
        plnnrc_assert(id);

        for (node* n = first; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == node_term_variable && !annotation<term_ann>(n)->var_def)
            {
                if (strcmp(n->s_expr->token, id) == 0)
                {
                    annotation<term_ann>(n)->var_def = variable;
                }
            }
        }
    }

    void link_branch_variables(tree& ast, node* method_atom, node* precondition, node* tasklist)
    {
        for (node* p = method_atom->first_child; p != 0; p = p->next_sibling)
        {
            PLNNRC_SKIP_ERROR_NODE(p);
            link_to_parameter(p, precondition);
            link_to_parameter(p, tasklist);
        }

        for (node* n = precondition; n != 0; n = preorder_traversal_next(precondition, n))
        {
            if (n->type == node_term_variable && !annotation<term_ann>(n)->var_def)
            {
                link_to_variable(n, precondition, preorder_traversal_next(precondition, n));
                link_to_variable(n, tasklist, tasklist);
            }
        }

        for (node* n = tasklist; n != 0; n = preorder_traversal_next(tasklist, n))
        {
            if (n->type == node_term_variable && !definition(n))
            {
                replace_with_error(ast, n, error_unbound_var);
            }
        }
    }

    void link_method_variables(tree& ast, node* method)
    {
        node* atom = method->first_child;
        plnnrc_assert(atom && atom->type == node_atom);

        for (node* p = atom->first_child; p != 0; p = p->next_sibling)
        {
            if (p->type != node_term_variable)
            {
                replace_with_error(ast, p, error_unexpected);
            }
        }

        for (node* branch = atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            PLNNRC_SKIP_ERROR_NODE(branch);
            node* precondition = branch->first_child;
            plnnrc_assert(precondition);
            node* tasklist = precondition->next_sibling;
            plnnrc_assert(tasklist);
            link_branch_variables(ast, atom, precondition, tasklist);
        }
    }

    void link_operator_variables(node* operatr)
    {
        node* atom = operatr->first_child;
        plnnrc_assert(atom && atom->type == node_atom);

        for (node* effect_list = atom->next_sibling; effect_list != 0; effect_list = effect_list->next_sibling)
        {
            for (node* param = atom->first_child; param != 0; param = param->next_sibling)
            {
                // stub operators can have call terms as "variables"
                if (param->type == node_term_variable)
                {
                    link_to_parameter(param, effect_list);
                }
            }
        }
    }
}

node* build_domain(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(domain, ast.make_node(node_domain, s_expr));

    PLNNRC_RETURN(expect_next_type(ast, s_expr->first_child, sexpr::node_list));
    sexpr::node* name_list_expr = s_expr->first_child->next_sibling;

    PLNNRC_CHECK_NODE(domain_namespace, build_namespace(ast, name_list_expr));
    append_child(domain, domain_namespace);

    PLNNRC_CHECK(ast.methods.init(count_elements(s_expr, token_method)));

    int num_operator_decls = count_elements(s_expr, token_operator);

    PLNNRC_CHECK(ast.operators.init(num_operator_decls > 0 ? num_operator_decls : 128));

    for (sexpr::node* c_expr = name_list_expr->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        node* element = 0;

        if (is_token(c_expr->first_child, token_method))
        {
            element = build_method(ast, c_expr);
            PLNNRC_CHECK(element);
        }

        if (is_token(c_expr->first_child, token_operator))
        {
            element = build_operator(ast, c_expr);
            PLNNRC_CHECK(element);
        }

        if (!element)
        {
            element = emit_error(ast, 0, error_unexpected, c_expr);
        }

        append_child(domain, element);
    }

    PLNNRC_CHECK(build_operator_stubs(ast));

    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        node* method = methods.value();
        node* method_atom = method->first_child;
        plnnrc_assert(method_atom && method_atom->type == node_atom);
        (void)(method_atom);

        link_method_variables(ast, method);
    }

    for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
    {
        node* operatr = operators.value();
        node* operator_atom = operatr->first_child;
        plnnrc_assert(operator_atom && operator_atom->type == node_atom);
        (void)(operator_atom);

        link_operator_variables(operatr);
    }

    return domain;
}

node* build_method(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(method, ast.make_node(node_method, s_expr));

    PLNNRC_RETURN(expect_next_type(ast, s_expr->first_child, sexpr::node_list));
    sexpr::node* task_atom_expr = s_expr->first_child->next_sibling;

    PLNNRC_CHECK_NODE(task_atom, build_atom(ast, task_atom_expr));
    append_child(method, task_atom);

    if (task_atom->type != node_error)
    {
        PLNNRC_RETURN(expect_valid_id(ast, task_atom->s_expr));
        ast.methods.insert(task_atom->s_expr->token, method);
    }

    for (sexpr::node* branch_expr = task_atom_expr->next_sibling; branch_expr != 0; branch_expr = next_branch_expr(branch_expr))
    {
        PLNNRC_CONTINUE(expect_type(ast, branch_expr, sexpr::node_list, method));
        PLNNRC_CHECK_NODE(branch, build_branch(ast, branch_expr));
        append_child(method, branch);
    }

    return method;
}

node* build_branch(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(branch, ast.make_node(node_branch, s_expr));

    sexpr::node* precondition_expr = 0;
    sexpr::node* tasklist_expr = 0;

    branch_ann* ann = annotation<branch_ann>(branch);
    ann->foreach = is_token(s_expr->first_child, token_foreach);

    if (ann->foreach)
    {
        PLNNRC_RETURN(expect_next_type(ast, s_expr->first_child, sexpr::node_list));
        precondition_expr = s_expr->first_child->next_sibling;
        PLNNRC_RETURN(expect_next_type(ast, precondition_expr, sexpr::node_list));
        tasklist_expr = precondition_expr->next_sibling;
    }
    else
    {
        precondition_expr = s_expr;
        PLNNRC_RETURN(expect_next_type(ast, precondition_expr, sexpr::node_list));
        tasklist_expr = precondition_expr->next_sibling;
    }

    PLNNRC_CHECK_NODE(precondition, build_logical_expression(ast, precondition_expr));

    if (!find_descendant(precondition, node_error))
    {
        PLNNRC_CHECK_NODE(precondition_dnf, convert_to_dnf(ast, precondition));
        precondition_dnf->s_expr = precondition_expr;
        append_child(branch, precondition_dnf);
    }
    else
    {
        append_child(branch, precondition);
    }

    PLNNRC_CHECK_NODE(task_list, build_task_list(ast, tasklist_expr));
    append_child(branch, task_list);

    return branch;
}

node* build_task_list(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(task_list, ast.make_node(node_task_list, s_expr));

    for (sexpr::node* c_expr = s_expr->first_child; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        if (is_token(c_expr->first_child, token_add) || is_token(c_expr->first_child, token_delete))
        {
            node_type type = is_token(c_expr->first_child, token_add) ? node_add_list : node_delete_list;
            PLNNRC_CHECK_NODE(task, ast.make_node(type, c_expr));

            for (sexpr::node* t_expr = c_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
            {
                PLNNRC_CONTINUE(expect_type(ast, t_expr, sexpr::node_list, task));
                PLNNRC_CHECK_NODE(atom, build_atom(ast, t_expr));
                append_child(task, atom);
            }

            append_child(task_list, task);
        }
        else
        {
            PLNNRC_CONTINUE(expect_type(ast, c_expr, sexpr::node_list, task_list));
            PLNNRC_CHECK_NODE(task, build_atom(ast, c_expr));
            append_child(task_list, task);
        }
    }

    return task_list;
}

node* build_operator(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(operatr, ast.make_node(node_operator, s_expr));

    PLNNRC_RETURN(expect_next_type(ast, s_expr->first_child, sexpr::node_list));
    sexpr::node* task_atom_expr = s_expr->first_child->next_sibling;

    PLNNRC_CHECK_NODE(task_atom, build_atom(ast, task_atom_expr));
    append_child(operatr, task_atom);

    if (task_atom->type != node_error)
    {
        PLNNRC_RETURN(expect_valid_id(ast, task_atom->s_expr));
        PLNNRC_CHECK(ast.operators.insert(task_atom->s_expr->token, operatr));
    }

    sexpr::node* delete_effects_expr = 0;
    sexpr::node* add_effects_expr = 0;

    for (sexpr::node* child = task_atom_expr->next_sibling; child != 0; child = child->next_sibling)
    {
        PLNNRC_RETURN(expect_type(ast, child, sexpr::node_list));

        if (is_token(child->first_child, token_delete))
        {
            PLNNRC_RETURN(expect_condition(ast, child, delete_effects_expr == 0, error_multiple_definitions));
            delete_effects_expr = child;
            continue;
        }

        if (is_token(child->first_child, token_add))
        {
            PLNNRC_RETURN(expect_condition(ast, child, add_effects_expr == 0, error_multiple_definitions));
            add_effects_expr = child;
            continue;
        }
    }

    PLNNRC_CHECK_NODE(delete_effects, ast.make_node(node_delete_list, delete_effects_expr));
    append_child(operatr, delete_effects);

    if (delete_effects_expr)
    {
        for (sexpr::node* t_expr = delete_effects_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
        {
            PLNNRC_CHECK_NODE(atom, build_atom(ast, t_expr));
            append_child(delete_effects, atom);
        }
    }

    PLNNRC_CHECK_NODE(add_effects, ast.make_node(node_add_list, add_effects_expr));
    append_child(operatr, add_effects);

    if (add_effects_expr)
    {
        for (sexpr::node* t_expr = add_effects_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
        {
            PLNNRC_CHECK_NODE(atom, build_atom(ast, t_expr));
            append_child(add_effects, atom);
        }
    }

    return operatr;
}

node* build_operator_stub(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_RETURN(expect_valid_id(ast, s_expr));

    PLNNRC_CHECK_NODE(operatr, ast.make_node(node_operator, s_expr->parent));
    PLNNRC_CHECK(ast.operators.insert(s_expr->token, operatr));

    PLNNRC_CHECK_NODE(operator_atom, build_atom(ast, s_expr->parent));
    append_child(operatr, operator_atom);

    PLNNRC_CHECK_NODE(delete_effects, ast.make_node(node_delete_list));
    append_child(operatr, delete_effects);

    PLNNRC_CHECK_NODE(add_effects, ast.make_node(node_add_list));
    append_child(operatr, add_effects);

    return operatr;
}

bool build_operator_stubs(tree& ast)
{
    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        node* method = methods.value();
        node* method_atom = method->first_child;
        plnnrc_assert(method_atom && method_atom->type == node_atom);

        for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            PLNNRC_SKIP_ERROR_NODE(branch);

            plnnrc_assert(branch->first_child);
            node* tasklist = branch->first_child->next_sibling;
            plnnrc_assert(tasklist);

            for (node* task = tasklist->first_child; task != 0; task = task->next_sibling)
            {
                PLNNRC_SKIP_SUBTREE_WITH_ERRORS(task);

                if (task->type == node_atom && !is_method(ast, task))
                {
                    if (ast.operators.find(task->s_expr->token))
                    {
                        // check number of arguments here.
                        continue;
                    }

                    PLNNRC_CHECK_NODE(operatr, build_operator_stub(ast, task->s_expr));
                }
            }
        }
    }

    return true;
}

}
}
