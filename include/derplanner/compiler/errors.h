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

#ifndef DERPLANNER_COMPILER_ERRORS_H_
#define DERPLANNER_COMPILER_ERRORS_H_

namespace plnnrc {

class Writer;
namespace sexpr { struct Node; }
namespace ast { struct Node; }

enum Compilation_Error
{
    error_none = 0,

    #define PLNNRC_ERROR(ID, DESC) ID,
    #include "derplanner/compiler/error_tags.inl"
    #undef PLNNRC_ERROR
};

class Location
{
public:
    Location(int line=-1, int column=-1)
        : line(line)
        , column(column)
    {
    }

    Location(sexpr::Node* s_expr);

    Location(ast::Node* Node);

    Location(const Location& other)
        : line(other.line)
        , column(other.column)
    {
    }

    Location& operator=(const Location& other)
    {
        line = other.line;
        column = other.column;
        return *this;
    }

    int line;
    int column;
};

namespace ast {

enum Error_Argument_Type
{
    error_argument_none = 0,
    error_argument_node_token,
    error_argument_node_location,
    error_argument_node_string,
    error_argument_selection,
};

enum { max_error_args = 4 };

struct Error_Ann
{
    Compilation_Error id;
    int line;
    int column;
    int argument_count;
    Error_Argument_Type argument_type[max_error_args];
    sexpr::Node* argument_node[max_error_args];
    Location argument_location[max_error_args];
    const char* argument_string[max_error_args];
    int argument_selection[max_error_args];
};

void format_error(Error_Ann* annotation, Writer& stream);

}
}

#endif
