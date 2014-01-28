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
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "tree_tools.h"
#include "ast_tools.h"
#include "ast_infer.h"

namespace plnnrc {
namespace ast {

void seed_types(tree& ast, node* root)
{
    for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
    {
        if (n->type == node_atom)
        {
            node* ws_atom = ast.ws_atoms.find(n->s_expr->token);
            plnnrc_assert(ws_atom);
            plnnrc_assert(ws_atom->first_child);

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

                if (c->type == node_term_call)
                {
                    node* ws_func = ast.ws_funcs.find(c->s_expr->token);
                    plnnrc_assert(ws_func);
                    node* ws_return_type = ws_func->first_child->next_sibling;
                    plnnrc_assert(ws_return_type);
                    plnnrc_assert(ws_return_type->type == node_worldstate_type);
                    (void)(ws_return_type);
                    // check argument type
                    plnnrc_assert(annotation<ws_type_ann>(ws_type)->type_tag == annotation<ws_type_ann>(ws_return_type)->type_tag);
                }

                ws_type = ws_type->next_sibling;
            }

            // wrong number of arguments
            plnnrc_assert(!ws_type);
        }

        if (n->type == node_term_call)
        {
            node* ws_func = ast.ws_funcs.find(n->s_expr->token);
            plnnrc_assert(ws_func);

            node* ws_type = ws_func->first_child->first_child;

            for (node* c = n->first_child; c != 0; c = c->next_sibling)
            {
                plnnrc_assert(ws_type);
                plnnrc_assert(ws_type->type == node_worldstate_type);

                if (c->type == node_term_variable)
                {
                    type_tag(c, annotation<ws_type_ann>(ws_type)->type_tag);
                }

                if (c->type == node_term_call)
                {
                    node* ws_return_type = ast.ws_funcs.find(c->s_expr->token)->first_child->next_sibling;
                    plnnrc_assert(ws_return_type);
                    plnnrc_assert(ws_return_type->type == node_worldstate_type);
                    // check argument type
                    int ws_type_tag = annotation<ws_type_ann>(ws_type)->type_tag;
                    int ws_return_type_tag = annotation<ws_type_ann>(ws_return_type)->type_tag;
                    plnnrc_assert(ws_type_tag == ws_return_type_tag);
                    (void)(ws_type_tag);
                    (void)(ws_return_type_tag);
                }

                ws_type = ws_type->next_sibling;
            }

            // wrong number of arguments
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

void infer_types(tree& ast)
{
    // for each precondition assign types to variables based on worldstate's atoms and function parameter types.
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

    // for each operator effect set variable types based on worldstate's atom parameter types.
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

    // for each effect in task list and each call term in task list
    for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        node* method = methods.value();
        node* method_atom = method->first_child;
        plnnrc_assert(method_atom && method_atom->type == node_atom);

        for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            node* tasklist = branch->first_child->next_sibling;
            plnnrc_assert(tasklist);

            for (node* task = tasklist->first_child; task != 0; task = task->next_sibling)
            {
                if (is_effect_list(task))
                {
                    seed_types(ast, task);
                }
                else
                {
                    for (node* argument = task->first_child; argument != 0; argument = argument->next_sibling)
                    {
                        if (argument->type == node_term_call)
                        {
                            seed_types(ast, argument);
                        }
                    }                        
                }
            }
        }
    }

    // for each precondition and for each effect and function call in task list propagate variable types to their definitions
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
                if (var->type == node_term_variable && is_bound(var) && (var->parent->type == node_atom || var->parent->type == node_term_call))
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

            node* tasklist = precondition->next_sibling;
            plnnrc_assert(tasklist);

            for (node* task = tasklist->first_child; task != 0; task = task->next_sibling)
            {
                if (is_effect_list(task))
                {
                    for (node* var = task->first_child; var != 0; var = var->next_sibling)
                    {
                        if (var->type == node_term_variable)
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
                else
                {
                    for (node* call = task->first_child; call != 0; call = call->next_sibling)
                    {
                        if (call->type == node_term_call)
                        {
                            for (node* var = call->first_child; var != 0; var = var->next_sibling)
                            {
                                if (var->type == node_term_variable)
                                {
                                    node* def = definition(var);
                                    plnnrc_assert(def);

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
                }
            }
        }
    }

    if (ast.methods.count() > 0)
    {
        // for each task in a task list propagate variable and function return types
        // down to undefined argument types of operators and methods

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
                        if (is_effect_list(task))
                        {
                            continue;
                        }

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

                            if (arg->type == node_term_variable)
                            {
                                plnnrc_assert(is_bound(arg));
                                node* def = definition(arg);
                                plnnrc_assert(type_tag(def) != 0);

                                if (!type_tag(param))
                                {
                                    type_tag(param, type_tag(def));
                                }
                                else
                                {
                                    // check types match
                                    plnnrc_assert(type_tag(param) == type_tag(def));
                                }
                            }

                            if (arg->type == node_term_call)
                            {
                                node* ws_func = ast.ws_funcs.find(arg->s_expr->token);
                                plnnrc_assert(ws_func);
                                node* ws_return_type = ws_func->first_child->next_sibling;
                                plnnrc_assert(ws_return_type);

                                if (!type_tag(param))
                                {
                                    type_tag(param, annotation<ws_type_ann>(ws_return_type)->type_tag);
                                }
                                else
                                {
                                    // check types match
                                    plnnrc_assert(type_tag(param) == annotation<ws_type_ann>(ws_return_type)->type_tag);
                                }
                            }

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
                        if (var->type == node_term_variable && is_bound(var) && var->parent->type == node_atom_eq)
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
}
