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
#include "formatter.h"
#include "codegen_precondition.h"

namespace plnnrc {

class paste_precondition_function_call : public paste_func
{
public:
    ast::node* function_call;
    const char* var_prefix;

    paste_precondition_function_call(ast::node* function_call, const char* var_prefix)
        : function_call(function_call)
        , var_prefix(var_prefix)
    {
    }

    virtual void operator()(formatter& output)
    {
        output.put_id(function_call->s_expr->token);
        output.put_char('(');

        for (ast::node* argument = function_call->first_child; argument != 0; argument = argument->next_sibling)
        {
            switch (argument->type)
            {
            case ast::node_term_variable:
                {
                    int var_index = ast::annotation<ast::term_ann>(argument)->var_index;
                    output.put_str(var_prefix);
                    output.put_int(var_index);
                }
                break;
            case ast::node_term_call:
                {
                    paste_precondition_function_call paste(argument, var_prefix);
                    output.put_str("world.");
                    paste(output);
                }
                break;
            default:
                // unsupported argument type
                plnnrc_assert(false);
            }

            if (!is_last(argument))
            {
                output.put_str(", ");
            }
        }

        output.put_char(')');
    }
};

void generate_preconditions(ast::tree& ast, ast::node* domain, formatter& output)
{
    unsigned branch_index = 0;

    for (ast::node* method = domain->first_child; method != 0; method = method->next_sibling)
    {
        if (method->type != ast::node_method)
        {
            continue;
        }

        for (ast::node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            plnnrc_assert(branch->type == ast::node_branch);

            ast::node* precondition = branch->first_child;

            generate_precondition_state(ast, precondition, branch_index, output);
            generate_precondition_next(ast, precondition, branch_index, output);

            ++branch_index;
        }
    }
}

void generate_precondition_state(ast::tree& ast, ast::node* root, unsigned branch_index, formatter& output)
{
    output.writeln("// method %s [%d:%d]", root->parent->parent->first_child->s_expr->token, root->s_expr->line, root->s_expr->column);

    output.writeln("struct p%d_state", branch_index);
    {
        class_scope s(output);

        int last_var_index = -1;

        for (ast::node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == ast::node_term_variable)
            {
                int var_index = ast::annotation<ast::term_ann>(n)->var_index;

                if (var_index > last_var_index)
                {
                    ast::node* ws_type = ast.type_tag_to_node[ast::type_tag(n)];
                    output.writeln("// %s [%d:%d]", n->s_expr->token, n->s_expr->line, n->s_expr->column);
                    output.writeln("%s _%d;", ws_type->s_expr->first_child->token, var_index);
                    last_var_index = var_index;
                }
            }
        }

        for (ast::node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (n->type == ast::node_atom)
            {
                const char* id = n->s_expr->token;
                output.writeln("%i_tuple* %i_%d;", id, id, ast::annotation<ast::atom_ann>(n)->index);
            }
        }

        output.writeln("int stage;");
    }
}

void generate_precondition_next(ast::tree& ast, ast::node* root, unsigned branch_index, formatter& output)
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

void generate_precondition_satisfier(ast::tree& ast, ast::node* root, formatter& output)
{
    plnnrc_assert(root->type == ast::node_op_or);

    for (ast::node* child = root->first_child; child != 0; child = child->next_sibling)
    {
        plnnrc_assert(child->type == ast::node_op_and);

        generate_conjunctive_clause(ast, child, output);
    }
}

void generate_conjunctive_clause(ast::tree& ast, ast::node* root, formatter& output)
{
    plnnrc_assert(root->type == ast::node_op_and);

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

void generate_literal_chain(ast::tree& ast, ast::node* root, formatter& output)
{
    plnnrc_assert(root->type == ast::node_op_not || root->type == ast::node_term_call || is_atom(root));

    ast::node* atom = root;

    if (root->type == ast::node_op_not)
    {
        atom = root->first_child;
    }

    // special case atoms
    if (atom->type == ast::node_atom_eq)
    {
        generate_literal_chain_atom_eq(ast, root, atom, output);
        return;
    }

    if (atom->type == ast::node_term_call)
    {
        generate_literal_chain_call_term(ast, root, atom, output);
        return;
    }

    const char* atom_id = atom->s_expr->token;
    int atom_index = ast::annotation<ast::atom_ann>(atom)->index;

    if (root->type == ast::node_op_not && all_unbound(atom))
    {
        output.writeln("if (!tuple_list::head<%i_tuple>(world.atoms[atom_%i]))", atom_id, atom_id);
        {
            scope s(output);

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
    else if (root->type == ast::node_op_not && all_bound(atom))
    {
        output.writeln("for (state.%i_%d = tuple_list::head<%i_tuple>(world.atoms[atom_%i]); state.%i_%d != 0; state.%i_%d = state.%i_%d->next)",
            atom_id, atom_index,
            atom_id,
            atom_id,
            atom_id, atom_index,
            atom_id, atom_index,
            atom_id, atom_index);
        {
            scope s(output);

            int atom_param_index = 0;

            for (ast::node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (term->type == ast::node_term_variable)
                {
                    int var_index = ast::annotation<ast::term_ann>(term)->var_index;

                    output.writeln("if (state.%i_%d->_%d == state._%d)", atom_id, atom_index, atom_param_index, var_index);
                    {
                        scope s(output, !is_last(term));
                        output.writeln("break;");
                    }
                }

                if (term->type == ast::node_term_call)
                {
                    paste_precondition_function_call paste(term, "state._");

                    output.writeln("if (state.%i_%d->_%d == world.%p)", atom_id, atom_index, atom_param_index, &paste);
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
        output.writeln("for (state.%i_%d = tuple_list::head<%i_tuple>(world.atoms[atom_%i]); state.%i_%d != 0; state.%i_%d = state.%i_%d->next)",
            atom_id, atom_index,
            atom_id,
            atom_id,
            atom_id, atom_index,
            atom_id, atom_index,
            atom_id, atom_index);
        {
            scope s(output, is_first(root));

            const char* comparison_op = "!=";

            if (root->type == ast::node_op_not)
            {
                comparison_op = "==";
            }

            int atom_param_index = 0;

            for (ast::node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (term->type == ast::node_term_variable && definition(term))
                {
                    int var_index = ast::annotation<ast::term_ann>(term)->var_index;

                    output.writeln("if (state.%i_%d->_%d %s state._%d)", atom_id, atom_index, atom_param_index, comparison_op, var_index);
                    {
                        scope s(output);
                        output.writeln("continue;");
                    }
                }

                if (term->type == ast::node_term_call)
                {
                    paste_precondition_function_call paste(term, "state._");

                    output.writeln("if (state.%i_%d->_%d %s world.%p)", atom_id, atom_index, atom_param_index, comparison_op, &paste);
                    {
                        scope s(output);
                        output.writeln("continue;");
                    }
                }

                ++atom_param_index;
            }

            atom_param_index = 0;

            for (ast::node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (term->type == ast::node_term_variable && !definition(term))
                {
                    int var_index = ast::annotation<ast::term_ann>(term)->var_index;
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

void generate_literal_chain_atom_eq(ast::tree& ast, ast::node* root, ast::node* atom, formatter& output)
{
    ast::node* arg_0 = atom->first_child;
    plnnrc_assert(arg_0 && arg_0->type == ast::node_term_variable);
    ast::node* arg_1 = arg_0->next_sibling;
    plnnrc_assert(arg_1 && arg_1->type == ast::node_term_variable && !arg_1->next_sibling);

    plnnrc_assert(definition(arg_0) && definition(arg_1));

    const char* comparison_op = "==";

    if (root->type == ast::node_op_not)
    {
        comparison_op = "!=";
    }

    int var_index_0 = ast::annotation<ast::term_ann>(arg_0)->var_index;
    int var_index_1 = ast::annotation<ast::term_ann>(arg_1)->var_index;

    output.writeln("if (state._%d %s state._%d)", var_index_0, comparison_op, var_index_1);
    {
        scope s(output, root->next_sibling != 0);

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

void generate_literal_chain_call_term(ast::tree& ast, ast::node* root, ast::node* atom, formatter& output)
{
    paste_precondition_function_call paste(atom, "state._");

    output.writeln("if (%sworld.%p)", root->type == ast::node_op_not ? "!" : "", &paste);
    {
        scope s(output, root->next_sibling != 0);

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
