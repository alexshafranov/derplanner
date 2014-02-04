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
#include "error_tools.h"
#include "ast_infer.h"

namespace plnnrc {
namespace ast {

namespace
{
    node* seed_parameter_types(tree& ast, node* root, node* argument_type)
    {
        node* ws_type = argument_type;

        for (node* c = root->first_child; c != 0; c = c->next_sibling, ws_type = ws_type->next_sibling)
        {
            PLNNRC_BREAK(replace_with_error_if(!ws_type, ast, root, error_wrong_number_of_arguments));

            if (c->type == node_term_variable)
            {
                type_tag(c, annotation<ws_type_ann>(ws_type)->type_tag);
            }

            if (c->type == node_term_call)
            {
                node* ws_func = ast.ws_funcs.find(c->s_expr->token);
                PLNNRC_CONTINUE(replace_with_error_if(!ws_func, ast, c, error_undefined) << c->s_expr);
                node* ws_return_type = ws_func->first_child->next_sibling;
                int ws_type_tag = annotation<ws_type_ann>(ws_type)->type_tag;
                int ws_return_type_tag = annotation<ws_type_ann>(ws_return_type)->type_tag;
                PLNNRC_CONTINUE(replace_with_error_if(ws_type_tag != ws_return_type_tag, ast, c, error_type_mismatch));
            }
        }

        return ws_type;
    }

    void set_or_check_var_type(tree& ast, node* var)
    {
        node* def = definition(var);

        if (is_parameter(def))
        {
            // if type is not yet assigned -> assign
            if (!type_tag(def))
            {
                type_tag(def, type_tag(var));
            }
            else
            {
                // otherwise, check types match
                int def_type = type_tag(def);
                int var_type = type_tag(var);

                replace_with_error_if(def_type != var_type, ast, var->parent, error_type_mismatch);
            }
        }
    }

    void propagate_types_up(tree& ast)
    {
        // for each precondition and for each effect and function call in task list propagate variable types to their definitions
        for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
        {
            node* method = methods.value();
            node* method_atom = method->first_child;

            for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                node* precondition = branch->first_child;

                for (node* var = precondition; var != 0; var = preorder_traversal_next(precondition, var))
                {
                    if (var->type == node_term_variable && is_bound(var) && (var->parent->type == node_atom || var->parent->type == node_term_call))
                    {
                        set_or_check_var_type(ast, var);
                    }
                }

                node* tasklist = precondition->next_sibling;

                for (node* task = tasklist->first_child; task != 0; task = task->next_sibling)
                {
                    if (is_effect_list(task))
                    {
                        for (node* var = task->first_child; var != 0; var = var->next_sibling)
                        {
                            if (var->type == node_term_variable)
                            {
                                set_or_check_var_type(ast, var);
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
                                        set_or_check_var_type(ast, var);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    void propagate_types_down(tree& ast)
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
                    node* tasklist = branch->first_child->next_sibling;

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

                        node* param = callee_atom->first_child;

                        for (node* arg = task->first_child; arg != 0; arg = arg->next_sibling, param = param->next_sibling)
                        {
                            PLNNRC_BREAK(replace_with_error_if(!param, ast, task, error_wrong_number_of_arguments));

                            if (arg->type == node_term_variable)
                            {
                                plnnrc_assert(is_bound(arg));
                                node* def = definition(arg);

                                PLNNRC_CONTINUE(replace_with_error_if(!type_tag(def), ast, arg, error_unable_to_infer_type));

                                if (!type_tag(param))
                                {
                                    type_tag(param, type_tag(def));
                                }
                                else
                                {
                                    // check types match
                                    replace_with_error_if(type_tag(param) != type_tag(def), ast, arg, error_type_mismatch);
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
                                    replace_with_error_if(type_tag(param) != annotation<ws_type_ann>(ws_return_type)->type_tag, ast, arg, error_type_mismatch);
                                }
                            }
                        }

                        replace_with_error_if(param != 0, ast, task, error_wrong_number_of_arguments);
                    }
                }

                ann->processed = true;
                num_methods_processed++;
            }

            if (num_methods_processed == 0)
            {
                // unable to infer some method parameters.
                for (id_table_values methods = ast.methods.values(); !methods.empty(); methods.pop())
                {
                    node* method = methods.value();
                    node* method_atom = method->first_child;

                    for (node* param = method_atom->first_child; param != 0; param = param->next_sibling)
                    {
                        if (!type_tag(param))
                        {
                            replace_with_error(ast, param, error_unable_to_infer_type);
                        }
                    }
                }

                break;
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

void seed_types(tree& ast, node* root)
{
    for (node* n = root; n != 0; n = preorder_traversal_next(root, n))
    {
        if (n->type == node_atom)
        {
            node* ws_atom = ast.ws_atoms.find(n->s_expr->token);
            PLNNRC_CONTINUE(replace_with_error_if(!ws_atom, ast, n, error_undefined) << n->s_expr);

            node* ws_type = seed_parameter_types(ast, n, ws_atom->first_child);

            if (n->type == node_error)
            {
                continue;
            }

            PLNNRC_CONTINUE(replace_with_error_if(ws_type != 0, ast, n, error_wrong_number_of_arguments));
        }

        if (n->type == node_term_call)
        {
            node* ws_func = ast.ws_funcs.find(n->s_expr->token);
            PLNNRC_CONTINUE(replace_with_error_if(!ws_func, ast, n, error_undefined) << n->s_expr);

            node* ws_type = seed_parameter_types(ast, n, ws_func->first_child->first_child);

            if (n->type == node_error)
            {
                continue;
            }

            PLNNRC_CONTINUE(replace_with_error_if(ws_type != 0, ast, n, error_wrong_number_of_arguments));
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

        for (node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            node* tasklist = branch->first_child->next_sibling;

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

    propagate_types_up(ast);

    if (ast.methods.count() > 0)
    {
        propagate_types_down(ast);
    }
}

}
}
