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
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/errors.h"
#include "formatter.h"
#include "tree_tools.h"
#include "error_tools.h"

namespace plnnrc {

ast::node* report_error(ast::tree& ast, compilation_error error_id, sexpr::node* s_expr)
{
    return report_error(ast, 0, error_id, s_expr); 
}

ast::node* report_error(ast::tree& ast, ast::node* parent, compilation_error error_id, sexpr::node* s_expr)
{
    ast::node* error = ast.make_node(ast::node_error, s_expr);

    if (error)
    {
        if (parent)
        {
            append_child(parent, error);
        }

        if (ast.error_nodes.size() < ast.error_nodes.capacity())
        {
            ast.error_nodes.append(error);
        }

        ast::annotation<ast::error_ann>(error)->id = error_id;
    }

    return error;
}

ast::node* expect_type(ast::tree& ast, sexpr::node* s_expr, sexpr::node_type expected, ast::node* parent)
{
    if (s_expr->type != expected)
    {
        return report_error(ast, parent, error_expected, s_expr);
    }

    return 0;
}

ast::node* expect_child_type(ast::tree& ast, sexpr::node* s_expr, sexpr::node_type expected, ast::node* parent)
{
    if (!s_expr->first_child)
    {
        return report_error(ast, parent, error_expected, s_expr);
    }

    if (s_expr->first_child->type != expected)
    {
        return report_error(ast, parent, error_expected, s_expr);
    }

    return 0;
}

ast::node* expect_next_type(ast::tree& ast, sexpr::node* s_expr, sexpr::node_type expected, ast::node* parent)
{
    if (!s_expr->next_sibling)
    {
        return report_error(ast, parent, error_expected, s_expr);
    }

    if (s_expr->next_sibling->type != expected)
    {
        return report_error(ast, parent, error_expected, s_expr->next_sibling);
    }

    return 0;
}

ast::node* expect_next_token(ast::tree& ast, sexpr::node* s_expr, str_ref expected, ast::node* parent)
{
    if (!s_expr->next_sibling)
    {
        return report_error(ast, parent, error_expected, s_expr);
    }

    if (!is_token(s_expr->next_sibling, expected))
    {
        return report_error(ast, parent, error_expected, s_expr->next_sibling);
    }

    return 0;
}

ast::node* expect_valid_id(ast::tree& ast, sexpr::node* s_expr, ast::node* parent)
{
    plnnrc_assert(s_expr);

    if (!is_valid_id(s_expr->token))
    {
        return report_error(ast, parent, error_invalid_id, s_expr);
    }

    return 0;
}

ast::node* expect_condition(ast::tree& ast, sexpr::node* s_expr, bool condition, compilation_error error_id, ast::node* parent)
{
    if (!condition)
    {
        return report_error(ast, parent, error_id, s_expr);
    }

    return 0;
}

}
