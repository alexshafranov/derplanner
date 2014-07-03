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

#ifndef DERPLANNER_COMPILER_CODEGEN_PRECONDITION_H_
#define DERPLANNER_COMPILER_CODEGEN_PRECONDITION_H_

namespace plnnrc {

namespace ast { struct node; }
namespace ast { class tree; }
class formatter;

void generate_preconditions(ast::tree& ast, ast::node* domain, formatter& output);

void generate_precondition_state(ast::tree& ast, ast::node* root, unsigned branch_index, formatter& output);
void generate_precondition_next(ast::tree& ast, ast::node* root, unsigned branch_index, formatter& output);

void generate_precondition_satisfier(ast::tree& ast, ast::node* root, formatter& output, int yield_label);
void generate_conjunctive_clause(ast::tree& ast, ast::node* root, formatter& output, int yield_label);
void generate_literal_chain(ast::tree& ast, ast::node* root, formatter& output, int yield_label);
void generate_literal_chain_call_term(ast::tree& ast, ast::node* root, ast::node* atom, formatter& output, int yield_label);
void generate_literal_chain_comparison(ast::tree& ast, ast::node* root, ast::node* atom, formatter& output, int yield_label);

}

#endif
