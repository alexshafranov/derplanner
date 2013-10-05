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
#include "formatter.h"
#include "ast_tools.h"
#include "build_tools.h"
#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/term.h"
#include "derplanner/compiler/logical_expression.h"
#include "derplanner/compiler/tree_ops.h"
#include "derplanner/compiler/domain.h"

namespace plnnrc {
namespace ast {

namespace
{
    const char token_worldstate[]   = ":worldstate";
    const char token_domain[]       = ":domain";
    const char token_method[]       = ":method";
    const char token_operator[]     = ":operator";
    const char token_foreach[]      = ":foreach";
    const char token_add[]          = ":add";
    const char token_delete[]       = ":delete";

    node* build_method(tree& ast, sexpr::node* s_expr);
    node* build_branch(tree& ast, sexpr::node* s_expr);
    node* build_operator(tree& ast, sexpr::node* s_expr);
    node* build_operator_stub(tree& ast, sexpr::node* s_expr);

    bool build_operator_stubs(tree& ast);

    sexpr::node* next_branch_expr(sexpr::node* branch_expr);

    int count_elements(sexpr::node* root, const char* token)
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

    void link_branch_variables(node* method_atom, node* precondition, node* tasklist)
    {
        for (node* p = method_atom->first_child; p != 0; p = p->next_sibling)
        {
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
    }

    void link_method_variables(node* method)
    {
        node* atom = method->first_child;
        plnnrc_assert(atom && atom->type == node_atom);

        for (node* branch = atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            node* precondition = branch->first_child;
            plnnrc_assert(precondition);
            node* tasklist = precondition->next_sibling;
            plnnrc_assert(tasklist);
            link_branch_variables(atom, precondition, tasklist);
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
                link_to_parameter(param, effect_list);
            }
        }
    }
}

node* build_domain(tree& ast, sexpr::node* s_expr)
{
    plnnrc_assert(s_expr->type == sexpr::node_list);
    plnnrc_assert(s_expr->first_child);
    plnnrc_assert(s_expr->first_child->type == sexpr::node_symbol);
    plnnrc_assert(is_token(s_expr->first_child, token_domain));

    node* domain = ast.make_node(node_domain, s_expr);
    PLNNRC_CHECK(domain);

    PLNNRC_CHECK(ast.methods.init(count_elements(s_expr, token_method)));

    int num_operator_decls = count_elements(s_expr, token_operator);

    PLNNRC_CHECK(ast.operators.init(num_operator_decls > 0 ? num_operator_decls : 128));

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
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

        plnnrc_assert(element != 0);
        append_child(domain, element);
    }

    PLNNRC_CHECK(build_operator_stubs(ast));

    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        node* method = methods.value();
        node* method_atom = method->first_child;
        plnnrc_assert(method_atom && method_atom->type == node_atom);

        link_method_variables(method);
    }

    for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
    {
        node* operatr = operators.value();
        node* operator_atom = operatr->first_child;
        plnnrc_assert(operator_atom && operator_atom->type == node_atom);

        link_operator_variables(operatr);
    }

    return domain;
}

namespace
{
    node* build_method(tree& ast, sexpr::node* s_expr)
    {
        plnnrc_assert(s_expr->type == sexpr::node_list);
        plnnrc_assert(s_expr->first_child);
        plnnrc_assert(s_expr->first_child->type == sexpr::node_symbol);
        plnnrc_assert(is_token(s_expr->first_child, token_method));

        node* method = ast.make_node(node_method, s_expr);
        PLNNRC_CHECK(method);

        sexpr::node* task_atom_expr = s_expr->first_child->next_sibling;
        plnnrc_assert(task_atom_expr);
        plnnrc_assert(task_atom_expr->type == sexpr::node_list);

        node* task_atom = build_atom(ast, task_atom_expr);
        PLNNRC_CHECK(task_atom);

        append_child(method, task_atom);

        plnnrc_assert(is_valid_id(task_atom->s_expr->token));

        {
            bool result = ast.methods.insert(task_atom->s_expr->token, method);
            plnnrc_assert(result);
        }

        for (sexpr::node* branch_expr = task_atom_expr->next_sibling; branch_expr != 0; branch_expr = next_branch_expr(branch_expr))
        {
            node* branch = build_branch(ast, branch_expr);
            PLNNRC_CHECK(branch);
            append_child(method, branch);
        }

        return method;
    }

    sexpr::node* next_branch_expr(sexpr::node* branch_expr)
    {
        if (is_token(branch_expr->first_child, token_foreach))
        {
            return branch_expr->next_sibling;
        }

        return branch_expr->next_sibling->next_sibling;
    }

    node* build_branch(tree& ast, sexpr::node* s_expr)
    {
        node* branch = ast.make_node(node_branch, s_expr);
        PLNNRC_CHECK(branch);

        sexpr::node* precondition_expr = 0;
        sexpr::node* tasklist_expr = 0;

        branch_ann* ann = annotation<branch_ann>(branch);

        if (is_token(s_expr->first_child, token_foreach))
        {
            precondition_expr = s_expr->first_child->next_sibling;
            plnnrc_assert(precondition_expr);
            tasklist_expr = precondition_expr->next_sibling;
            ann->foreach = true;
        }
        else
        {
            precondition_expr = s_expr;
            tasklist_expr = precondition_expr->next_sibling;
            plnnrc_assert(precondition_expr);
            plnnrc_assert(tasklist_expr);
            ann->foreach = false;
        }

        node* precondition = build_logical_expression(ast, precondition_expr);
        PLNNRC_CHECK(precondition);
        node* precondition_dnf = convert_to_dnf(ast, precondition);
        PLNNRC_CHECK(precondition_dnf);
        append_child(branch, precondition_dnf);

        node* task_list = build_atom_list(ast, tasklist_expr);
        PLNNRC_CHECK(task_list);
        append_child(branch, task_list);

        return branch;
    }

    node* build_operator(tree& ast, sexpr::node* s_expr)
    {
        plnnrc_assert(s_expr->type == sexpr::node_list);
        plnnrc_assert(s_expr->first_child);
        plnnrc_assert(s_expr->first_child->type == sexpr::node_symbol);
        plnnrc_assert(is_token(s_expr->first_child, token_operator));

        node* operatr = ast.make_node(node_operator, s_expr);
        PLNNRC_CHECK(operatr);

        sexpr::node* task_atom_expr = s_expr->first_child->next_sibling;
        plnnrc_assert(task_atom_expr);
        plnnrc_assert(task_atom_expr->type == sexpr::node_list);

        plnnrc_assert(is_valid_id(task_atom_expr->first_child->token));

        node* task_atom = build_atom(ast, task_atom_expr);
        PLNNRC_CHECK(task_atom);
        append_child(operatr, task_atom);

        plnnrc_assert(is_valid_id(task_atom->s_expr->token));

        PLNNRC_CHECK(ast.operators.insert(task_atom->s_expr->token, operatr));

        sexpr::node* delete_effects_expr = 0;
        sexpr::node* add_effects_expr = 0;

        for (sexpr::node* child = task_atom_expr->next_sibling; child != 0; child = child->next_sibling)
        {
            plnnrc_assert(child->type == sexpr::node_list);

            if (is_token(child->first_child, token_delete))
            {
                plnnrc_assert(!delete_effects_expr);
                delete_effects_expr = child;
                continue;
            }

            if (is_token(child->first_child, token_add))
            {
                plnnrc_assert(!add_effects_expr);
                add_effects_expr = child;
                continue;
            }

            // error: unknown element in operator definition
            plnnrc_assert(false);
        }

        node* delete_effects = ast.make_node(node_atomlist, delete_effects_expr);
        PLNNRC_CHECK(delete_effects);
        append_child(operatr, delete_effects);

        if (delete_effects_expr)
        {
            for (sexpr::node* t_expr = delete_effects_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
            {
                node* atom = build_atom(ast, t_expr);
                PLNNRC_CHECK(atom);
                append_child(delete_effects, atom);
            }
        }

        node* add_effects = ast.make_node(node_atomlist, add_effects_expr);
        PLNNRC_CHECK(add_effects);
        append_child(operatr, add_effects);

        if (add_effects_expr)
        {
            for (sexpr::node* t_expr = add_effects_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
            {
                node* atom = build_atom(ast, t_expr);
                PLNNRC_CHECK(atom);
                append_child(add_effects, atom);
            }
        }

        return operatr;
    }

    node* build_operator_stub(tree& ast, sexpr::node* s_expr)
    {
        plnnrc_assert(is_valid_id(s_expr->token));

        node* operatr = ast.make_node(node_operator, s_expr->parent);
        PLNNRC_CHECK(operatr);

        PLNNRC_CHECK(ast.operators.insert(s_expr->token, operatr));

        node* operator_atom = build_atom(ast, s_expr->parent);
        PLNNRC_CHECK(operator_atom);
        append_child(operatr, operator_atom);

        node* delete_effects = ast.make_node(node_atomlist);
        PLNNRC_CHECK(delete_effects);
        append_child(operatr, delete_effects);

        node* add_effects = ast.make_node(node_atomlist);
        PLNNRC_CHECK(add_effects);
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
                plnnrc_assert(branch->first_child);
                node* tasklist = branch->first_child->next_sibling;
                plnnrc_assert(tasklist);

                for (node* task = tasklist->first_child; task != 0; task = task->next_sibling)
                {
                    if (is_operator(ast, task))
                    {
                        if (ast.operators.find(task->s_expr->token))
                        {
                            // check number of arguments here.
                            continue;
                        }

                        node* operatr = build_operator_stub(ast, task->s_expr);
                        PLNNRC_CHECK(operatr);
                    }
                }
            }
        }

        return true;
    }
}

node* build_worldstate(tree& ast, sexpr::node* s_expr)
{
    plnnrc_assert(s_expr->type == sexpr::node_list);
    plnnrc_assert(s_expr->first_child);
    plnnrc_assert(s_expr->first_child->type == sexpr::node_symbol);
    plnnrc_assert(is_token(s_expr->first_child, token_worldstate));

    node* worldstate = ast.make_node(node_worldstate, s_expr);
    PLNNRC_CHECK(worldstate);

    unsigned total_atom_count = 0;

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        total_atom_count++;
    }

    PLNNRC_CHECK(ast.ws_atoms.init(total_atom_count));
    PLNNRC_CHECK(ast.ws_types.init(32));

    int type_tag = 1;

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        node* atom = ast.make_node(node_atom, c_expr->first_child);
        PLNNRC_CHECK(atom);

        plnnrc_assert(is_valid_id(c_expr->first_child->token));

        ast.ws_atoms.insert(c_expr->first_child->token, atom);

        for (sexpr::node* t_expr = c_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
        {
            glue_tokens(t_expr);

            node* type = ast.make_node(node_worldstate_type, t_expr);
            PLNNRC_CHECK(type);

            node* type_proto = ast.ws_types.find(t_expr->first_child->token);

            if (!type_proto)
            {
                annotation<ws_type_ann>(type)->type_tag = type_tag;

                if (!ast.ws_types.insert(t_expr->first_child->token, type))
                {
                    return 0;
                }

                type_tag++;
            }
            else
            {
                annotation<ws_type_ann>(type)->type_tag = annotation<ws_type_ann>(type_proto)->type_tag;
            }

            append_child(atom, type);
        }

        append_child(worldstate, atom);
    }

    PLNNRC_CHECK(ast.type_tag_to_node.init(ast.ws_types.count() + 1));

    ast.type_tag_to_node[0] = 0;

    for (id_table_values types = ast.ws_types.values(); !types.empty(); types.pop())
    {
        node* ws_type = types.value();
        int type_tag = annotation<ws_type_ann>(ws_type)->type_tag;
        ast.type_tag_to_node[type_tag] = ws_type;
    }

    plnnrc_assert(!ast.type_tag_to_node[0]);

    return worldstate;
}

namespace
{
    void seed_types(tree& ast, node* root)
    {
        for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == node_atom)
            {
                node* ws_atom = ast.ws_atoms.find(n->s_expr->token);
                plnnrc_assert(ws_atom);

                node* ws_type = ws_atom->first_child;
                plnnrc_assert(ws_type->type == node_worldstate_type);

                for (node* c = n->first_child; c != 0; c = c->next_sibling)
                {
                    plnnrc_assert(ws_type);
                    plnnrc_assert(ws_type->type == node_worldstate_type);

                    if (c->type == node_term_variable)
                    {
                        type_tag(c, annotation<ws_type_ann>(ws_type)->type_tag);
                    }

                    ws_type = ws_type->next_sibling;
                }

                plnnrc_assert(!ws_type);
            }
        }
    }

    bool has_untyped_params(node* method_atom)
    {
        for (node* param = method_atom->first_child; param != 0; param = param->next_sibling)
        {
            if (!type_tag(param))
            {
                return true;
            }
        }

        return false;
    }
}

void infer_types(tree& ast)
{
    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        node* method = methods.value();
        node* method_atom = method->first_child;
        plnnrc_assert(method_atom && method_atom->type == node_atom);

        for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            seed_types(ast, branch->first_child);
        }
    }

    for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
    {
        node* operatr = operators.value();
        node* operator_atom = operatr->first_child;
        plnnrc_assert(operator_atom && operator_atom->type == node_atom);

        for (node* effect_list = operator_atom->next_sibling; effect_list != 0; effect_list = effect_list->next_sibling)
        {
            seed_types(ast, effect_list);
        }
    }

    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        node* method = methods.value();
        node* method_atom = method->first_child;
        plnnrc_assert(method_atom && method_atom->type == node_atom);

        for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            node* precondition = branch->first_child;
            plnnrc_assert(precondition);

            for (node* var = precondition; var != 0; var = preorder_traversal_next(precondition, var))
            {
                if (var->type == node_term_variable && is_bound(var) && var->parent->type == node_atom)
                {
                    node* def = definition(var);

                    if (is_parameter(def))
                    {
                        if (!type_tag(def))
                        {
                            type_tag(def, type_tag(var));
                        }

                        // check types match
                        plnnrc_assert(type_tag(def) == type_tag(var));
                    }
                }
            }
        }
    }

    bool all_method_param_types_inferred = false;

    while (!all_method_param_types_inferred)
    {
        all_method_param_types_inferred = true;

        int num_methods_processed = 0;

        for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
        {
            node* method = methods.value();
            node* method_atom = method->first_child;
            plnnrc_assert(method_atom && method_atom->type == node_atom);

            method_ann* ann = annotation<method_ann>(method);

            if (ann->processed)
            {
                continue;
            }

            if (has_untyped_params(method_atom))
            {
                all_method_param_types_inferred = false;
                continue;
            }

            for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                plnnrc_assert(branch->first_child);
                node* tasklist = branch->first_child->next_sibling;
                plnnrc_assert(tasklist);

                for (node* task = tasklist->first_child; task != 0; task = task->next_sibling)
                {
                    node* callee = ast.methods.find(task->s_expr->token);

                    if (!callee)
                    {
                        callee = ast.operators.find(task->s_expr->token);
                    }

                    plnnrc_assert(callee);

                    node* callee_atom = callee->first_child;
                    plnnrc_assert(callee_atom && callee_atom->type == node_atom);

                    node* param = callee_atom->first_child;

                    for (node* arg = task->first_child; arg != 0; arg = arg->next_sibling)
                    {
                        plnnrc_assert(param);
                        plnnrc_assert(arg->type == node_term_variable);
                        plnnrc_assert(is_bound(arg));
                        node* def = definition(arg);
                        plnnrc_assert(type_tag(def) != 0);

                        if (!type_tag(param))
                        {
                            type_tag(param, type_tag(def));
                        }

                        // check types match
                        plnnrc_assert(type_tag(param) == type_tag(def));

                        param = param->next_sibling;
                    }

                    plnnrc_assert(!param);
                }
            }

            ann->processed = true;
            num_methods_processed++;
        }

        if (num_methods_processed == 0)
        {
            // unable to infer some method parameters.
            plnnrc_assert(false);
        }

        // propagate types for non atom vars in preconditions
        for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
        {
            node* method = methods.value();
            node* method_atom = method->first_child;
            plnnrc_assert(method_atom && method_atom->type == node_atom);

            for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                node* precondition = branch->first_child;
                plnnrc_assert(precondition);

                for (node* var = precondition; var != 0; var = preorder_traversal_next(precondition, var))
                {
                    if (var->type == node_term_variable && is_bound(var) && var->parent->type != node_atom)
                    {
                        node* def = definition(var);
                        plnnrc_assert(type_tag(def));
                        type_tag(var, type_tag(def));
                    }
                }
            }
        }
    }
}

}
}
