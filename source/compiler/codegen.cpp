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

namespace
{
    void generate_includes(ast::tree& ast, const char* header_file_name, formatter& output)
    {
        output.writeln("#include <derplanner/runtime/runtime.h>");
        output.writeln("#include \"%s\"", header_file_name);
        output.newline();

        output.writeln("using namespace plnnr;");
        output.newline();
    }

    void generate_worldstate(tree& ast, node* worldstate, formatter& output)
    {
        plnnrc_assert(worldstate && worldstate->type == node_worldstate);

        output.writeln("namespace plnnr");
        {
            scope s(output);
            output.writeln("namespace tuple_list");
            {
                scope s(output, false);
                output.writeln("struct handle;");
            }
        }

        output.writeln("struct worldstate");
        {
            class_scope s(output);

            for (node* atom = worldstate->first_child; atom != 0; atom = atom->next_sibling)
            {
                output.writeln("plnnr::tuple_list::handle* %i;", atom->s_expr->token);
            }
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
                output.writeln("enum { worldstate_offset = offsetof(worldstate, %i) };", atom->s_expr->token);
            }
        }
    }

    void generate_precondition_state(tree& ast, node* root, unsigned branch_index, formatter& output)
    {
        output.writeln("// method %s [%d:%d]", root->parent->parent->first_child->s_expr->token, root->s_expr->line, root->s_expr->column);

        output.writeln("struct p%d_state", branch_index);
        {
            class_scope s(output);

            int last_var_index = -1;

            for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
            {
                if (n->type == node_term_variable)
                {
                    int var_index = annotation<term_ann>(n)->var_index;

                    if (var_index > last_var_index)
                    {
                        node* ws_type = ast.type_tag_to_node[type_tag(n)];
                        output.writeln("// %s [%d:%d]", n->s_expr->token, n->s_expr->line, n->s_expr->column);
                        output.writeln("%s _%d;", ws_type->s_expr->first_child->token, var_index);
                        last_var_index = var_index;
                    }
                }
            }

            for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
            {
                if (n->type == node_atom)
                {
                    const char* id = n->s_expr->token;
                    output.writeln("%i_tuple* %i_%d;", id, id, annotation<atom_ann>(n)->index);
                }
            }

            output.writeln("int stage;");
        }
    }

    void generate_literal_chain(tree& ast, node* root, formatter& output);

    void generate_literal_chain_atom_eq(tree& ast, node* root, node* atom, formatter& output)
    {
        node* arg_0 = atom->first_child;
        plnnrc_assert(arg_0 && arg_0->type == node_term_variable);
        node* arg_1 = arg_0->next_sibling;
        plnnrc_assert(arg_1 && arg_1->type == node_term_variable && !arg_1->next_sibling);

        node* def_0 = definition(arg_0);
        node* def_1 = definition(arg_1);
        plnnrc_assert(def_0 && def_1);

        const char* comparison_op = "==";

        if (root->type == node_op_not)
        {
            comparison_op = "!=";
        }

        int var_index_0 = annotation<term_ann>(arg_0)->var_index;
        int var_index_1 = annotation<term_ann>(arg_1)->var_index;

        output.writeln("if (state._%d %s state._%d)", var_index_0, comparison_op, var_index_1);
        {
            scope s(output, root->next_sibling);

            if (root->next_sibling)
            {
                generate_literal_chain(ast, root->next_sibling, output);
            }
            else
            {
                output.writeln("PLNNR_COROUTINE_YIELD(state);");
            }
        }
    }

    void generate_literal_chain(tree& ast, node* root, formatter& output)
    {
        plnnrc_assert(root->type == node_op_not || is_atom(root));

        node* atom = root;

        if (root->type == node_op_not)
        {
            atom = root->first_child;
        }

        // special case atoms
        if (atom->type == node_atom_eq)
        {
            generate_literal_chain_atom_eq(ast, root, atom, output);
            return;
        }

        const char* atom_id = atom->s_expr->token;
        int atom_index = annotation<atom_ann>(atom)->index;

        if (root->type == node_op_not && all_bound(atom))
        {
            output.writeln("for (state.%i_%d = tuple_list::head<%i_tuple>(world.%i); state.%i_%d != 0; state.%i_%d = state.%i_%d->next)",
                atom_id, atom_index,
                atom_id,
                atom_id,
                atom_id, atom_index,
                atom_id, atom_index,
                atom_id, atom_index);
            {
                scope s(output);

                int atom_param_index = 0;

                for (node* term = atom->first_child; term != 0; term = term->next_sibling)
                {
                    if (term->type == node_term_variable)
                    {
                        int var_index = annotation<term_ann>(term)->var_index;

                        output.writeln("if (state.%i_%d->_%d == state._%d)", atom_id, atom_index, atom_param_index, var_index);
                        {
                            scope s(output, !is_last(term));
                            output.writeln("break;");
                        }
                    }

                    ++atom_param_index;
                }
            }

            output.writeln("if (state.%i_%d == 0)", atom_id, atom_index);
            {
                scope s(output, is_first(root));

                if (root->next_sibling)
                {
                    generate_literal_chain(ast, root->next_sibling, output);
                }
                else
                {
                    output.writeln("PLNNR_COROUTINE_YIELD(state);");
                }
            }
        }
        else
        {
            output.writeln("for (state.%i_%d = tuple_list::head<%i_tuple>(world.%i); state.%i_%d != 0; state.%i_%d = state.%i_%d->next)",
                atom_id, atom_index,
                atom_id,
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
                    generate_literal_chain(ast, root->next_sibling, output);
                }
                else
                {
                    output.writeln("PLNNR_COROUTINE_YIELD(state);");
                }
            }
        }
    }

    void generate_conjunctive_clause(tree& ast, node* root, formatter& output)
    {
        plnnrc_assert(root->type == node_op_and);

        if (root->first_child)
        {
            generate_literal_chain(ast, root->first_child, output);
        }
        else
        {
            // the formula is trivial: (or (and))
            output.writeln("PLNNR_COROUTINE_YIELD(state);");
            output.newline();
        }
    }

    void generate_precondition_satisfier(tree& ast, node* root, formatter& output)
    {
        plnnrc_assert(root->type == node_op_or);

        for (node* child = root->first_child; child != 0; child = child->next_sibling)
        {
            plnnrc_assert(child->type == node_op_and);

            generate_conjunctive_clause(ast, child, output);
        }
    }

    void generate_precondition_next(tree& ast, node* root, unsigned branch_index, formatter& output)
    {
        plnnrc_assert(is_logical_op(root));

        output.writeln("bool next(p%d_state& state, worldstate& world)", branch_index);
        {
            scope s(output);

            output.writeln("PLNNR_COROUTINE_BEGIN(state);");
            output.newline();

            generate_precondition_satisfier(ast, root, output);

            output.writeln("PLNNR_COROUTINE_END();");
        }
    }

    void generate_preconditions(tree& ast, node* domain, formatter& output)
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

                generate_precondition_state(ast, precondition, branch_index, output);
                generate_precondition_next(ast, precondition, branch_index, output);

                ++branch_index;
            }
        }
    }

    void generate_task_type_enum(tree& ast, node* domain, formatter& output)
    {
        output.writeln("enum task_type");
        {
            class_scope s(output);

            output.writeln("task_none=0,");

            for (node* method = domain->first_child; method != 0; method = method->next_sibling)
            {
                if (method->type != node_method)
                {
                    continue;
                }

                node* method_atom = method->first_child;
                plnnrc_assert(method_atom);

                output.writeln("task_%i,", method_atom->s_expr->token);
            }

            for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
            {
                node* operatr = operators.value();
                node* operator_atom = operatr->first_child;

                output.writeln("task_%i,", operator_atom->s_expr->token);
            }
        }

        output.writeln("const char* task_name(task_type type);");
        output.newline();
    }

    void generate_param_struct(tree& ast, node* task, formatter& output)
    {
        node* atom = task->first_child;

        if (!atom->first_child)
        {
            return;
        }

        output.writeln("struct %i_args", atom->s_expr->token);
        {
            class_scope s(output);

            for (node* param = atom->first_child; param != 0; param = param->next_sibling)
            {
                node* ws_type = ast.type_tag_to_node[type_tag(param)];
                output.writeln("%s _%d;", ws_type->s_expr->first_child->token, annotation<term_ann>(param)->var_index);
            }
        }
    }

    void generate_param_structs(tree& ast, node* domain, formatter& output)
    {
        for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
        {
            generate_param_struct(ast, operators.value(), output);
        }

        for (node* method = domain->first_child; method != 0; method = method->next_sibling)
        {
            if (method->type != node_method)
            {
                continue;
            }

            generate_param_struct(ast, method, output);
        }
    }

    void generate_forward_decls(tree& ast, node* domain, formatter& output)
    {
        output.writeln("namespace plnnr");
        {
            scope s(output);
            output.writeln("struct planner_state;");
        }

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
                output.writeln("bool %i_branch_%d_expand(plnnr::planner_state& pstate, void* world);", method_name, branch_index);
                ++branch_index;
            }
        }

        if (ast.methods.count() > 0)
        {
            output.newline();
        }
    }

    void generate_effects_delete(node* effects, formatter& output)
    {
        for (node* effect = effects->first_child; effect != 0; effect = effect->next_sibling)
        {
            const char* atom_id = effect->s_expr->token;

            output.writeln("for (%i_tuple* tuple = tuple_list::head<%i_tuple>(wstate->%i); tuple != 0; tuple = tuple->next)", atom_id, atom_id, atom_id);
            {
                scope s(output, !is_last(effect));

                int param_index = 0;

                for (node* arg = effect->first_child; arg != 0; arg = arg->next_sibling)
                {
                    node* def = definition(arg);
                    plnnrc_assert(def);
                    int var_index = annotation<term_ann>(def)->var_index;

                    if (is_operator_parameter(def))
                    {
                        output.writeln("if (tuple->_%d != a->_%d)", param_index, var_index);
                    }
                    else if (is_method_parameter(def))
                    {
                        output.writeln("if (tuple->_%d != method_args->_%d)", param_index, var_index);
                    }
                    else
                    {
                        output.writeln("if (tuple->_%d != precondition->_%d)", param_index, var_index);
                    }

                    {
                        scope s(output);
                        output.writeln("continue;");
                    }

                    ++param_index;
                }

                output.writeln("tuple_list::handle* list = wstate->%i;", atom_id, atom_id);
                output.writeln("operator_effect* effect = push<operator_effect>(pstate.journal);");
                output.writeln("effect->tuple = tuple;");
                output.writeln("effect->list = list;");
                output.writeln("tuple_list::detach(list, tuple);");
                output.newline();
                output.writeln("break;");
            }
        }
    }

    void generate_effects_add(node* effects, formatter& output)
    {
        for (node* effect = effects->first_child; effect != 0; effect = effect->next_sibling)
        {
            scope s(output, !is_last(effect));

            const char* atom_id = effect->s_expr->token;

            output.writeln("tuple_list::handle* list = wstate->%i;", atom_id, atom_id);
            output.writeln("%i_tuple* tuple = tuple_list::append<%i_tuple>(list);", atom_id, atom_id);

            int param_index = 0;

            for (node* arg = effect->first_child; arg != 0; arg = arg->next_sibling)
            {
                node* def = definition(arg);
                plnnrc_assert(def);
                int var_index = annotation<term_ann>(def)->var_index;

                if (is_operator_parameter(def))
                {
                    output.writeln("tuple->_%d = a->_%d;", param_index, var_index);
                }
                else if (is_method_parameter(def))
                {
                    output.writeln("tuple->_%d = method_args->_%d;", param_index, var_index);
                }
                else
                {
                    output.writeln("tuple->_%d = precondition->_%d;", param_index, var_index);
                }

                ++param_index;
            }

            output.writeln("operator_effect* effect = push<operator_effect>(pstate.journal);");
            output.writeln("effect->tuple = tuple;");
            output.writeln("effect->list = list;");
        }
    }

    void generate_operator_effects(tree& ast, node* method, node* task_atom, formatter& output)
    {
        node* operatr = ast.operators.find(task_atom->s_expr->token);
        plnnrc_assert(operatr);

        node* effects_delete = operatr->first_child->next_sibling;
        node* effects_add = effects_delete->next_sibling;
        plnnrc_assert(effects_delete && effects_add);

        if (effects_delete->first_child)
        {
            output.newline();

            generate_effects_delete(effects_delete, output);

            if (effects_add->first_child)
            {
                output.newline();
            }
        }

        if (effects_add->first_child)
        {
            if (!effects_delete->first_child)
            {
                output.newline();
            }

            generate_effects_add(effects_add, output);
        }
    }

    void generate_operator_task(tree& ast, node* method, node* task_atom, formatter& output)
    {
        plnnrc_assert(is_operator(ast, task_atom));

        output.writeln("task_instance* t = push_task(pstate, task_%i);", task_atom->s_expr->token);

        if (task_atom->first_child)
        {
            output.writeln("%i_args* a = push<%i_args>(pstate.tstack);", task_atom->s_expr->token, task_atom->s_expr->token);
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

        if (task_atom->first_child)
        {
            output.writeln("t->args = a;");
        }

        generate_operator_effects(ast, method, task_atom, output);
    }

    void generate_method_task(tree& ast, node* method, node* task_atom, formatter& output)
    {
        plnnrc_assert(is_method(ast, task_atom));

        output.writeln("method_instance* t = push_method(pstate, %i_branch_0_expand);", task_atom->s_expr->token);

        if (task_atom->first_child)
        {
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

        if (task_atom->first_child)
        {
            output.writeln("t->args = a;");
        }
    }

    void generate_branch_expands(tree& ast, node* domain, formatter& output)
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

                plnnrc_assert(tasklist->type == node_task_list);

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

                        if (!tasklist->first_child)
                        {
                            if (!ann->foreach)
                            {
                                output.writeln("method->expanded = true;");
                            }

                            output.writeln("PLNNR_COROUTINE_YIELD(*method);");
                        }

                        for (node* task_atom = tasklist->first_child; task_atom != 0; task_atom = task_atom->next_sibling)
                        {
                            {
                                scope s(output);

                                if (task_atom->type == node_add_list)
                                {
                                    generate_effects_add(task_atom, output);
                                }
                                else if (task_atom->type == node_delete_list)
                                {
                                    generate_effects_delete(task_atom, output);
                                }
                                else if (is_operator(ast, task_atom))
                                {
                                    generate_operator_task(ast, method, task_atom, output);
                                }
                                else if (is_method(ast, task_atom))
                                {
                                    generate_method_task(ast, method, task_atom, output);
                                }
                                else
                                {
                                    // unknown construct in task list
                                    plnnrc_assert(false);
                                }
                            }

                            if (is_last(task_atom) && !ann->foreach)
                            {
                                output.writeln("method->expanded = true;");
                            }

                            if (is_last(task_atom) || (!is_effect_list(task_atom) && is_method(ast, task_atom)))
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
    }

    void generate_task_name_function(ast::tree& ast, node* domain, bool enabled, formatter& output)
    {
        if (!enabled)
        {
            output.writeln("const char* task_name(task_type type) { return \"<none>\"; }");
            output.newline();
        }
        else
        {
            output.writeln("static const char* task_type_to_name[] =");
            {
                class_scope s(output);

                output.writeln("\"<none>\",");

                for (node* method = domain->first_child; method != 0; method = method->next_sibling)
                {
                    if (method->type != node_method)
                    {
                        continue;
                    }

                    node* method_atom = method->first_child;
                    plnnrc_assert(method_atom);

                    output.writeln("\"%s\",", method_atom->s_expr->token);
                }

                for (id_table_values operators = ast.operators.values(); !operators.empty(); operators.pop())
                {
                    node* operatr = operators.value();
                    node* operator_atom = operatr->first_child;
                    output.writeln("\"%s\",", operator_atom->s_expr->token);
                }
            }

            output.writeln("const char* task_name(task_type type) { return task_type_to_name[type]; }");
            output.newline();
        }
    }
}

bool generate_header(ast::tree& ast, writer& writer, codegen_options options)
{
    formatter output(writer, options.tab, options.newline);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    node* worldstate = find_child(ast.root(), node_worldstate);
    plnnrc_assert(worldstate);

    node* domain = find_child(ast.root(), node_domain);
    plnnrc_assert(domain);

    output.writeln("#ifndef %s", options.include_guard);
    output.writeln("#define %s", options.include_guard);
    output.newline();

    generate_worldstate(ast, worldstate, output);
    generate_task_type_enum(ast, domain, output);
    generate_param_structs(ast, domain, output);
    generate_forward_decls(ast, domain, output);

    output.writeln("#endif");

    return true;
}

bool generate_source(ast::tree& ast, writer& writer, codegen_options options)
{
    formatter output(writer, options.tab, options.newline);

    if (!output.init(DERPLANNER_CODEGEN_OUTPUT_BUFFER_SIZE))
    {
        return false;
    }

    node* domain = find_child(ast.root(), node_domain);
    plnnrc_assert(domain);

    generate_includes(ast, options.header_file_name, output);
    generate_task_name_function(ast, domain, options.runtime_task_names, output);
    generate_preconditions(ast, domain, output);
    generate_branch_expands(ast, domain, output);

    return true;
}

}
