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

#include <stdlib.h> // size_t
#include <string.h> // memset
#include <stdint.h> // unitptr_t
#include "derplanner/compiler/derplanner_assert.h"
#include "derplanner/compiler/derplanner_memory.h"
#include "derplanner/compiler/ast.h"

namespace plnnrc {
namespace ast {

namespace
{
    const size_t page_size = 64 * 1024;
    const uintptr_t alignment = sizeof(void*);

    struct page
    {
        page* next;
        char* top;
        char* end;
    };

    inline char* align(char* ptr)
    {
        return reinterpret_cast<char*>((reinterpret_cast<uintptr_t>(ptr) + alignment) & ~(alignment - 1));
    }

    void* pool_allocate(void*& memory, size_t size)
    {
        page* p = reinterpret_cast<page*>(memory);

        if (!p || align(p->top) + size > p->end)
        {
            char* m = reinterpret_cast<char*>(memory::allocate(page_size));
            page* n = reinterpret_cast<page*>(m);

            if (!n)
            {
                return 0;
            }

            n->next = p;
            n->top = m + sizeof(page);
            n->end = m + page_size;

            p = n;
            memory = n;
        }

        char* result = align(p->top);
        p->top = result + size;

        return result;
    }

    void pool_clear(void* memory)
    {
        for (page* p = reinterpret_cast<page*>(memory); p != 0;)
        {
            page* n = p->next;
            memory::deallocate(p);
            p = n;
        }
    }
}

tree::tree()
    : _memory(0)
{
    memset(&_root, 0, sizeof(_root));
    _root.type = node_domain;
}

tree::~tree()
{
    if (_memory)
    {
        pool_clear(_memory);
    }
}

node* tree::make_node(node_type type, sexpr::node* token)
{
    node* n = reinterpret_cast<node*>(pool_allocate(_memory, sizeof(node)));

    if (n)
    {
        n->type = type;
        n->s_expr = token;
        n->parent = 0;
        n->first_child = 0;
        n->next_sibling = 0;
        n->prev_sibling_cyclic = 0;
    }

    return n;
}

node* tree::clone_node(node* original)
{
    plnnrc_assert(original != 0);

    node* n = reinterpret_cast<node*>(pool_allocate(_memory, sizeof(node)));

    if (n)
    {
        n->type = original->type;
        n->s_expr = original->s_expr;
        n->first_child = 0;
        n->parent = 0;
        n->next_sibling = 0;
        n->prev_sibling_cyclic = 0;
    }

    return n;
}

node* tree::clone_subtree(node* original)
{
    plnnrc_assert(original != 0);

    node* clone = clone_node(original);

    if (!clone)
    {
        return 0;
    }

    for (node* child_original = original->first_child; child_original != 0; child_original = child_original->next_sibling)
    {
        node* child_clone = clone_subtree(child_original);

        if (!child_clone)
        {
            return 0;
        }

        append_child(clone, child_clone);
    }

    return clone;
}

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

}
}
