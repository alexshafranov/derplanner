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

#ifndef DERPLANNER_S_EXPRESSION_H_
#define DERPLANNER_S_EXPRESSION_H_

#include <stddef.h>

namespace derplanner {
namespace s_expression {

typedef void* (*allocate_function)(size_t size);
typedef void* (*deallocate_function)(void* ptr);
void set_custom_allocation(allocate_function alloc, deallocate_function free);

enum node_type
{
    node_error = 0,
    node_list,
    node_symbol,
    node_number,
};

struct node
{
    node_type type;
    int line;
    int column;
    int text_begin_offset;
    int text_end_offset;
    node* first_child;
    node* sibling;
};

class tree
{
public:
    tree();
    ~tree();
    node* root;
private:
    void* _memory;
};

tree parse(const char* text);

}
}

#endif

// 
// s_expression::tree s_expr = s_expression::parse("(10 20 40 30)");
//

// derplanner
//      deps
//          unittestpp
//      tests
//          ...
//      include
//          derplanner
//              compiler
//                  s_expression.h
//                  ast.h
//              runtime
//                  ...
//      source
//          derplanner
//              compiler
//                  s_expression.cpp
//                  ast.cpp
//              runtime
//                  ...
