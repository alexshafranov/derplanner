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

#ifndef DERPLANNER_COMPILER_CODEGEN_H_
#define DERPLANNER_COMPILER_CODEGEN_H_

namespace plnnrc {

namespace ast { class Tree; }
class Writer;

struct Codegen_Options
{
    const char* tab;
    const char* newline;
    const char* include_guard;
    const char* header_file_name;
    const char* custom_header;
    bool runtime_atom_names;
    bool runtime_task_names;
    bool enable_reflection;
};

bool generate_header(ast::Tree& ast, Writer& output, Codegen_Options options);
bool generate_source(ast::Tree& ast, Writer& output, Codegen_Options options);

}

#endif
