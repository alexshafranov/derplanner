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

Node_Array::Node_Array()
    : _nodes(0)
    , _size(0)
    , _capacity(0)
{
}

Node_Array::~Node_Array()
{
    memory::deallocate(_nodes);
}

bool Node_Array::init(unsigned max_size)
{
    ast::Node** nodes = static_cast<ast::Node**>(memory::allocate(sizeof(_nodes[0])*max_size));

    if (!nodes)
    {
        return false;
    }

    memory::deallocate(_nodes);

    _nodes = nodes;
    _size = 0;
    _capacity = max_size;

    return true;
}

}
