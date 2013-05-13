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

#ifndef DERPLANNER_COMPILER_AST_H_
#define DERPLANNER_COMPILER_AST_H_

namespace plnnrc {
namespace ast {

class node
{
public:
    node* parent;
    node* first_child;
    node* next_sibling;
    node* prev_sibling_cyclic;
};

class domain : public node
{
};

class method : public node
{
};

class branch : public node
{
};

enum logical_op_type
{
    logical_op_none = 0,
    logical_op_or,
    logical_op_and,
    logical_op_not,
};

class logical_op : public node
{
public:
    logical_op_type type;
};

class atom : public node
{
};

class term : public node
{
};

}
}

#endif
