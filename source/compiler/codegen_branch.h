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

#ifndef DERPLANNER_COMPILER_CODEGEN_BRANCH_H_
#define DERPLANNER_COMPILER_CODEGEN_BRANCH_H_

namespace plnnrc {

namespace ast { struct Node; }
namespace ast { class Tree; }
class Formatter;

void generate_branch_expands(ast::Tree& ast, ast::Node* domain, Formatter& output);

void generate_operator_effects(ast::Tree& ast, ast::Node* method, ast::Node* task_atom, Formatter& output);
void generate_effects_add(ast::Node* effects, Formatter& output);
void generate_effects_delete(ast::Node* effects, Formatter& output);
void generate_operator_task(ast::Tree& ast, ast::Node* method, ast::Node* task_atom, Formatter& output);
void generate_method_task(ast::Tree& ast, ast::Node* method, ast::Node* task_atom, Formatter& output);

}

#endif
