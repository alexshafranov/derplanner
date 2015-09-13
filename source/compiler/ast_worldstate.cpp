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
#include "error_tools.h"
#include "tokens.h"
#include "ast_term.h"
#include "ast_worldstate.h"

namespace plnnrc {
namespace ast {

Node* build_worldstate(Tree& ast, sexpr::Node* s_expr)
{
    PLNNRC_CHECK_NODE(worldstate, ast.make_node(node_worldstate, s_expr));

    PLNNRC_RETURN(expect_next_type(ast, s_expr->first_child, sexpr::node_list));
    sexpr::Node* name_list_expr = s_expr->first_child->next_sibling;

    PLNNRC_CHECK_NODE(worldstate_namespace, build_namespace(ast, name_list_expr));
    append_child(worldstate, worldstate_namespace);

    unsigned total_atom_count = 0;
    unsigned total_func_count = 0;

    for (sexpr::Node* c_expr = name_list_expr->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        if (is_token(c_expr->first_child, token_function))
        {
            total_func_count++;
        }
        else
        {
            total_atom_count++;
        }
    }

    PLNNRC_CHECK(ast.ws_atoms.init(total_atom_count));
    PLNNRC_CHECK(ast.ws_funcs.init(total_func_count));
    PLNNRC_CHECK(ast.ws_types.init(32));

    int type_tag = 1;

    for (sexpr::Node* c_expr = name_list_expr->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        PLNNRC_CONTINUE(expect_type(ast, c_expr, sexpr::node_list, worldstate));
        PLNNRC_CONTINUE(expect_child_type(ast, c_expr, sexpr::node_symbol, worldstate));

        if (is_token(c_expr->first_child, token_function))
        {
            PLNNRC_CHECK_NODE(function_def, ast.make_node(node_function, c_expr));
            PLNNRC_CONTINUE(expect_next_type(ast, c_expr->first_child, sexpr::node_list, worldstate));

            sexpr::Node* func_atom_expr = c_expr->first_child->next_sibling;
            PLNNRC_CHECK_NODE(atom, build_worldstate_atom(ast, func_atom_expr, type_tag));
            append_child(function_def, atom);

            if (!is_error(atom))
            {
                PLNNRC_CONTINUE(expect_condition(ast, atom->s_expr, !ast.ws_funcs.find(atom->s_expr->token), error_redefinition, worldstate)
                    << atom->s_expr
                    << Location(ast.ws_funcs.find(atom->s_expr->token)));
                ast.ws_funcs.insert(atom->s_expr->token, function_def);
            }

            PLNNRC_CONTINUE(expect_next_token(ast, func_atom_expr, token_return, worldstate));
            PLNNRC_CONTINUE(expect_next_type(ast, func_atom_expr->next_sibling, sexpr::node_list, worldstate));
            sexpr::Node* return_type_expr = func_atom_expr->next_sibling->next_sibling;

            PLNNRC_CHECK_NODE(return_type, build_worldstate_type(ast, return_type_expr, type_tag));
            append_child(function_def, return_type);

            append_child(worldstate, function_def);
        }
        else
        {
            PLNNRC_CHECK_NODE(atom, build_worldstate_atom(ast, c_expr, type_tag));

            if (!is_error(atom))
            {
                PLNNRC_CONTINUE(expect_condition(ast, atom->s_expr, !ast.ws_atoms.find(atom->s_expr->token), error_redefinition, worldstate)
                    << atom->s_expr
                    << Location(ast.ws_atoms.find(atom->s_expr->token)));
                ast.ws_atoms.insert(atom->s_expr->token, atom);
            }

            append_child(worldstate, atom);
        }
    }

    PLNNRC_CHECK(ast.type_tag_to_node.init(ast.ws_types.count() + 1));

    ast.type_tag_to_node[0] = 0;

    for (Id_Table_Values types = ast.ws_types.values(); !types.empty(); types.pop())
    {
        Node* ws_type = types.value();
        int type_tag = annotation<WS_Type_Ann>(ws_type)->type_tag;
        ast.type_tag_to_node[type_tag] = ws_type;
    }

    plnnrc_assert(!ast.type_tag_to_node[0]);

    return worldstate;
}

Node* build_worldstate_atom(Tree& ast, sexpr::Node* s_expr, int& type_tag)
{
    PLNNRC_RETURN(expect_valid_id(ast, s_expr->first_child));
    PLNNRC_CHECK_NODE(atom, ast.make_node(node_atom, s_expr->first_child));

    for (sexpr::Node* t_expr = s_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
    {
        PLNNRC_RETURN(expect_type(ast, t_expr, sexpr::node_list));
        PLNNRC_CHECK_NODE(type_node, build_worldstate_type(ast, t_expr, type_tag));
        append_child(atom, type_node);
    }

    return atom;
}

Node* build_worldstate_type(Tree& ast, sexpr::Node* s_expr, int& type_tag)
{
    PLNNRC_RETURN(expect_child_type(ast, s_expr, sexpr::node_symbol));

    for (sexpr::Node* c_expr = s_expr->first_child; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        PLNNRC_RETURN(expect_type(ast, c_expr, sexpr::node_symbol));
    }

    glue_tokens(s_expr);

    PLNNRC_CHECK_NODE(type, ast.make_node(node_worldstate_type, s_expr));

    Node* type_proto = ast.ws_types.find(s_expr->first_child->token);

    if (!type_proto)
    {
        annotation<WS_Type_Ann>(type)->type_tag = type_tag;
        PLNNRC_CHECK(ast.ws_types.insert(s_expr->first_child->token, type));
        type_tag++;
    }
    else
    {
        annotation<WS_Type_Ann>(type)->type_tag = annotation<WS_Type_Ann>(type_proto)->type_tag;
    }

    return type;
}

}
}
