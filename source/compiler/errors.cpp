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

#include <ctype.h>
#include "formatter.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/s_expression.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/errors.h"

namespace plnnrc {

namespace
{
    const char* error_format_string(compilation_error id)
    {
        switch (id)
        {
        #define PLNNRC_ERROR(ID, STR) case ID: return STR;
        #include "derplanner/compiler/error_tags.inl"
        #undef PLNNRC_ERROR
        default:
            return "<none>";
        };
    }
}

location::location(sexpr::node* s_expr)
    : line(s_expr->line)
    , column(s_expr->column)
{
}

location::location(ast::node* node)
    : line(0)
    , column(0)
{
    if (node && node->s_expr)
    {
        line = node->s_expr->line;
        column = node->s_expr->column;
    }
}

namespace ast {

void format_error(error_ann* annotation, writer& stream)
{
    formatter output(stream);
    output.init(2048);

    const char* format = error_format_string(annotation->id);

    output.put_str("error: ");
    output.put_char('[');
    output.put_int(annotation->line);
    output.put_char(':');
    output.put_int(annotation->column);
    output.put_str("] ");

    while (*format)
    {
        if (*format == '$')
        {
            char digit = *(++format);
            plnnrc_assert(isdigit(digit));
            int slot = digit - '0';
            plnnrc_assert(slot < annotation->argument_count);
            error_argument_type arg_type = annotation->argument_types[slot];
            plnnrc_assert(arg_type != error_argument_none);

            switch (arg_type)
            {
            case error_argument_node_token:
                {
                    sexpr::node* s_expr = annotation->argument_nodes[slot];
                    output.put_str(s_expr->token);
                    break;
                }
            case error_argument_node_location:
                {
                    location arg = annotation->argument_locations[slot];
                    output.put_char('[');
                    output.put_int(arg.line);
                    output.put_char(':');
                    output.put_int(arg.column);
                    output.put_char(']');
                    break;
                }
            default:
                break;
            }
        }
        else
        {
            output.put_char(*format);
        }

        ++format;
    }

    output.put_char('\n');
}

}
}
