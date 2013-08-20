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
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/node_array.h"

namespace plnnrc {

node_array::node_array()
    : _nodes(0)
    , _count(0)
    , _capacity(0)
{
}

node_array::~node_array()
{
    memory::deallocate(_nodes);
}

bool node_array::init(unsigned max_size)
{
    ast::node** nodes = static_cast<ast::node**>(memory::allocate(sizeof(_nodes[0])*max_size));

    if (!nodes)
    {
        return false;
    }

    memory::deallocate(_nodes);
    _nodes = nodes;
    _capacity = max_size;
    _count = 0;

    return true;
}

const ast::node* node_array::operator[](unsigned index) const
{
    plnnrc_assert(index < _capacity);
    return _nodes[index];
}

ast::node*& node_array::operator[](unsigned index)
{
    plnnrc_assert(index < _capacity);
    return _nodes[index];
}

}
