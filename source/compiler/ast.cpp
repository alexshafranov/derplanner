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
#include "derplanner/compiler/derplanner_assert.h"
#include "derplanner/compiler/derplanner_memory.h"
#include "derplanner/compiler/ast.h"

namespace plnnrc {
namespace ast {

tree::tree()
    : _memory(0)
{
    memset(&_root, 0, sizeof(_root));
    _root.type = node_domain;
}

tree::~tree()
{
}

node* tree::make_node(node_type type)
{
    return 0;
}

void append_child(node* parent, node* child)
{
    plnnrc_assert(parent != 0);
    plnnrc_assert(child != 0);

    child->parent = parent;
    node* first_child = parent->first_child;

    if (first_child)
    {
        node* last_child = first_child->prev_sibling_cyclic;
        last_child->next_sibling = child;
        child->prev_sibling_cyclic = last_child;
        first_child->prev_sibling_cyclic = child;
    }
    else
    {
        parent->first_child = child;
        child->prev_sibling_cyclic = child;
    }
}

void detach_node(node* n)
{
    plnnrc_assert(n != 0);

    node* p = n->parent;

    if (p)
    {
        plnnrc_assert(n->prev_sibling_cyclic != 0);

        node* l = n->prev_sibling_cyclic;
        node* r = n->next_sibling;

        l->next_sibling = r;

        if (r)
        {
            r->prev_sibling_cyclic = l;
        }

        if (p->first_child == n)
        {
            p->first_child = r;
        }
    }
}

}
}
