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

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "tokens.h"
#include "ast_build_tools.h"
#include "tree_tools.h"
#include "ast_worldstate.h"
#include "ast_domain.h"
#include "ast_infer.h"
#include "ast_annotate.h"
#include "derplanner/compiler/ast_build.h"

namespace plnnrc {
namespace ast {

bool build_translation_unit(tree& ast, sexpr::node* s_expr)
{
    for (sexpr::node* c_expr = s_expr->first_child; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        if (c_expr->type == sexpr::node_list)
        {
            if (is_token(c_expr->first_child, token_worldstate))
            {
                node* worldstate = build_worldstate(ast, c_expr);
                PLNNRC_CHECK(worldstate);
                append_child(ast.root(), worldstate);
                continue;
            }

            if (is_token(c_expr->first_child, token_domain))
            {
                node* domain = build_domain(ast, c_expr);
                PLNNRC_CHECK(domain);
                append_child(ast.root(), domain);

                infer_types(ast);
                annotate(ast);

                continue;
            }

            report_error(ast, ast.root(), (compilation_error)0, c_expr);
        }
    }

    return true;
}

}
}
