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

#include <string.h>
#include <unittestpp.h>
#include <derplanner/compiler/s_expression.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/ast_build.h>
#include <derplanner/compiler/errors.h>
#include "test_errors.h"

using namespace plnnrc;

namespace test
{
    struct Buffer_Context
    {
        Buffer_Context(size_t bytes)
            : bytes(bytes)
        {
            data = new char[bytes];
        }

        ~Buffer_Context()
        {
            delete [] data;
        }

        size_t bytes;
        char* data;
    };

    void check_error(const char* code, Compilation_Error error_id, int line, int column)
    {
        Buffer_Context buffer(strlen(code) + 1);
        strncpy(buffer.data, code, buffer.bytes);
        sexpr::Tree expr;
        expr.parse(buffer.data);
        ast::Tree tree;
        ast::build_translation_unit(tree, expr.root());
        CHECK(tree.error_node_cache.size());
        ast::Node* error_node = tree.error_node_cache[0];
        ast::Error_Ann* ann = ast::annotation<ast::Error_Ann>(error_node);
        CHECK_EQUAL(error_id, ann->id);
        CHECK_EQUAL(line, ann->line);
        CHECK_EQUAL(column, ann->column);
    }
}
