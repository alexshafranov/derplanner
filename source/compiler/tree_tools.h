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

#ifndef DERPLANNER_COMPILER_TREE_TOOLS_H_
#define DERPLANNER_COMPILER_TREE_TOOLS_H_

#include "derplanner/compiler/assert.h"

namespace plnnrc {

template <typename Node>
bool is_first(const Node* child)
{
    return child == child->parent->first_child;
}

template <typename Node>
bool is_last(const Node* child)
{
    return child == child->parent->first_child->prev_sibling_cyclic;
}

template <typename Node>
void append_child(Node* parent, Node* child)
{
    plnnrc_assert(parent != 0);
    plnnrc_assert(child != 0);

    child->parent = parent;
    child->prev_sibling_cyclic = 0;
    child->next_sibling = 0;

    Node* first_child = parent->first_child;

    if (first_child)
    {
        Node* last_child = first_child->prev_sibling_cyclic;
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

template <typename Node>
void prepend_child(Node* parent, Node* child)
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

template <typename Node>
void insert_child(Node* after, Node* child)
{
    plnnrc_assert(after != 0);
    plnnrc_assert(child != 0);
    plnnrc_assert(after->parent != 0);

    Node* l = after;
    Node* r = after->next_sibling;
    Node* p = after->parent;

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

template <typename Node>
void detach_node(Node* n)
{
    plnnrc_assert(n != 0);

    Node* p = n->parent;
    Node* n_next = n->next_sibling;
    Node* n_prev = n->prev_sibling_cyclic;

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

template <typename Node>
Node* preorder_traversal_next(const Node* root, Node* current)
{
    Node* n = current;

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
