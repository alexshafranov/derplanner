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

#ifndef DERPLANNER_COMPILER_BUILD_TOOLS_H_
#define DERPLANNER_COMPILER_BUILD_TOOLS_H_

#include "derplanner/compiler/s_expression.h"
#include "ast_tools.h"
#include "tokens.h"

#define PLNNRC_CHECK(EXPR) do { if (!(EXPR)) return 0; } while ((void)(__LINE__==-1), false)

#define PLNNRC_CHECK_NODE(NODE, ALLOC_EXPR) ::plnnrc::ast::node* NODE = (ALLOC_EXPR); PLNNRC_CHECK(NODE)

#define PLNNRC_RETURN(EXPECT_EXPR)                          \
    do                                                      \
    {                                                       \
        ::plnnrc::ast::node* error_node = (EXPECT_EXPR);    \
        if (error_node) return error_node;                  \
    }                                                       \
    while ((void)(__LINE__==-1), false)                     \

#define PLNNRC_CONTINUE(EXPECT_EXPR) if ((EXPECT_EXPR)) continue;
#define PLNNRC_SKIP_ERROR_NODE(NODE) PLNNRC_CONTINUE(((NODE)->type == ::plnnrc::ast::node_error))
#define PLNNRC_SKIP_SUBTREE_WITH_ERRORS(NODE) PLNNRC_CONTINUE(::plnnrc::ast::find_descendant((NODE), ::plnnrc::ast::node_error) != 0)

namespace plnnrc {

namespace ast { class tree; }
namespace ast { struct node; }

ast::node* expect_type(ast::tree& ast, sexpr::node* s_expr, sexpr::node_type expected, ast::node* parent=0);
ast::node* expect_child_type(ast::tree& ast, sexpr::node* s_expr, sexpr::node_type expected, ast::node* parent=0);
ast::node* expect_next_type(ast::tree& ast, sexpr::node* s_expr, sexpr::node_type expected, ast::node* parent=0);
ast::node* expect_next_token(ast::tree& ast, sexpr::node* s_expr, str_ref expected, ast::node* parent=0);
ast::node* expect_valid_id(ast::tree& ast, sexpr::node* s_expr, ast::node* parent=0);
ast::node* expect_condition(ast::tree& ast, sexpr::node* s_expr, bool condition, compilation_error error_id, ast::node* parent=0);

ast::node* replace_with_error(ast::tree& ast, ast::node* node, compilation_error error_id);
ast::node* replace_with_error_if(bool condition, ast::tree& ast, ast::node* node, compilation_error error_id);

ast::node* emit_error(ast::tree& ast, compilation_error error_id, sexpr::node* s_expr, bool past_token_locaion=false);
ast::node* emit_error(ast::tree& ast, ast::node* parent, compilation_error error_id, sexpr::node* s_expr, bool past_token_locaion=false);

}

#endif
