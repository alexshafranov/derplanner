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

#include "formatter.h"
#include "ast_tools.h"
#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/tree_ops.h"
#include "derplanner/compiler/codegen.h"

namespace plnnrc {

using namespace ast;

bool generate_header(ast::tree& ast, writer& writer)
{
    formatter output(writer);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    output.writeln("#include <derplanner/runtime/runtime.h>");
    output.newline();

    return true;
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
        output.writeln("struct %i_tuple", atom->s_expr->token);
        {
            class_scope s(output);

            unsigned param_index = 0;

            for (node* param = atom->first_child; param != 0; param = param->next_sibling)
            {
                output.writeln("%s _%d;", param->s_expr->first_child->token, param_index++);
            }

            output.writeln("%i_tuple* next;", atom->s_expr->token);
            output.writeln("%i_tuple* prev;", atom->s_expr->token);
        }
    }

    output.writeln("struct worldstate");
    {
        class_scope s(output);

        for (node* atom = worldstate->first_child; atom != 0; atom = atom->next_sibling)
        {
            output.writeln("%i_tuple* %i;", atom->s_expr->token, atom->s_expr->token);
        }
    }

    return true;
}

namespace
{
    bool generate_precondition_state(tree& ast, node* root, unsigned branch_index, formatter& output)
    {
        output.writeln("struct p%d_state", branch_index);
        {
            class_scope s(output);

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

                        output.writeln("%s _%d;", ws_type->s_expr->first_child->token, var_index);

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

                    output.writeln("%i_tuple* %i_%d;", id, id, atom_index);

                    annotation<atom_ann>(n)->index = atom_index;

                    ++atom_index;
                }
            }

            output.writeln("int stage;");
        }

        return true;
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

        output.writeln("for (state.%i_%d = world.%i; state.%i_%d != 0; state.%i_%d = state.%i_%d->next)",
            atom_id, atom_index,
            atom_id,
            atom_id, atom_index,
            atom_id, atom_index,
            atom_id, atom_index);
        {
            scope s(output, is_first(root));

            int atom_param_index = 0;

            for (node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (term->type == node_term_variable && definition(term))
                {
                    int var_index = annotation<term_ann>(term)->var_index;

                    const char* comparison_op = "!=";

                    if (root->type == node_op_not)
                    {
                        comparison_op = "==";
                    }

                    output.writeln("if (state.%i_%d->_%d %s state._%d)", atom_id, atom_index, atom_param_index, comparison_op, var_index);
                    {
                        scope s(output);

                        output.writeln("continue;");
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
                    output.writeln("state._%d = state.%i_%d->_%d;", var_index, atom_id, atom_index, atom_param_index);
                    output.newline();
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
                output.writeln("PLNNR_COROUTINE_YIELD(state);");
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
        }

        return true;
    }

    bool generate_precondition_next(tree& ast, node* root, unsigned branch_index, formatter& output)
    {
        plnnrc_assert(is_logical_op(root));

        output.writeln("bool next(p%d_state& state, worldstate& world)", branch_index);
        {
            scope s(output);

            output.writeln("PLNNR_COROUTINE_BEGIN(state);");
            output.newline();

            if (!generate_precondition_satisfier(ast, root, output))
            {
                return false;
            }

            output.writeln("PLNNR_COROUTINE_END();");
        }

        return true;
    }

    bool generate_preconditions(tree& ast, node* domain, formatter& output)
    {
        unsigned branch_index = 0;

        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (method->type != node_method)
            {
                continue;
            }

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
        output.writeln("enum task_type");
        {
            class_scope s(output);

            output.writeln("task_none=0,");

            for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
            {
                node* operatr = operators.value();
                node* operator_atom = operatr->first_child;

                output.writeln("task_%i,", operator_atom->s_expr->token);
            }
        }

        return true;
    }

    bool generate_param_struct(tree& ast, node* task, formatter& output)
    {
        node* atom = task->first_child;

        if (!atom->first_child)
        {
            return true;
        }

        output.writeln("struct %i_args", atom->s_expr->token);
        {
            class_scope s(output);

            int param_index = 0;

            for (node* param = atom->first_child; param != 0; param = param->next_sibling)
            {
                node* ws_type = ast.type_tag_to_node[type_tag(param)];
                output.writeln("%s _%d;", ws_type->s_expr->first_child->token, param_index);
                annotation<term_ann>(param)->var_index = param_index;
                ++param_index;
            }
        }

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

    bool generate_forward_decls(tree& ast, node* domain, formatter& output)
    {
        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (method->type != node_method)
            {
                continue;
            }

            node* atom = method->first_child;
            const char* method_name = atom->s_expr->token;

            unsigned branch_index = 0;

            for (node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                plnnrc_assert(branch->type == node_branch);
                output.writeln("bool %i_branch_%d_expand(planner_state& pstate, void* world);", method_name, branch_index);
                ++branch_index;
            }
        }

        if (domain->first_child)
        {
            output.newline();
        }

        return true;
    }

    bool generate_operator_effects(tree& ast, node* method, node* task_atom, formatter& output)
    {
        node* operatr = ast.operators.find(task_atom->s_expr->token);
        plnnrc_assert(operatr);

        node* effects_remove = operatr->first_child->next_sibling;
        node* effects_add = effects_remove->next_sibling;
        plnnrc_assert(effects_remove && effects_add);

        if (effects_remove->first_child)
        {
            output.newline();

            for (node* effect = effects_remove->first_child; effect != 0; effect = effect->next_sibling)
            {
                const char* atom_id = effect->s_expr->token;

                output.writeln("for (%i_tuple* tuple = world.%i; tuple != 0; tuple = tuple->next)", atom_id, atom_id);
                {
                    scope s(output, effects_add->first_child);

                    int param_index = 0;

                    for (node* arg = effect->first_child; arg != 0; arg = arg->next_sibling)
                    {
                        node* def = definition(arg);
                        plnnrc_assert(def);
                        int var_index = annotation<term_ann>(def)->var_index;
                        plnnrc_assert(is_parameter(def));

                        output.writeln("if (tuple->_%d != a->_%d)", param_index, var_index);
                        {
                            scope s(output);
                            output.writeln("continue;");
                        }

                        ++param_index;
                    }

                    output.writeln("tuple_list::handle* list = tuple_list::head_to_handle<%i_tuple>(world.%i);", atom_id, atom_id);
                    output.writeln("operator_effect* effect = push<operator_effect>(pstate.journal);");
                    output.writeln("effect->tuple = tuple;");
                    output.writeln("effect->list = list;");
                    output.writeln("tuple_list::detach(list, tuple);");
                    output.newline();
                    output.writeln("break;");
                }
            }
        }

        if (effects_add->first_child)
        {
            if (!effects_remove->first_child)
            {
                output.newline();
            }

            for (node* effect = effects_add->first_child; effect != 0; effect = effect->next_sibling)
            {
                scope s(output, !is_last(effect));

                const char* atom_id = effect->s_expr->token;

                output.writeln("tuple_list::handle* list = tuple_list::head_to_handle<%i_tuple>(world.%i);", atom_id, atom_id);
                output.writeln("%i_tuple* tuple = tuple_list::append<%i_tuple>(list);", atom_id, atom_id);

                int param_index = 0;

                for (node* arg = effect->first_child; arg != 0; arg = arg->next_sibling)
                {
                    node* def = definition(arg);
                    plnnrc_assert(def);
                    int var_index = annotation<term_ann>(def)->var_index;
                    plnnrc_assert(is_parameter(def));

                    output.writeln("tuple->_%d = a->_%d;", param_index, var_index);
                    ++param_index;
                }

                output.writeln("operator_effect* effect = push<operator_effect>(pstate.journal);");
                output.writeln("effect->tuple = tuple;");
                output.writeln("effect->list = list;");
            }
        }

        return true;
    }

    bool generate_branch_expands(tree& ast, node* domain, formatter& output)
    {
        unsigned precondition_index = 0;

        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (method->type != node_method)
            {
                continue;
            }

            node* atom = method->first_child;
            const char* method_name = atom->s_expr->token;

            unsigned branch_index = 0;

            for (node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                plnnrc_assert(branch->type == node_branch);

                branch_ann* ann = annotation<branch_ann>(branch);

                node* precondition = branch->first_child;
                node* tasklist = precondition->next_sibling;

                plnnrc_assert(tasklist->type == node_atomlist);

                output.writeln("bool %i_branch_%d_expand(planner_state& pstate, void* world)", method_name, branch_index);
                {
                    scope s(output);

                    output.writeln("method_instance* method = pstate.top_method;");
                    output.writeln("p%d_state* precondition = static_cast<p%d_state*>(method->precondition);", precondition_index, precondition_index);
                    output.writeln("worldstate* wstate = static_cast<worldstate*>(world);");

                    if (has_parameters(method))
                    {
                        output.writeln("%i_args* method_args = static_cast<%i_args*>(method->args);", method_name, method_name);
                    }

                    output.newline();
                    output.writeln("PLNNR_COROUTINE_BEGIN(*method);");
                    output.newline();

                    output.writeln("precondition = push<p%d_state>(pstate.mstack);", precondition_index);
                    output.writeln("precondition->stage = 0;");

                    for (node* param = atom->first_child; param != 0; param = param->next_sibling)
                    {
                        node* var = first_parameter_usage(param, precondition);

                        if (var)
                        {
                            int param_index = annotation<term_ann>(param)->var_index;
                            int var_index = annotation<term_ann>(var)->var_index;

                            output.writeln("precondition->_%d = method_args->_%d;", var_index, param_index);
                        }
                    }

                    output.newline();
                    output.writeln("method->precondition = precondition;");
                    output.writeln("method->mrewind = pstate.mstack->top();");
                    output.writeln("method->trewind = pstate.tstack->top();");
                    output.writeln("method->jrewind = pstate.journal->top();");
                    output.newline();

                    output.writeln("while (next(*precondition, *wstate))");
                    {
                        scope s(output);

                        for (node* task_atom = tasklist->first_child; task_atom != 0; task_atom = task_atom->next_sibling)
                        {
                            {
                                scope s(output);

                                if (is_operator(ast, task_atom))
                                {
                                    output.writeln("task_instance* t = push_task(pstate, task_%i);", task_atom->s_expr->token);
                                    output.writeln("%i_args* a = push<%i_args>(pstate.tstack);", task_atom->s_expr->token, task_atom->s_expr->token);
                                }
                                else
                                {
                                    plnnrc_assert(ast.methods.find(task_atom->s_expr->token));
                                    output.writeln("method_instance* t = push_method(pstate, %i_branch_0_expand);", task_atom->s_expr->token);
                                    output.writeln("%i_args* a = push<%i_args>(pstate.mstack);", task_atom->s_expr->token, task_atom->s_expr->token);
                                }

                                int param_index = 0;

                                for (node* arg = task_atom->first_child; arg != 0; arg = arg->next_sibling)
                                {
                                    plnnrc_assert(arg->type == node_term_variable);
                                    node* def = definition(arg);
                                    plnnrc_assert(def);
                                    int var_index = annotation<term_ann>(def)->var_index;

                                    if (is_parameter(def))
                                    {
                                        output.writeln("a->_%d = method_args->_%d;", param_index, var_index);
                                    }
                                    else
                                    {
                                        output.writeln("a->_%d = precondition->_%d;", param_index, var_index);
                                    }

                                    ++param_index;
                                }

                                output.writeln("t->args = a;");

                                if (is_operator(ast, task_atom))
                                {
                                    if (!generate_operator_effects(ast, method, task_atom, output))
                                    {
                                        return false;
                                    }
                                }
                            }

                            if (is_last(task_atom) && !ann->foreach)
                            {
                                output.writeln("method->expanded = true;");
                            }

                            if (ast.methods.find(task_atom->s_expr->token) || is_last(task_atom))
                            {
                                output.writeln("PLNNR_COROUTINE_YIELD(*method);");

                                if (!is_last(task_atom))
                                {
                                    output.newline();
                                }
                            }
                        }
                    }

                    if (ann->foreach)
                    {
                        output.writeln("if (precondition->stage > 0)");
                        {
                            scope s(output);
                            output.writeln("method->expanded = true;");
                            output.writeln("PLNNR_COROUTINE_YIELD(*method);");
                        }
                    }

                    if (!is_last(branch))
                    {
                        output.writeln("return next_branch(pstate, %i_branch_%d_expand, world);", method_name, branch_index+1);
                    }

                    output.writeln("PLNNR_COROUTINE_END();");

                    ++branch_index;
                    ++precondition_index;
                }
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

    if (!generate_forward_decls(ast, domain, output))
    {
        return false;
    }

    if (!generate_branch_expands(ast, domain, output))
    {
        return false;
    }

    return true;
}

}
