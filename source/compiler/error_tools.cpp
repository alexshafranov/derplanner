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

void Error_Annotation_Builder::add_argument(sexpr::Node* arg)
{
    if (_node)
    {
        ast::Error_Ann* ann = ast::annotation<ast::Error_Ann>(_node);
        plnnrc_assert(ann->argument_count < ast::max_error_args);
        ann->argument_type[ann->argument_count] = ast::error_argument_node_token;
        ann->argument_node[ann->argument_count] = arg;
        ann->argument_count++;
    }
}

void Error_Annotation_Builder::add_argument(const Location& arg)
{
    if (_node)
    {
        ast::Error_Ann* ann = ast::annotation<ast::Error_Ann>(_node);
        plnnrc_assert(ann->argument_count < ast::max_error_args);
        ann->argument_type[ann->argument_count] = ast::error_argument_node_location;
        ann->argument_location[ann->argument_count++] = arg;
        ann->argument_count++;
    }
}

void Error_Annotation_Builder::add_argument(const char* arg)
{
    if (_node)
    {
        ast::Error_Ann* ann = ast::annotation<ast::Error_Ann>(_node);
        plnnrc_assert(ann->argument_count < ast::max_error_args);
        ann->argument_type[ann->argument_count] = ast::error_argument_node_string;
        ann->argument_string[ann->argument_count++] = arg;
        ann->argument_count++;
    }
}

void Error_Annotation_Builder::add_argument(int arg)
{
    if (_node)
    {
        ast::Error_Ann* ann = ast::annotation<ast::Error_Ann>(_node);
        plnnrc_assert(ann->argument_count < ast::max_error_args);
        ann->argument_type[ann->argument_count] = ast::error_argument_selection;
        ann->argument_selection[ann->argument_count++] = arg;
        ann->argument_count++;
    }
}

Error_Annotation_Builder emit_error(ast::Tree& ast, Compilation_Error error_id, sexpr::Node* s_expr, bool past_expr_location)
{
    return emit_error(ast, 0, error_id, s_expr, past_expr_location);
}

Error_Annotation_Builder emit_error(ast::Tree& ast, ast::Node* parent, Compilation_Error error_id, sexpr::Node* s_expr, bool past_expr_location)
{
    ast::Node* error = ast.make_node(ast::node_error, s_expr);

    if (error)
    {
        if (parent)
        {
            append_child(parent, error);
        }

        if (ast.error_node_cache.size() < ast.error_node_cache.capacity())
        {
            ast.error_node_cache.append(error);
        }

        ast::Error_Ann* annotation = ast::annotation<ast::Error_Ann>(error);
        annotation->id = error_id;
        annotation->line = s_expr->line;

        if (past_expr_location)
        {
            annotation->line = s_expr->line_end;
            annotation->column = s_expr->column_end;
        }
        else
        {
            annotation->column = s_expr->column;
        }
    }

    return Error_Annotation_Builder(error);
}

Error_Annotation_Builder expect_type(ast::Tree& ast, sexpr::Node* s_expr, sexpr::Node_Type expected, ast::Node* parent)
{
    if (s_expr->type != expected)
    {
        return emit_error(ast, parent, error_expected_type, s_expr) << static_cast<int>(expected);
    }

    return 0;
}

Error_Annotation_Builder expect_child_type(ast::Tree& ast, sexpr::Node* s_expr, sexpr::Node_Type expected, ast::Node* parent)
{
    if (!s_expr->first_child)
    {
        return emit_error(ast, parent, error_expected_type, s_expr) << static_cast<int>(expected);
    }

    if (s_expr->first_child->type != expected)
    {
        return emit_error(ast, parent, error_expected_type, s_expr) << static_cast<int>(expected);
    }

    return 0;
}

Error_Annotation_Builder expect_next_type(ast::Tree& ast, sexpr::Node* s_expr, sexpr::Node_Type expected, ast::Node* parent)
{
    if (!s_expr->next_sibling)
    {
        return emit_error(ast, parent, error_expected_type, s_expr, true) << static_cast<int>(expected);
    }

    if (s_expr->next_sibling->type != expected)
    {
        return emit_error(ast, parent, error_expected_type, s_expr->next_sibling) << static_cast<int>(expected);
    }

    return 0;
}

Error_Annotation_Builder expect_next_token(ast::Tree& ast, sexpr::Node* s_expr, Str_Ref expected, ast::Node* parent)
{
    if (!s_expr->next_sibling)
    {
        return emit_error(ast, parent, error_expected_token, s_expr, true) << (expected.str);
    }

    if (!is_token(s_expr->next_sibling, expected))
    {
        return emit_error(ast, parent, error_expected_token, s_expr->next_sibling) << (expected.str);
    }

    return 0;
}

Error_Annotation_Builder expect_valid_id(ast::Tree& ast, sexpr::Node* s_expr, ast::Node* parent)
{
    plnnrc_assert(s_expr);

    if (!is_valid_id(s_expr->token))
    {
        return emit_error(ast, parent, error_invalid_id, s_expr) << s_expr;
    }

    return 0;
}

Error_Annotation_Builder expect_condition(ast::Tree& ast, sexpr::Node* s_expr, bool condition, Compilation_Error error_id, ast::Node* parent)
{
    if (!condition)
    {
        return emit_error(ast, parent, error_id, s_expr);
    }

    return 0;
}

Error_Annotation_Builder replace_with_error(ast::Tree& ast, ast::Node* Node, Compilation_Error error_id)
{
    ast::Node* error_node = emit_error(ast, error_id, Node->s_expr);

    Node->type = error_node->type;
    Node->annotation = error_node->annotation;

    return Error_Annotation_Builder(Node);
}

Error_Annotation_Builder replace_with_error_if(bool condition, ast::Tree& ast, ast::Node* Node, Compilation_Error error_id)
{
    if (condition)
    {
        return replace_with_error(ast, Node, error_id);
    }

    return 0;
}

}
