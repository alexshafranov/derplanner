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

class Paste_Precondition_Function_Call : public Paste_Func
{
public:
    ast::Node* function_call;
    const char* var_prefix;

    Paste_Precondition_Function_Call(ast::Node* function_call, const char* var_prefix)
        : function_call(function_call)
        , var_prefix(var_prefix)
    {
    }

    virtual void operator()(Formatter& output)
    {
        output.put_id(function_call->s_expr->token);
        output.put_char('(');

        for (ast::Node* argument = function_call->first_child; argument != 0; argument = argument->next_sibling)
        {
            switch (argument->type)
            {
            case ast::node_term_variable:
                {
                    int var_index = ast::annotation<ast::Term_Ann>(argument)->var_index;
                    output.put_str(var_prefix);
                    output.put_int(var_index);
                }
                break;
            case ast::node_term_call:
                {
                    Paste_Precondition_Function_Call paste(argument, var_prefix);
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

void generate_preconditions(ast::Tree& ast, ast::Node* domain, Formatter& output)
{
    unsigned branch_index = 0;

    for (ast::Node* method = domain->first_child; method != 0; method = method->next_sibling)
    {
        if (!ast::is_method(method))
        {
            continue;
        }

        for (ast::Node* branch = method->first_child->next_sibling; branch != 0; branch = branch->next_sibling)
        {
            plnnrc_assert(ast::is_branch(branch));

            ast::Node* precondition = branch->first_child;

            generate_precondition_state(ast, precondition, branch_index, output);
            generate_precondition_next(ast, precondition, branch_index, output);

            ++branch_index;
        }
    }
}

void generate_precondition_state(ast::Tree& ast, ast::Node* root, unsigned branch_index, Formatter& output)
{
    output.writeln("// method %s [%d:%d]", root->parent->parent->first_child->s_expr->token, root->s_expr->line, root->s_expr->column);

    output.writeln("struct p%d_state", branch_index);
    {
        Class_Scope s(output);

        int last_var_index = -1;

        for (ast::Node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (ast::is_term_variable(n))
            {
                int var_index = ast::annotation<ast::Term_Ann>(n)->var_index;

                if (var_index > last_var_index)
                {
                    ast::Node* ws_type = ast.type_tag_to_node[ast::type_tag(n)];
                    output.writeln("// %s [%d:%d]", n->s_expr->token, n->s_expr->line, n->s_expr->column);
                    output.writeln("%s _%d;", ws_type->s_expr->first_child->token, var_index);
                    last_var_index = var_index;
                }
            }
        }

        for (ast::Node* n = root; n != 0; n = preorder_traversal_next(root, n))
        {
            if (ast::is_atom(n))
            {
                const char* id = n->s_expr->token;
                output.writeln("%i_tuple* %i_%d;", id, id, ast::annotation<ast::Atom_Ann>(n)->index);
            }
        }

        output.writeln("int stage;");
    }
}

void generate_precondition_next(ast::Tree& ast, ast::Node* root, unsigned branch_index, Formatter& output)
{
    plnnrc_assert(is_logical_op(root));

    output.writeln("bool next(p%d_state& state, Worldstate& world)", branch_index);
    {
        Scope s(output);

        output.writeln("PLNNR_COROUTINE_BEGIN(state);");
        output.newline();

        generate_precondition_satisfier(ast, root, output, 1);

        output.writeln("PLNNR_COROUTINE_END();");
    }
}

void generate_precondition_satisfier(ast::Tree& ast, ast::Node* root, Formatter& output, int yield_label)
{
    plnnrc_assert(ast::is_op_or(root));

    for (ast::Node* child = root->first_child; child != 0; child = child->next_sibling)
    {
        plnnrc_assert(ast::is_op_and(child));
        generate_conjunctive_clause(ast, child, output, yield_label);
    }
}

void generate_conjunctive_clause(ast::Tree& ast, ast::Node* root, Formatter& output, int yield_label)
{
    plnnrc_assert(ast::is_op_and(root));

    if (root->first_child)
    {
        generate_literal_chain(ast, root->first_child, output, yield_label);
    }
    else
    {
        // the formula is trivial: (or (and))
        output.writeln("PLNNR_COROUTINE_YIELD(state, %d);", yield_label);
        output.newline();
    }
}

void generate_literal_chain(ast::Tree& ast, ast::Node* root, Formatter& output, int yield_label)
{
    plnnrc_assert(ast::is_op_not(root) || ast::is_term_call(root) || is_atom(root) || is_comparison_op(root));

    ast::Node* atom = root;

    if (ast::is_op_not(root))
    {
        atom = root->first_child;
    }

    if (ast::is_comparison_op(atom))
    {
        generate_literal_chain_comparison(ast, root, atom, output, yield_label);
        return;
    }

    if (ast::is_term_call(atom))
    {
        generate_literal_chain_call_term(ast, root, atom, output, yield_label);
        return;
    }

    const char* atom_id = atom->s_expr->token;
    int atom_index = ast::annotation<ast::Atom_Ann>(atom)->index;

    if (ast::is_op_not(root) && all_unbound(atom))
    {
        output.writeln("if (!tuple_list::head<%i_tuple>(world.atoms[atom_%i]))", atom_id, atom_id);
        {
            Scope s(output);

            if (root->next_sibling)
            {
                generate_literal_chain(ast, root->next_sibling, output, yield_label + 1);
            }
            else
            {
                output.writeln("PLNNR_COROUTINE_YIELD(state, %d);", yield_label);
            }
        }
    }
    else if (ast::is_op_not(root) && all_bound(atom))
    {
        output.writeln("for (state.%i_%d = tuple_list::head<%i_tuple>(world.atoms[atom_%i]); state.%i_%d != 0; state.%i_%d = state.%i_%d->next)",
            atom_id, atom_index,
            atom_id,
            atom_id,
            atom_id, atom_index,
            atom_id, atom_index,
            atom_id, atom_index);
        {
            Scope s(output);

            int atom_param_index = 0;

            for (ast::Node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (ast::is_term_variable(term))
                {
                    int var_index = ast::annotation<ast::Term_Ann>(term)->var_index;

                    output.writeln("if (state.%i_%d->_%d == state._%d)", atom_id, atom_index, atom_param_index, var_index);
                    {
                        Scope s(output, !is_last(term));
                        output.writeln("break;");
                    }
                }

                if (ast::is_term_call(term))
                {
                    Paste_Precondition_Function_Call paste(term, "state._");

                    output.writeln("if (state.%i_%d->_%d == world.%p)", atom_id, atom_index, atom_param_index, &paste);
                    {
                        Scope s(output, !is_last(term));
                        output.writeln("break;");
                    }
                }

                ++atom_param_index;
            }
        }

        output.writeln("if (state.%i_%d == 0)", atom_id, atom_index);
        {
            Scope s(output, is_first(root));

            if (root->next_sibling)
            {
                generate_literal_chain(ast, root->next_sibling, output, yield_label + 1);
            }
            else
            {
                output.writeln("PLNNR_COROUTINE_YIELD(state, %d);", yield_label);
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
            Scope s(output, is_first(root));

            const char* comparison_op = "!=";

            if (ast::is_op_not(root))
            {
                comparison_op = "==";
            }

            int atom_param_index = 0;

            for (ast::Node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (ast::is_term_variable(term) && definition(term))
                {
                    int var_index = ast::annotation<ast::Term_Ann>(term)->var_index;

                    output.writeln("if (state.%i_%d->_%d %s state._%d)", atom_id, atom_index, atom_param_index, comparison_op, var_index);
                    {
                        Scope s(output);
                        output.writeln("continue;");
                    }
                }

                if (ast::is_term_call(term))
                {
                    Paste_Precondition_Function_Call paste(term, "state._");

                    output.writeln("if (state.%i_%d->_%d %s world.%p)", atom_id, atom_index, atom_param_index, comparison_op, &paste);
                    {
                        Scope s(output);
                        output.writeln("continue;");
                    }
                }

                ++atom_param_index;
            }

            atom_param_index = 0;

            for (ast::Node* term = atom->first_child; term != 0; term = term->next_sibling)
            {
                if (ast::is_term_variable(term) && !definition(term))
                {
                    int var_index = ast::annotation<ast::Term_Ann>(term)->var_index;
                    output.writeln("state._%d = state.%i_%d->_%d;", var_index, atom_id, atom_index, atom_param_index);
                    output.newline();
                }

                ++atom_param_index;
            }

            if (root->next_sibling)
            {
                generate_literal_chain(ast, root->next_sibling, output, yield_label + 1);
            }
            else
            {
                output.writeln("PLNNR_COROUTINE_YIELD(state, %d);", yield_label);
            }
        }
    }
}

void generate_literal_chain_comparison(ast::Tree& ast, ast::Node* root, ast::Node* atom, Formatter& output, int yield_label)
{
    ast::Node* arg_0 = atom->first_child;
    plnnrc_assert(arg_0 && ast::is_term_variable(arg_0));
    ast::Node* arg_1 = arg_0->next_sibling;
    plnnrc_assert(arg_1 && ast::is_term_variable(arg_1) && !arg_1->next_sibling);

    plnnrc_assert(definition(arg_0) && definition(arg_1));

    const char* comparison_op = atom->s_expr->token;

    int var_index_0 = ast::annotation<ast::Term_Ann>(arg_0)->var_index;
    int var_index_1 = ast::annotation<ast::Term_Ann>(arg_1)->var_index;

    if (ast::is_op_not(root))
    {
        output.writeln("if (!(state._%d %s state._%d))", var_index_0, comparison_op, var_index_1);
    }
    else
    {
        output.writeln("if (state._%d %s state._%d)", var_index_0, comparison_op, var_index_1);
    }

    {
        Scope s(output, root->next_sibling != 0);

        if (root->next_sibling)
        {
            generate_literal_chain(ast, root->next_sibling, output, yield_label);
        }
        else
        {
            output.writeln("PLNNR_COROUTINE_YIELD(state, %d);", yield_label);
        }
    }
}

void generate_literal_chain_call_term(ast::Tree& ast, ast::Node* root, ast::Node* atom, Formatter& output, int yield_label)
{
    Paste_Precondition_Function_Call paste(atom, "state._");

    output.writeln("if (%sworld.%p)", ast::is_op_not(root) ? "!" : "", &paste);
    {
        Scope s(output, root->next_sibling != 0);

        if (root->next_sibling)
        {
            generate_literal_chain(ast, root->next_sibling, output, yield_label);
        }
        else
        {
            output.writeln("PLNNR_COROUTINE_YIELD(state, %d);", yield_label);
        }
    }
}

}
