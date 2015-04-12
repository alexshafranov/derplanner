//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
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

#ifndef DERPLANNER_COMPILER_TRANSFORMS_H_
#define DERPLANNER_COMPILER_TRANSFORMS_H_

#include "derplanner/compiler/types.h"

namespace plnnrc {

// create AST.
void init(ast::Root& root);

// destroy AST.
void destroy(ast::Root& root);

// walks the tree to build `ast::Task` and `ast::Fact_Type` name lookups.
void build_lookups(ast::Root& root);

/// `create_*` functions for `ast` node types.

ast::World*         create_world(ast::Root& tree);
ast::Fact_Type*     create_fact_type(ast::Root& tree, ast::Fact_Type* previous);
ast::Fact_Param*    create_fact_param(ast::Root& tree, ast::Fact_Param* previous);
ast::Domain*        create_domain(ast::Root& tree);
ast::Task*          create_task(ast::Root& tree, const Token_Value& name);
ast::Task_Param*    create_task_param(ast::Root& tree, const Token_Value& name, ast::Task_Param* previous);
ast::Case*          create_case(ast::Root& tree);
ast::Expr*          create_expr(ast::Root& tree, Token_Type type);

// lookup `ast::Task` node by name (`token_value`).
ast::Task*      get_task(ast::Root& tree, const Token_Value& token_value);
// lookup `ast::Fact_Type` node by name (`token_value`).
ast::Fact_Type* get_fact(ast::Root& tree, const Token_Value& token_value);

// writes formatted Abstract-Syntax-Tree to `output`.
void debug_output_ast(const ast::Root& root, Writer* output);

/// ast::Expr

// make node `child` the last child of node `parent`.
void        append_child(ast::Expr* parent, ast::Expr* child);
// make `child` the next sibling of `after`.
void        insert_child(ast::Expr* after, ast::Expr* child);
// unparent `node` from it's current parent.
void        unparent(ast::Expr* node);
// returns the next node in pre-order (visit node then visit it's children) traversal.
ast::Expr*  preorder_next(const ast::Expr* root, ast::Expr* current);

/// Expression transformations.

// converts expression `root` to Disjunctive-Normal-Form.
ast::Expr* convert_to_dnf(ast::Root& tree, ast::Expr* root);

// figure out types of parameters and variables.
void infer_types(ast::Root& tree);

}

#endif
