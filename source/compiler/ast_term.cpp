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
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "error_tools.h"
#include "tree_tools.h"
#include "tokens.h"
#include "ast_term.h"

namespace plnnrc {
namespace ast {

node* build_namespace(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_EXPECT_CHILD(ast, s_expr, sexpr::node_symbol);

    for (sexpr::node* n = s_expr->first_child; n != 0; n = n->next_sibling)
    {
        PLNNRC_EXPECT_TYPE(ast, n, sexpr::node_symbol);
    }

    PLNNRC_CHECK_NODE(result, ast.make_node(node_namespace, s_expr));

    return result;
}

node* build_atom(tree& ast, sexpr::node* s_expr)
{
    node_type atom_type = node_atom;

    if (is_token(s_expr->first_child, token_eq))
    {
        atom_type = node_atom_eq;
    }

    sexpr::node* name_expr = s_expr->first_child;

    bool lazy = is_token(name_expr, token_lazy);

    if (lazy)
    {
        name_expr = name_expr->next_sibling;
    }

    PLNNRC_CHECK_NODE(atom, ast.make_node(atom_type, name_expr));

    if (atom_type == node_atom)
    {
        annotation<atom_ann>(atom)->lazy = lazy;
    }

    for (sexpr::node* c_expr = name_expr->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        PLNNRC_CHECK_NODE(argument, build_term(ast, c_expr));
        append_child(atom, argument);
    }

    return atom;
}

node* build_term(tree& ast, sexpr::node* s_expr)
{
    switch (s_expr->type)
    {
    case sexpr::node_symbol:
        return build_variable_term(ast, s_expr);
    case sexpr::node_int:
        return build_int_term(ast, s_expr);
    case sexpr::node_float:
        return build_float_term(ast, s_expr);
    case sexpr::node_list:
        return build_call_term(ast, s_expr);
    default:
        plnnrc_assert(false);
        return 0;
    }
}

node* build_variable_term(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(variable, ast.make_node(node_term_variable, s_expr));
    return variable;
}

node* build_int_term(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(literal, ast.make_node(node_term_int, s_expr));
    return literal;
}

node* build_float_term(tree& ast, sexpr::node* s_expr)
{
    PLNNRC_CHECK_NODE(literal, ast.make_node(node_term_float, s_expr));
    return literal;
}

node* build_call_term(tree& ast, sexpr::node* s_expr)
{
    sexpr::node* name_expr = s_expr->first_child;
    plnnrc_assert(name_expr && name_expr->type == sexpr::node_symbol);

    PLNNRC_CHECK_NODE(call_term, ast.make_node(node_term_call, name_expr));

    for (sexpr::node* c_expr = name_expr->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        PLNNRC_CHECK_NODE(term, build_term(ast, c_expr));
        append_child(call_term, term);
    }

    return call_term;
}

}
}
