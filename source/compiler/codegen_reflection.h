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

#ifndef DERPLANNER_COMPILER_CODEGEN_REFLECTION_H_
#define DERPLANNER_COMPILER_CODEGEN_REFLECTION_H_

namespace plnnrc {

namespace ast { struct Node; }
namespace ast { class Tree; }
class Formatter;
class Paste_Func;

void generate_worldstate_reflectors(ast::Tree& ast, ast::Node* worldstate, Formatter& output);
void generate_domain_reflectors(ast::Tree& ast, ast::Node* domain, Formatter& output);
void generate_atom_reflector(ast::Tree& ast, ast::Node* atom, Paste_Func* paste_namespace,
                             const char* name_function, const char* tuple_struct_postfix, const char* tuple_id_prefix, Formatter& output);
void generate_task_type_dispatcher(ast::Tree& ast, ast::Node* domain, Formatter& output);

}

#endif
