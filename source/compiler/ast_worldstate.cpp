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

node* build_worldstate(tree& ast, sexpr::node* s_expr)
{
    node* worldstate = ast.make_node(node_worldstate, s_expr);
    PLNNRC_CHECK(worldstate);

    PLNNRC_EXPECT_NEXT(ast, s_expr->first_child, sexpr::node_list);
    sexpr::node* name_list_expr = s_expr->first_child->next_sibling;

    node* worldstate_namespace = build_namespace(ast, name_list_expr);
    PLNNRC_CHECK(worldstate_namespace);
    append_child(worldstate, worldstate_namespace);

    unsigned total_atom_count = 0;
    unsigned total_func_count = 0;

    for (sexpr::node* c_expr = name_list_expr->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        plnnrc_assert(c_expr->type == sexpr::node_list);
        plnnrc_assert(c_expr->first_child && c_expr->first_child->type == sexpr::node_symbol);

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

    for (sexpr::node* c_expr = name_list_expr->next_sibling; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        if (is_token(c_expr->first_child, token_function))
        {
            node* function_def = ast.make_node(node_function, c_expr);
            PLNNRC_CHECK(function_def);

            sexpr::node* func_atom_expr = c_expr->first_child->next_sibling;
            node* atom = build_worldstate_atom(ast, func_atom_expr, type_tag);
            PLNNRC_CHECK(atom);
            append_child(function_def, atom);

            ast.ws_funcs.insert(atom->s_expr->token, function_def);

            plnnrc_assert(is_token(func_atom_expr->next_sibling, token_return));

            sexpr::node* return_type_expr = func_atom_expr->next_sibling->next_sibling;
            node* return_type = build_worldstate_type(ast, return_type_expr, type_tag);
            PLNNRC_CHECK(return_type);
            append_child(function_def, return_type);

            append_child(worldstate, function_def);
        }
        else
        {
            node* atom = build_worldstate_atom(ast, c_expr, type_tag);
            PLNNRC_CHECK(atom);
            ast.ws_atoms.insert(atom->s_expr->token, atom);
            append_child(worldstate, atom);
        }
    }

    PLNNRC_CHECK(ast.type_tag_to_node.init(ast.ws_types.count() + 1));

    ast.type_tag_to_node[0] = 0;

    for (id_table_values types = ast.ws_types.values(); !types.empty(); types.pop())
    {
        node* ws_type = types.value();
        int type_tag = annotation<ws_type_ann>(ws_type)->type_tag;
        ast.type_tag_to_node[type_tag] = ws_type;
    }

    plnnrc_assert(!ast.type_tag_to_node[0]);

    return worldstate;
}

node* build_worldstate_atom(tree& ast, sexpr::node* s_expr, int& type_tag)
{
    node* atom = ast.make_node(node_atom, s_expr->first_child);
    PLNNRC_CHECK(atom);

    for (sexpr::node* t_expr = s_expr->first_child->next_sibling; t_expr != 0; t_expr = t_expr->next_sibling)
    {
        node* type_node = build_worldstate_type(ast, t_expr, type_tag);
        PLNNRC_CHECK(type_node);
        append_child(atom, type_node);
    }

    return atom;
}

node* build_worldstate_type(tree& ast, sexpr::node* s_expr, int& type_tag)
{
    glue_tokens(s_expr);

    node* type = ast.make_node(node_worldstate_type, s_expr);
    PLNNRC_CHECK(type);

    node* type_proto = ast.ws_types.find(s_expr->first_child->token);

    if (!type_proto)
    {
        annotation<ws_type_ann>(type)->type_tag = type_tag;

        if (!ast.ws_types.insert(s_expr->first_child->token, type))
        {
            return 0;
        }

        type_tag++;
    }
    else
    {
        annotation<ws_type_ann>(type)->type_tag = annotation<ws_type_ann>(type_proto)->type_tag;
    }

    return type;
}

}
}
