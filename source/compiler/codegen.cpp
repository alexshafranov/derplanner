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

#include "derplanner/compiler/io.h"
#include "derplanner/compiler/codegen.h"

using namespace plnnrc;

void plnnrc::init(Codegen& state, ast::Root* tree)
{
    state.tree = tree;
}

void plnnrc::generate_header(Codegen& state, const char* header_guard, Writer* output)
{
    plnnrc::init(state.fmtr, "  ", "\n", output);
    ast::Root* tree = state.tree;
    ast::Domain* domain = tree->domain;
    Formatter& fmtr = state.fmtr;

    Token_Value domain_name = domain->name;
    writeln(fmtr, "#ifndef %s", header_guard);
    writeln(fmtr, "#define %s", header_guard);
    writeln(fmtr, "#pragma once");
    newline(fmtr);
    writeln(fmtr, "#include \"derplanner/runtime/types.h\"");
    newline(fmtr);
    writeln(fmtr, "#ifndef PLNNR_DOMAIN_API");
    writeln(fmtr, "#define PLNNR_DOMAIN_API");
    writeln(fmtr, "#endif");
    newline(fmtr);
    writeln(fmtr, "extern \"C\" PLNNR_DOMAIN_API void %n_init_domain_info();", domain_name);
    writeln(fmtr, "extern \"C\" PLNNR_DOMAIN_API const plnnr::Domain_Info* %n_get_domain_info();", domain_name);
    newline(fmtr);
    writeln(fmtr, "#endif");
    flush(fmtr);
}
