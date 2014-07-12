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

#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/s_expression.h"
#include "ast_tools.h"
#include "tokens.h"

#define PLNNRC_CHECK(EXPR) do { if (!(EXPR)) return 0; } while ((void)(__LINE__==-1), false)

#define PLNNRC_CHECK_NODE(NODE, ALLOC_EXPR) ::plnnrc::ast::Node* NODE = (ALLOC_EXPR); PLNNRC_CHECK(NODE)

#define PLNNRC_RETURN(EXPECT_EXPR)                          \
    do                                                      \
    {                                                       \
        ::plnnrc::ast::Node* error_node = (EXPECT_EXPR);    \
        if (error_node) return error_node;                  \
    }                                                       \
    while ((void)(__LINE__==-1), false)                     \

#define PLNNRC_CONTINUE(EXPECT_EXPR) if ((EXPECT_EXPR)) continue;
#define PLNNRC_BREAK(EXPECT_EXPR) if ((EXPECT_EXPR)) break;
#define PLNNRC_SKIP_ERROR_NODE(NODE) PLNNRC_CONTINUE(::plnnrc::ast::is_error((NODE)))
#define PLNNRC_SKIP_SUBTREE_WITH_ERRORS(NODE) PLNNRC_CONTINUE(::plnnrc::ast::find_descendant((NODE), ::plnnrc::ast::node_error) != 0)

namespace plnnrc {

namespace ast { class Tree; }
namespace ast { struct Node; }

class Error_Annotation_Builder
{
public:
    Error_Annotation_Builder(ast::Node* Node)
        : _node(Node)
    {
    }

    operator ast::Node*() const { return _node; }

    void add_argument(sexpr::Node* arg);
    void add_argument(const Location& arg);
    void add_argument(const char* arg);
    void add_argument(int arg);

private:
    ast::Node* _node;
};

template <typename T>
Error_Annotation_Builder operator<<(Error_Annotation_Builder builder, const T& t)
{
    builder.add_argument(t);
    return builder;
}

Error_Annotation_Builder expect_type(ast::Tree& ast, sexpr::Node* s_expr, sexpr::Node_Type expected, ast::Node* parent=0);
Error_Annotation_Builder expect_child_type(ast::Tree& ast, sexpr::Node* s_expr, sexpr::Node_Type expected, ast::Node* parent=0);
Error_Annotation_Builder expect_next_type(ast::Tree& ast, sexpr::Node* s_expr, sexpr::Node_Type expected, ast::Node* parent=0);
Error_Annotation_Builder expect_next_token(ast::Tree& ast, sexpr::Node* s_expr, Str_Ref expected, ast::Node* parent=0);
Error_Annotation_Builder expect_valid_id(ast::Tree& ast, sexpr::Node* s_expr, ast::Node* parent=0);
Error_Annotation_Builder expect_condition(ast::Tree& ast, sexpr::Node* s_expr, bool condition, Compilation_Error error_id, ast::Node* parent=0);

Error_Annotation_Builder replace_with_error(ast::Tree& ast, ast::Node* Node, Compilation_Error error_id);
Error_Annotation_Builder replace_with_error_if(bool condition, ast::Tree& ast, ast::Node* Node, Compilation_Error error_id);

Error_Annotation_Builder emit_error(ast::Tree& ast, Compilation_Error error_id, sexpr::Node* s_expr, bool past_expr_location=false);
Error_Annotation_Builder emit_error(ast::Tree& ast, ast::Node* parent, Compilation_Error error_id, sexpr::Node* s_expr, bool past_expr_location=false);

}

#endif
