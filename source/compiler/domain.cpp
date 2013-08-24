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
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/io.h"
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

                    node* operator_atom = ast.make_node(node_atom, task->first_child->s_expr);

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

                if (!ast.ws_types.insert(t_expr->first_child->token, type))
                {
                    return 0;
                }

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

    if (!ast.type_tag_to_node.init(ast.ws_types.count() + 1))
    {
        return 0;
    }

    ast.type_tag_to_node[0] = 0;

    for (id_table_values types = ast.ws_types.values(); !types.empty(); types.pop())
    {
        node* ws_type = types.value();
        int type_tag = annotation<worldstate_type>(ws_type)->type_tag;
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
                        type_tag(c, annotation<worldstate_type>(ws_type)->type_tag);
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

namespace
{
    void write(writer& output, const char* str)
    {
        output.write(str, strlen(str));
    }
}

bool generate_worldstate(tree& ast, node* worldstate, writer& output)
{
    plnnrc_assert(worldstate && worldstate->type == node_worldstate);

    char buffer[10];

    for (node* atom = worldstate->first_child; atom != 0; atom = atom->next_sibling)
    {
        write(output, "struct "); write(output, atom->s_expr->token); write(output, "_tuple");
        write(output, "\n");
        write(output, "{");
        write(output, "\n");

        unsigned param_index = 0;

        for (node* param = atom->first_child; param != 0; param = param->next_sibling)
        {
            write(output, "\t");
            write(output, param->s_expr->first_child->token);
            write(output, " _");
            sprintf(buffer, "%d", param_index);
            write(output, buffer);
            write(output, ";");
            write(output, "\n");
            ++param_index;
        }

        write(output, "\t");
        write(output, atom->s_expr->token); write(output, "_tuple* next;");
        write(output, "\n");

        write(output, "};");
        write(output, "\n\n");
    }

    write(output, "struct worldstate\n");
    write(output, "{\n");

    for (node* atom = worldstate->first_child; atom != 0; atom = atom->next_sibling)
    {
        write(output, "\t");
        write(output, atom->s_expr->token); write(output, "_tuple* "); write(output, atom->s_expr->token); write(output, ";\n");
    }

    write(output, "};\n\n");

    return true;
}

namespace
{
    bool generate_precondition_state(tree& ast, node* root, unsigned branch_index, writer& output)
    {
        char buffer[10];
        sprintf(buffer, "%d", branch_index);
        write(output, "struct p");
        write(output, buffer);
        write(output, "_state\n");
        write(output, "{\n");

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
                    write(output, "\t");
                    write(output, ws_type->s_expr->first_child->token);
                    write(output, " ");
                    write(output, "_");
                    sprintf(buffer, "%d", var_index);
                    write(output, buffer);
                    write(output, ";\n");

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
                write(output, "\t");
                write(output, id);
                write(output, "_tuple* ");
                write(output, id);
                sprintf(buffer, "%d", atom_index);
                write(output, "_");
                write(output, buffer);
                write(output, ";\n");

                annotation<atom>(n)->index = atom_index;

                ++atom_index;
            }
        }

        write(output, "\tint stage;\n");
        write(output, "};\n\n");

        return true;
    }

    void indent(writer& output, int level)
    {
        for (int i = 0; i < level; ++i)
        {
            write(output, "\t");
        }
    }

    bool generate_literal(tree& ast, node* root, writer& output, int indent_level)
    {
        plnnrc_assert(root->type == node_op_not || root->type == node_atom);

        node* atom = root;
        bool negative = false;

        if (root->type == node_op_not)
        {
            atom = root->first_child;
            negative = true;
        }

        // const char* atom_id = atom->s_expr->token;
        // int atom_index = annotation<atom>(atom)->index;

        // char buffer[10];
        // sprintf(buffer, "%d", atom_index);

        // indent(output, indent_level);
        // write(output, "for (state.");
        // write(output, atom_id);
        // write(output, "_");
        // write(output, buffer);
        // write(output, " = world.");
        // write(output, atom_id);
        // write(output, "; state.");
        // write(output, atom_id);
        // write(output, "_");
        // write(output, buffer);
        // write(output, " != 0; ");
        // write(output, "state.");
        // write(output, atom_id);
        // write(output, "_");
        // write(output, buffer);
        // write(output, " = ");
        // write(output, "state.");
        // write(output, atom_id);
        // write(output, "_");
        // write(output, buffer);
        // write(output, "->next)\n");
        // indent(output, indent_level);
        // write(output, "{\n");

        for (node* term = atom->first_child; term != 0; term = term->next_sibling)
        {
            if (term->type == node_term_variable)
            {
                (void)negative;
            }
        }

        return true;
    }

    bool generate_conjunctive_clause(tree& ast, node* root, writer& output, int indent_level)
    {
        plnnrc_assert(root->type == node_op_and);

        int child_indent_level = indent_level;

        for (node* child = root->first_child; child != 0; child = child->next_sibling)
        {
            if (!generate_literal(ast, child, output, child_indent_level))
            {
                return false;
            }

            ++child_indent_level;
        }

        for (int i = child_indent_level-1; i >= indent_level; --i)
        {
            indent(output, i);
            write(output, "}\n");
        }

        return true;
    }

    bool generate_precondition_satisfier(tree& ast, node* root, writer& output)
    {
        plnnrc_assert(root->type == node_op_or);

        int indent_level = 0;

        for (node* child = root->first_child; child != 0; child = child->next_sibling)
        {
            switch (child->type)
            {
            case node_op_and:
                if (!generate_conjunctive_clause(ast, child, output, indent_level+1))
                {
                    return false;
                }

                if (child != root->first_child->prev_sibling_cyclic)
                {
                    write(output, "\n");
                }

                break;
            case node_atom:
            case node_op_not:
                if (!generate_literal(ast, child, output, indent_level+1))
                {
                    return false;
                }
                break;
            default:
                plnnrc_assert(false);
                return false;
            }
        }

        write(output, "\n");

        return true;
    }

    bool generate_precondition_next(tree& ast, node* root, unsigned branch_index, writer& output)
    {
        plnnrc_assert(is_logical_op(root));

        char buffer[10];
        sprintf(buffer, "%d", branch_index);

        write(output, "bool next(");
        write(output, "p");
        write(output, buffer);
        write(output, "_state& state, worldstate& world)\n{\n");
        write(output, "\tPLNNRC_COROUTINE_BEGIN(state);\n\n");

        if (!generate_precondition_satisfier(ast, root, output))
        {
            return false;
        }

        write(output, "\tPLNNRC_COROUTINE_END();\n}\n\n");
        return true;
    }
}

bool generate_domain(tree& ast, node* domain, writer& output)
{
    plnnrc_assert(domain && domain->type == node_domain);

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

}
}
