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

#include <string>
#include <unittestpp.h>
#include <derplanner/compiler/assert.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/domain.h>

using namespace plnnrc;

namespace
{
    TEST(empty_domain)
    {
        char buffer[] = "(:domain)";
        sexpr::tree expr;
        expr.parse(buffer);
        ast::tree tree;
        ast::node* actual = ast::build_domain(tree, expr.root()->first_child);
        CHECK(actual);
        CHECK(!actual->first_child);
    }
}

//         char buffer[] = 
// "(:domain                                                   "
// "    (:method (root)                                        "
// "        ((start ?s) (finish ?f))                           "
// "        ((travel ?s ?f))                                   "
// "    )                                                      "
// "                                                           "
// "    (:method (travel ?x ?y)                                "
// "        ((short_distance ?x ?y))                           "
// "        ((!ride_taxi ?x ?y))                               "
// "                                                           "
// "        ((long_distance ?x ?y))                            "
// "        ((travel_by_air ?x ?y))                            "
// "    )                                                      "
// "                                                           "
// "    (:method (travel_by_air ?x ?y)                         "
// "        ((airport ?x ?ax) (airport ?y ?ay))                "
// "        ((travel ?x ?ax) (!fly ?ax ?ay) (travel ?y ?ay))   "
// "    )                                                      "
// ")                                                          ";
