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

#include <string.h> // memset, memcpy
#include <stdint.h> // unitptr_t
#include "pool.h"
#include "tree_tools.h"
#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/ast.h"

namespace plnnrc {
namespace ast {

namespace
{
    const size_t node_page_size = DERPLANNER_AST_MEMPAGE_SIZE;

    struct Annotation_Trait
    {
        size_t size;
        size_t alignment;
    };

    Annotation_Trait type_annotation_trait(Node_Type type)
    {
        Annotation_Trait result = {0, 0};

        if (is_term(type))
        {
            result.size = sizeof(Term_Ann);
            result.alignment = plnnrc_alignof(Term_Ann);
            return result;
        }

        if (type == node_worldstate_type)
        {
            result.size = sizeof(WS_Type_Ann);
            result.alignment = plnnrc_alignof(WS_Type_Ann);
            return result;
        }

        if (type == node_atom)
        {
            result.size = sizeof(Atom_Ann);
            result.alignment = plnnrc_alignof(Atom_Ann);
            return result;
        }

        if (type == node_branch)
        {
            result.size = sizeof(Branch_Ann);
            result.alignment = plnnrc_alignof(Branch_Ann);
            return result;
        }

        if (type == node_method)
        {
            result.size = sizeof(Method_Ann);
            result.alignment = plnnrc_alignof(Method_Ann);
            return result;
        }

        if (type == node_error)
        {
            result.size = sizeof(Error_Ann);
            result.alignment = plnnrc_alignof(Error_Ann);
        }

        return result;
    }
}

Tree::Tree(int error_node_cache_size)
    : _node_pool(0)
{
    memset(&_root, 0, sizeof(_root));
    _root.type = node_root;
    error_node_cache.init(error_node_cache_size);
}

Tree::~Tree()
{
    if (_node_pool)
    {
        pool::destroy(_node_pool);
    }
}

Node* Tree::make_node(Node_Type type, sexpr::Node* token)
{
    if (!_node_pool)
    {
        pool::Handle* pool = pool::create(node_page_size);

        if (!pool)
        {
            return 0;
        }

        _node_pool = pool;
    }

    Node* n = static_cast<Node*>(pool::allocate(_node_pool, sizeof(Node), plnnrc_alignof(Node)));

    if (n)
    {
        n->type = type;
        n->s_expr = token;
        n->parent = 0;
        n->first_child = 0;
        n->next_sibling = 0;
        n->prev_sibling_cyclic = 0;
        n->annotation = 0;

        Annotation_Trait t = type_annotation_trait(type);

        if (t.size > 0)
        {
            n->annotation = pool::allocate(_node_pool, t.size, t.alignment);

            if (!n->annotation)
            {
                return 0;
            }

            memset(n->annotation, 0, t.size);
        }
    }

    return n;
}

Node* Tree::clone_node(Node* original)
{
    plnnrc_assert(original != 0);
    Node* n = make_node(original->type, original->s_expr);

    if (n && n->annotation)
    {
        memcpy(n->annotation, original->annotation, type_annotation_trait(original->type).size);
    }

    return n;
}

Node* Tree::clone_subtree(Node* original)
{
    plnnrc_assert(original != 0);

    Node* clone = clone_node(original);

    if (!clone)
    {
        return 0;
    }

    for (Node* child_original = original->first_child; child_original != 0; child_original = child_original->next_sibling)
    {
        Node* child_clone = clone_subtree(child_original);

        if (!child_clone)
        {
            return 0;
        }

        append_child(clone, child_clone);
    }

    return clone;
}

}
}
