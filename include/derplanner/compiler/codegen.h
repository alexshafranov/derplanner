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

#ifndef DERPLANNER_COMPILER_CODEGEN_H_
#define DERPLANNER_COMPILER_CODEGEN_H_

#include "derplanner/compiler/types.h"

namespace plnnrc {

/// Codegen

// initialize code generator state.
void init(Codegen* self, ast::Root* tree, Memory_Stack* scratch);

// writes domain header code. `header_guard` string is used as macro include guard.
void generate_header(Codegen* self, const char* header_guard, Writer* output);
// writes domain source code.
void generate_source(Codegen* self, const char* domain_header, Writer* output);

}

#endif
