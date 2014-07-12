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
    Node* seed_parameter_types(Tree& ast, Node* root, Node* argument_type)
    {
        Node* ws_type = argument_type;

        for (Node* c = root->first_child; c != 0; c = c->next_sibling, ws_type = ws_type->next_sibling)
        {
            PLNNRC_BREAK(replace_with_error_if(!ws_type, ast, root, error_wrong_number_of_arguments) << root->s_expr);

            if (is_term_variable(c))
            {
                type_tag(c, annotation<WS_Type_Ann>(ws_type)->type_tag);

                Node* def = definition(c);

                if (def && type_tag(def))
                {
                    int def_type = type_tag(def);
                    int var_type = type_tag(c);

                    replace_with_error_if(def_type != var_type, ast, c, error_type_mismatch)
                        << ast.type_tag_to_node[def_type]->s_expr->first_child
                        << ast.type_tag_to_node[var_type]->s_expr->first_child;
                }
            }

            if (is_term_call(c))
            {
                Node* ws_func = ast.ws_funcs.find(c->s_expr->token);
                PLNNRC_CONTINUE(replace_with_error_if(!ws_func, ast, c, error_undefined) << c->s_expr);
                Node* ws_return_type = ws_func->first_child->next_sibling;
                int ws_type_tag = annotation<WS_Type_Ann>(ws_type)->type_tag;
                int ws_return_type_tag = annotation<WS_Type_Ann>(ws_return_type)->type_tag;
                PLNNRC_CONTINUE(replace_with_error_if(ws_type_tag != ws_return_type_tag, ast, c, error_type_mismatch)
                    << ws_type->s_expr->first_child
                    << ws_return_type->s_expr->first_child);
            }
        }

        return ws_type;
    }

    void set_or_check_var_type(Tree& ast, Node* var)
    {
        Node* def = definition(var);

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

                replace_with_error_if(def_type != var_type, ast, var->parent, error_type_mismatch)
                    << ast.type_tag_to_node[def_type]->s_expr->first_child
                    << ast.type_tag_to_node[var_type]->s_expr->first_child;
            }
        }
    }

    void propagate_types_up(Tree& ast)
    {
        // for each precondition and for each effect and function call in task list propagate variable types to their definitions
        for (Id_Table_Values methods = ast.methods.values(); !methods.empty(); methods.pop())
        {
            Node* method = methods.value();
            Node* method_atom = method->first_child;

            for (Node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                Node* precondition = branch->first_child;

                for (Node* var = precondition; var != 0; var = preorder_traversal_next(precondition, var))
                {
                    if (is_term_variable(var) && is_bound(var) && (is_atom(var->parent) || is_term_call(var->parent)))
                    {
                        set_or_check_var_type(ast, var);
                    }
                }

                Node* tasklist = precondition->next_sibling;

                for (Node* task = tasklist->first_child; task != 0; task = task->next_sibling)
                {
                    if (is_effect_list(task))
                    {
                        for (Node* var = task->first_child; var != 0; var = var->next_sibling)
                        {
                            if (is_term_variable(var))
                            {
                                set_or_check_var_type(ast, var);
                            }
                        }
                    }
                    else
                    {
                        for (Node* call = task->first_child; call != 0; call = call->next_sibling)
                        {
                            if (is_term_call(call))
                            {
                                for (Node* var = call->first_child; var != 0; var = var->next_sibling)
                                {
                                    if (is_term_variable(var))
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

    void propagate_types_down(Tree& ast)
    {
        // for each task in a task list propagate variable and function return types
        // down to undefined argument types of operators and methods

        bool all_method_param_types_inferred = false;

        while (!all_method_param_types_inferred)
        {
            all_method_param_types_inferred = true;

            int num_methods_processed = 0;

            for (Id_Table_Values methods = ast.methods.values(); !methods.empty(); methods.pop())
            {
                Node* method = methods.value();
                Node* method_atom = method->first_child;

                Method_Ann* ann = annotation<Method_Ann>(method);

                if (ann->processed)
                {
                    continue;
                }

                if (has_untyped_params(method_atom))
                {
                    all_method_param_types_inferred = false;
                    continue;
                }

                for (Node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
                {
                    Node* tasklist = branch->first_child->next_sibling;

                    for (Node* task = tasklist->first_child; task != 0; task = task->next_sibling)
                    {
                        if (is_effect_list(task))
                        {
                            continue;
                        }

                        Node* callee = ast.methods.find(task->s_expr->token);

                        if (!callee)
                        {
                            callee = ast.operators.find(task->s_expr->token);
                        }

                        plnnrc_assert(callee);

                        Node* callee_atom = callee->first_child;

                        Node* param = callee_atom->first_child;

                        for (Node* arg = task->first_child; arg != 0; arg = arg->next_sibling, param = param->next_sibling)
                        {
                            PLNNRC_BREAK(replace_with_error_if(!param, ast, task, error_wrong_number_of_arguments) << task->s_expr);

                            if (is_term_variable(arg))
                            {
                                plnnrc_assert(is_bound(arg));
                                Node* def = definition(arg);

                                PLNNRC_CONTINUE(replace_with_error_if(!type_tag(def), ast, arg, error_unable_to_infer_type) << arg->s_expr);

                                if (!type_tag(param))
                                {
                                    type_tag(param, type_tag(def));
                                }
                                else
                                {
                                    // check types match
                                    replace_with_error_if(type_tag(param) != type_tag(def), ast, arg, error_type_mismatch)
                                        << ast.type_tag_to_node[type_tag(param)]->s_expr->first_child
                                        << ast.type_tag_to_node[type_tag(def)]->s_expr->first_child;
                                }
                            }

                            if (is_term_call(arg))
                            {
                                Node* ws_func = ast.ws_funcs.find(arg->s_expr->token);
                                plnnrc_assert(ws_func);
                                Node* ws_return_type = ws_func->first_child->next_sibling;
                                plnnrc_assert(ws_return_type);

                                if (!type_tag(param))
                                {
                                    type_tag(param, annotation<WS_Type_Ann>(ws_return_type)->type_tag);
                                }
                                else
                                {
                                    // check types match
                                    replace_with_error_if(type_tag(param) != annotation<WS_Type_Ann>(ws_return_type)->type_tag, ast, arg, error_type_mismatch)
                                        << ast.type_tag_to_node[type_tag(param)]->s_expr->first_child
                                        << ws_return_type->s_expr->first_child;
                                }
                            }
                        }

                        replace_with_error_if(param != 0, ast, task, error_wrong_number_of_arguments) << task->s_expr;
                    }
                }

                ann->processed = true;
                num_methods_processed++;
            }

            if (num_methods_processed == 0)
            {
                // unable to infer some method parameters.
                for (Id_Table_Values methods = ast.methods.values(); !methods.empty(); methods.pop())
                {
                    Node* method = methods.value();
                    Node* method_atom = method->first_child;

                    for (Node* param = method_atom->first_child; param != 0; param = param->next_sibling)
                    {
                        if (!type_tag(param))
                        {
                            replace_with_error(ast, param, error_unable_to_infer_type) << param->s_expr;
                        }
                    }
                }

                break;
            }
        }

        // assign types to all other bound variables, once method parameter types are figured out
        for (Id_Table_Values methods = ast.methods.values(); !methods.empty(); methods.pop())
        {
            Node* method = methods.value();
            Node* method_atom = method->first_child;

            for (Node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
            {
                Node* precondition = branch->first_child;

                for (Node* n = precondition; n != 0; n = preorder_traversal_next(precondition, n))
                {
                    if (is_comparison_op(n))
                    {
                        for (Node* var = n->first_child; var != 0; var = var->next_sibling)
                        {
                            if (is_term_variable(var))
                            {
                                Node* def = definition(var);
                                PLNNRC_SKIP_ERROR_NODE(def);
                                type_tag(var, type_tag(def));
                            }
                        }
                    }
                }
            }
        }
    }
}

void seed_types(Tree& ast, Node* root)
{
    for (Node* n = root; n != 0; n = preorder_traversal_next(root, n))
    {
        if (is_atom(n))
        {
            Node* ws_atom = ast.ws_atoms.find(n->s_expr->token);
            PLNNRC_CONTINUE(replace_with_error_if(!ws_atom, ast, n, error_undefined) << n->s_expr);

            Node* ws_type = seed_parameter_types(ast, n, ws_atom->first_child);

            if (is_error(n))
            {
                continue;
            }

            PLNNRC_CONTINUE(replace_with_error_if(ws_type != 0, ast, n, error_wrong_number_of_arguments) << n->s_expr);
        }

        if (is_term_call(n))
        {
            Node* ws_func = ast.ws_funcs.find(n->s_expr->token);
            PLNNRC_CONTINUE(replace_with_error_if(!ws_func, ast, n, error_undefined) << n->s_expr);

            Node* ws_type = seed_parameter_types(ast, n, ws_func->first_child->first_child);

            if (is_error(n))
            {
                continue;
            }

            PLNNRC_CONTINUE(replace_with_error_if(ws_type != 0, ast, n, error_wrong_number_of_arguments) << n->s_expr);
        }
    }
}

bool has_untyped_params(Node* method_atom)
{
    for (Node* param = method_atom->first_child; param != 0; param = param->next_sibling)
    {
        if (!type_tag(param))
        {
            return true;
        }
    }

    return false;
}

void seed_types(Tree& ast)
{
    // for each precondition assign types to variables based on worldstate's atoms and function parameter types.
    for (Id_Table_Values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        Node* method = methods.value();
        Node* method_atom = method->first_child;

        for (Node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            seed_types(ast, branch->first_child);
        }
    }

    // for each operator effect set variable types based on worldstate's atom parameter types.
    for (Id_Table_Values operators = ast.operators.values(); !operators.empty(); operators.pop())
    {
        Node* operatr = operators.value();
        Node* operator_atom = operatr->first_child;

        for (Node* effect_list = operator_atom->next_sibling; effect_list != 0; effect_list = effect_list->next_sibling)
        {
            seed_types(ast, effect_list);
        }
    }

    // for each effect in task list and each call term in task list
    for (Id_Table_Values methods = ast.methods.values(); !methods.empty(); methods.pop())
    {
        Node* method = methods.value();
        Node* method_atom = method->first_child;

        for (Node* branch = method_atom->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            PLNNRC_SKIP_SUBTREE_WITH_ERRORS(branch);

            Node* tasklist = branch->first_child->next_sibling;

            for (Node* task = tasklist->first_child; task != 0; task = task->next_sibling)
            {
                if (is_effect_list(task))
                {
                    seed_types(ast, task);
                }
                else
                {
                    for (Node* argument = task->first_child; argument != 0; argument = argument->next_sibling)
                    {
                        if (is_term_call(argument))
                        {
                            seed_types(ast, argument);
                        }
                    }
                }
            }
        }
    }
}

void infer_types(Tree& ast)
{
    propagate_types_up(ast);

    if (ast.methods.count() > 0)
    {
        propagate_types_down(ast);
    }
}

}
}
