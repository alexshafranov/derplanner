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

#ifndef DERPLANNER_COMPILER_NODE_ARRAY_
#define DERPLANNER_COMPILER_NODE_ARRAY_

#include "derplanner/compiler/assert.h"

namespace plnnrc {

namespace ast
{
    struct node;
}

class node_array
{
public:
    node_array();
    ~node_array();

    bool init(unsigned max_size);

    const ast::node* operator[](unsigned index) const;
    ast::node*& operator[](unsigned index);

    void append(ast::node* node);

    unsigned size() const { return _size; }
    unsigned capacity() const { return _capacity; }

private:
    node_array(const node_array&);
    const node_array& operator=(const node_array&);

    ast::node** _nodes;
    unsigned    _size;
    unsigned    _capacity;
};

inline const ast::node* node_array::operator[](unsigned index) const
{
    plnnrc_assert(index < _capacity);
    return _nodes[index];
}

inline ast::node*& node_array::operator[](unsigned index)
{
    plnnrc_assert(index < _capacity);
    return _nodes[index];
}

inline void node_array::append(ast::node* node)
{
    plnnrc_assert(_size < _capacity);
    _nodes[_size++] = node;
}

}

#endif
