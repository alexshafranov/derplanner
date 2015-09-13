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
#include "error_tools.h"
#include "tree_tools.h"
#include "ast_worldstate.h"
#include "ast_domain.h"
#include "ast_infer.h"
#include "ast_annotate.h"
#include "derplanner/compiler/ast_build.h"

namespace plnnrc {
namespace ast {

bool build_translation_unit(Tree& ast, sexpr::Node* s_expr)
{
    sexpr::Node* worldstate_expr = 0;
    sexpr::Node* domain_expr = 0;

    for (sexpr::Node* c_expr = s_expr->first_child; c_expr != 0; c_expr = c_expr->next_sibling)
    {
        if (sexpr::is_list(c_expr))
        {
            if (is_token(c_expr->first_child, token_worldstate))
            {
                PLNNRC_CONTINUE(expect_condition(ast, c_expr, !worldstate_expr, error_multiple_definitions, ast.root()) << 0);
                worldstate_expr = c_expr;
                continue;
            }

            if (is_token(c_expr->first_child, token_domain))
            {
                PLNNRC_CONTINUE(expect_condition(ast, c_expr, !domain_expr, error_multiple_definitions, ast.root()) << 1);
                domain_expr = c_expr;
                continue;
            }
        }

        emit_error(ast, ast.root(), error_unexpected, c_expr);
    }

    if (worldstate_expr)
    {
        PLNNRC_CHECK_NODE(Worldstate, build_worldstate(ast, worldstate_expr));
        append_child(ast.root(), Worldstate);
    }

    if (domain_expr)
    {
        PLNNRC_CHECK_NODE(domain, build_domain(ast, domain_expr));
        append_child(ast.root(), domain);

        seed_types(ast);

        if (!ast.error_node_cache.size())
        {
            infer_types(ast);
        }

        if (!ast.error_node_cache.size())
        {
            annotate(ast);
        }
    }

    return true;
}

}
}
