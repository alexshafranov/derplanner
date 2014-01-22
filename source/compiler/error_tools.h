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

#define PLNNRC_CHECK(EXPR) do { if (!(EXPR)) return 0; } while ((void)(__LINE__==-1), false)

namespace plnnrc {

namespace ast { class tree; }
namespace ast { struct node; }
namespace sexpr { struct node; }

ast::node* report_error(ast::tree& ast, ast::node* parent, compilation_error error_id, sexpr::node* s_expr);

}

#endif
