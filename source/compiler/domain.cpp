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
#include <stdio.h>
#include "formatter.h"
#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/io.h"
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

    inline bool is_operator(tree& ast, node* atom)
    {
        plnnrc_assert(atom->type == node_atom);
        return !ast.methods.find(atom->s_expr->token);
    }

    node* build_method(tree& ast, sexpr::node* s_expr);
    node* build_branch(tree& ast, sexpr::node* s_expr);
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

    if (!ast.operators.init(32))
    {
        return 0;
    }

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
                    node* operatr = ast.make_node(node_operator);

                    if (!operatr)
                    {
                        return 0;
                    }

                    node* operator_atom = ast.make_node(node_atom, task->s_expr);

                    if (!operator_atom)
                    {
                        return 0;
                    }

                    append_child(operatr, operator_atom);

                    if (ast.operators.find(task->s_expr->token))
                    {
                        // check number of arguments here.
                        continue;
                    }

                    plnnrc_assert(is_valid_id(task->s_expr->token));

                    if (!ast.operators.insert(task->s_expr->token, operatr))
                    {
                        return 0;
                    }

                    for (node* arg = task->first_child; arg != 0; arg = arg->next_sibling)
                    {
                        node* param = ast.make_node(node_term_variable);

                        if (!param)
                        {
                            return 0;
                        }

                        append_child(operator_atom, param);
                    }
                }
            }
        }
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

        plnnrc_assert(is_valid_id(task_atom->s_expr->token));

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

    for (sexpr::node* c_expr = s_expr->first_child->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        total_atom_count++;
    }

    if (!ast.ws_atoms.init(total_atom_count))
    {
        return 0;
    }

    if (!ast.ws_types.init(32))
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

        plnnrc_assert(is_valid_id(c_expr->first_child->token));

        ast.ws_atoms.insert(c_expr->first_child->token, atom);

        for (sexpr::node* t_expr = c_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
        {
            glue_tokens(t_expr);

            node* type = ast.make_node(node_worldstate_type, t_expr);

            if (!type)
            {
                return 0;
            }

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

    if (!ast.type_tag_to_node.init(ast.ws_types.count() + 1))
    {
        return 0;
    }

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
        return var->parent->parent->type == node_method;
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

    void seed_precondition_types(tree& ast, node* root)
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

    void link_variables(node* method)
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
            seed_precondition_types(ast, branch->first_child);
        }

        link_variables(method);
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
                if (var->type == node_term_variable && is_bound(var))
                {
                    node* def = definition(var);

                    if (is_parameter(def))
                    {
                        if (!type_tag(def))
                        {
                            type_tag(def, type_tag(var));
                        }

                        plnnrc_assert(type_tag(def) == type_tag(var));
                    }
                }
            }
        }
    }

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

                    plnnrc_assert(type_tag(param) == type_tag(def));

                    param = param->next_sibling;
                }

                plnnrc_assert(!param);
            }
        }
    }
}

bool generate_worldstate(tree& ast, node* worldstate, writer& writer)
{
    plnnrc_assert(worldstate && worldstate->type == node_worldstate);

    formatter output(writer);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    for (node* atom = worldstate->first_child; atom != 0; atom = atom->next_sibling)
    {
        output.write("struct %i_tuple\n{\n", atom->s_expr->token);

        unsigned param_index = 0;

        for (node* param = atom->first_child; param != 0; param = param->next_sibling)
        {
            output.write("\t%s _%d;\n", param->s_expr->first_child->token, param_index++);
        }

        output.write("\t%i_tuple* next;\n};\n\n", atom->s_expr->token);
    }

    output.write("struct worldstate\n{\n");

    for (node* atom = worldstate->first_child; atom != 0; atom = atom->next_sibling)
    {
        output.write("\t%i_tuple* %i;\n", atom->s_expr->token, atom->s_expr->token);
    }

    output.write("};\n\n");

    return true;
}

namespace
{
    bool generate_precondition_state(tree& ast, node* root, unsigned branch_index, formatter& output)
    {
        output.write("struct p%d_state\n{\n", branch_index);

        for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == node_term_variable)
            {
                node* def = definition(n);

                if (def)
                {
                    annotation<term_ann>(def)->var_index = -1;
                }
            }
        }

        int var_index = 0;

        for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == node_term_variable)
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

                    node* ws_type = ast.type_tag_to_node[type_tag(n)];

                    output.write("\t%s _%d;\n", ws_type->s_expr->first_child->token, var_index);

                    ++var_index;
                }
                else
                {
                    annotation<term_ann>(n)->var_index = annotation<term_ann>(def)->var_index;
                }
            }
        }

        int atom_index = 0;

        for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == node_atom)
            {
                const char* id = n->s_expr->token;

                output.write("\t%i_tuple* %i_%d;\n", id, id, atom_index);

                annotation<atom_ann>(n)->index = atom_index;

                ++atom_index;
            }
        }

        output.write("\tint stage;\n};\n\n");

        return true;
    }

    void indent(formatter& output, int level)
    {
        for (int i = 0; i < level; ++i)
        {
            output.write("\t");
        }
    }

    bool generate_literal_chain(tree& ast, node* root, formatter& output)
    {
        plnnrc_assert(root->type == node_op_not || root->type == node_atom);

        node* atom = root;

        if (root->type == node_op_not)
        {
            atom = root->first_child;
        }

        const char* atom_id = atom->s_expr->token;
        int atom_index = annotation<atom_ann>(atom)->index;

        output.write("for (state.%i_%d = world.%i; state.%i_%d != 0; state.%i_%d = state.%i_%d->next)\n",
            atom_id, atom_index,
            atom_id,
            atom_id, atom_index,
            atom_id, atom_index,
            atom_id, atom_index);

        scope s(output);
        {
            int atom_param_index = 0;

            for (node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (term->type == node_term_variable && definition(term))
                {
                    int var_index = annotation<term_ann>(term)->var_index;
                    output.write("if (state.%i_%d->_%d != state._%d)\n", atom_id, atom_index, atom_param_index, var_index);

                    scope s(output, true);
                    {
                        output.write("continue;\n");
                    }
                }

                ++atom_param_index;
            }

            atom_param_index = 0;

            for (node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (term->type == node_term_variable && !definition(term))
                {
                    int var_index = annotation<term_ann>(term)->var_index;
                    output.write("state._%d = state.%i_%d->_%d;\n\n", var_index, atom_id, atom_index, atom_param_index);
                }

                ++atom_param_index;
            }

            if (root->next_sibling)
            {
                if (!generate_literal_chain(ast, root->next_sibling, output))
                {
                    return false;
                }
            }
            else
            {
                output.write("PLNNR_COROUTINE_YIELD(state);\n");
            }
        }

        return true;
    }

    bool generate_conjunctive_clause(tree& ast, node* root, formatter& output)
    {
        plnnrc_assert(root->type == node_op_and);

        if (root->first_child)
        {
            if (!generate_literal_chain(ast, root->first_child, output))
            {
                return false;
            }
        }

        return true;
    }

    bool generate_precondition_satisfier(tree& ast, node* root, formatter& output)
    {
        plnnrc_assert(root->type == node_op_or);

        for (node* child = root->first_child; child != 0; child = child->next_sibling)
        {
            plnnrc_assert(child->type == node_op_and);

            if (!generate_conjunctive_clause(ast, child, output))
            {
                return false;
            }

            if (!is_last(child))
            {
                output.write("\n");
            }
        }

        output.write("\n");

        return true;
    }

    bool generate_precondition_next(tree& ast, node* root, unsigned branch_index, formatter& output)
    {
        plnnrc_assert(is_logical_op(root));

        output.write("bool next(p%d_state& state, worldstate& world)\n", branch_index);

        scope s(output, true);
        {
            output.write("PLNNR_COROUTINE_BEGIN(state);\n\n");

            if (!generate_precondition_satisfier(ast, root, output))
            {
                return false;
            }

            output.write("PLNNR_COROUTINE_END();\n");
        }

        return true;
    }

    bool generate_preconditions(tree& ast, node* domain, formatter& output)
    {
        unsigned branch_index = 0;

        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            plnnrc_assert(method->type == node_method);

            for (node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                plnnrc_assert(branch->type == node_branch);

                node* precondition = branch->first_child;

                if (!generate_precondition_state(ast, precondition, branch_index, output))
                {
                    return false;
                }

                if (!generate_precondition_next(ast, precondition, branch_index, output))
                {
                    return false;
                }

                ++branch_index;
            }
        }

        return true;
    }

    bool generate_operators_enum(tree& ast, formatter& output)
    {
        output.write("enum task_type\n{\n");
        output.write("\ttask_none=0,\n");

        for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
        {
            node* operatr = operators.value();
            node* operator_atom = operatr->first_child;

            output.write("\ttask_%i,\n", operator_atom->s_expr->token);
        }

        output.write("};\n\n");

        return true;
    }

    bool generate_param_struct(tree& ast, node* task, formatter& output)
    {
        node* atom = task->first_child;

        if (!atom->first_child)
        {
            return true;
        }

        output.write("struct %i_args\n{\n", atom->s_expr->token);

        int param_index = 0;

        for (node* param = atom->first_child; param != 0; param = param->next_sibling)
        {
            node* ws_type = ast.type_tag_to_node[type_tag(param)];
            output.write("\t%s _%d;\n", ws_type->s_expr->first_child->token, param_index);
            annotation<term_ann>(param)->var_index = param_index;
            ++param_index;
        }

        output.write("};\n\n");
        return true;
    }

    bool generate_param_structs(tree& ast, formatter& output)
    {
        for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
        {
            if (!generate_param_struct(ast, operators.value(), output))
            {
                return false;
            }
        }

        for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
        {
            if (!generate_param_struct(ast, methods.value(), output))
            {
                return false;
            }
        }

        return true;
    }

    bool generate_methods(tree& ast, node* domain, formatter& output)
    {
        unsigned precondition_index = 0;

        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            plnnrc_assert(method->type == node_method);

            node* atom = method->first_child;
            const char* method_name = atom->s_expr->token;

            unsigned branch_index = 0;

            for (node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                plnnrc_assert(branch->type == node_branch);

                node* precondition = branch->first_child;
                node* tasklist = precondition->next_sibling;

                plnnrc_assert(tasklist->type == node_tasklist);

                output.write("bool %i_branch_%d_expand(planner_state& pstate, void* world)\n{\n", method_name, branch_index);
                output.write("\tmethod_instance* method = pstate.top_method;\n");
                output.write("\tp%d_state* precondition = static_cast<p%d_state*>(method->precondition);\n", precondition_index, precondition_index);
                output.write("\tworldstate* wstate = static_cast<worldstate*>(world);\n");

                if (has_parameters(method))
                {
                    output.write("\t%i_args* method_args = static_cast<%i_args*>(method->args);\n", method_name, method_name);
                }

                output.write("\n\tPLNNR_COROUTINE_BEGIN(*method);\n\n");

                output.write("\tprecondition = push<p%d_state>(pstate.mstack);\n", precondition_index);
                output.write("\tprecondition->stage = 0;\n");

                for (node* param = atom->first_child; param != 0; param = param->next_sibling)
                {
                    node* var = first_parameter_usage(param, precondition);

                    if (var)
                    {
                        int param_index = annotation<term_ann>(param)->var_index;
                        int var_index = annotation<term_ann>(var)->var_index;

                        output.write("\tprecondition->_%d = method_args->_%d;\n", var_index, param_index);
                    }
                }

                output.write("\n");
                output.write("\tmethod->precondition = precondition;\n");
                output.write("\tmethod->mrewind = pstate.mstack->top();\n");
                output.write("\tmethod->trewind = pstate.tstack->top();\n\n");

                output.write("\twhile (next(*precondition, *wstate))\n\t{\n");

                for (node* task_atom = tasklist->first_child; task_atom != 0; task_atom = task_atom->next_sibling)
                {
                    output.write("\t\t{\n");

                    if (is_operator(ast, task_atom))
                    {
                        output.write("\t\t\ttask_instance* t = push_task(pstate, task_%i);\n", task_atom->s_expr->token);
                        output.write("\t\t\t%i_args* a = push<%i_args>(pstate.tstack);\n", task_atom->s_expr->token, task_atom->s_expr->token);
                    }
                    else
                    {
                        plnnrc_assert(ast.methods.find(task_atom->s_expr->token));
                        output.write("\t\t\tmethod_instance* t = push_method(pstate, %i_branch_0_expand);\n", task_atom->s_expr->token);
                        output.write("\t\t\t%i_args* a = push<%i_args>(pstate.mstack);\n", task_atom->s_expr->token, task_atom->s_expr->token);
                    }

                    int param_index = 0;

                    for (node* arg = task_atom->first_child; arg != 0; arg = arg->next_sibling)
                    {
                        plnnrc_assert(arg->type == node_term_variable);
                        node* def = definition(arg);
                        plnnrc_assert(def);

                        output.write("\t\t\ta->_%d = ", param_index);

                        if (is_parameter(def))
                        {
                            output.write("method_args->");
                        }
                        else
                        {
                            output.write("precondition->");
                        }

                        int var_index = annotation<term_ann>(def)->var_index;
                        output.write("_%d;\n", var_index);

                        ++param_index;
                    }

                    output.write("\t\t\tt->args = a;\n");
                    output.write("\t\t}\n");

                    if (is_last(task_atom))
                    {
                        output.write("\t\tmethod->expanded = true;\n");
                    }

                    if (ast.methods.find(task_atom->s_expr->token) || is_last(task_atom))
                    {
                        output.write("\t\tPLNNR_COROUTINE_YIELD(*method);\n");
                    }
                }

                output.write("\t}\n\n");

                if (!is_last(branch))
                {
                    output.write("\treturn next_branch(pstate, %i_branch_%d_expand, world);\n", method_name, branch_index+1);
                }

                output.write("\tPLNNR_COROUTINE_END();\n}\n\n");

                ++branch_index;
                ++precondition_index;
            }
        }

        return true;
    }
}

bool generate_domain(tree& ast, node* domain, writer& writer)
{
    plnnrc_assert(domain && domain->type == node_domain);

    formatter output(writer);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    if (!generate_preconditions(ast, domain, output))
    {
        return false;
    }

    if (!generate_operators_enum(ast, output))
    {
        return false;
    }

    if (!generate_param_structs(ast, output))
    {
        return false;
    }

    if (!generate_methods(ast, domain, output))
    {
        return false;
    }

    return true;
}

}
}
