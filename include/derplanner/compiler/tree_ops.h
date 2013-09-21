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

#ifndef DERPLANNER_COMPILER_TREE_OPS_H_
#define DERPLANNER_COMPILER_TREE_OPS_H_

#include "derplanner/compiler/assert.h"

namespace plnnrc {

template <typename node>
bool is_last(const node* child)
{
    return child == child->parent->first_child->prev_sibling_cyclic;
}

template <typename node>
void append_child(node* parent, node* child)
{
    plnnrc_assert(parent != 0);
    plnnrc_assert(child != 0);

    child->parent = parent;
    child->prev_sibling_cyclic = 0;
    child->next_sibling = 0;

    node* first_child = parent->first_child;

    if (first_child)
    {
        node* last_child = first_child->prev_sibling_cyclic;
        plnnrc_assert(last_child != 0);
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

template <typename node>
void prepend_child(node* parent, node* child)
{
    plnnrc_assert(parent != 0);
    plnnrc_assert(child != 0);

    child->parent = parent;
    child->prev_sibling_cyclic = child;
    child->next_sibling = parent->first_child;

    if (parent->first_child)
    {
        parent->first_child->prev_sibling_cyclic = child;
    }

    parent->first_child = child;
}

template <typename node>
void insert_child(node* after, node* child)
{
    plnnrc_assert(after != 0);
    plnnrc_assert(child != 0);
    plnnrc_assert(after->parent != 0);

    node* l = after;
    node* r = after->next_sibling;
    node* p = after->parent;

    l->next_sibling = child;

    if (r)
    {
        r->prev_sibling_cyclic = child;
    }
    else
    {
        p->first_child->prev_sibling_cyclic = child;
    }

    child->prev_sibling_cyclic = l;
    child->next_sibling = r;
    child->parent = p;
}

template <typename node>
void detach_node(node* n)
{
    plnnrc_assert(n != 0);

    node* p = n->parent;
    node* n_next = n->next_sibling;
    node* n_prev = n->prev_sibling_cyclic;

    plnnrc_assert(p != 0);
    plnnrc_assert(n_prev != 0);

    if (n_next)
    {
        n_next->prev_sibling_cyclic = n_prev;
    }
    else
    {
        p->first_child->prev_sibling_cyclic = n_prev;
    }

    if (n_prev->next_sibling)
    {
        n_prev->next_sibling = n_next;
    }
    else
    {
        p->first_child = n_next;
    }

    n->parent = 0;
    n->next_sibling = 0;
    n->prev_sibling_cyclic = 0;
}

template <typename node>
node* preorder_traversal_next(const node* root, node* current)
{
    node* n = current;

    if (n->first_child)
    {
        return n->first_child;
    }

    while (n != root && !n->next_sibling) { n = n->parent; }

    if (n == root)
    {
        return 0;
    }

    return n->next_sibling;
}

}

#endif
