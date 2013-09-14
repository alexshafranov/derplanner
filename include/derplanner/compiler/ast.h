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

#include <stddef.h> // for size_t

#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/node_array.h"

namespace plnnrc {

namespace sexpr
{
    struct node;
}

namespace pool
{
    struct handle;
}

namespace ast {

enum node_type
{
    node_none = 0,

    #define PLNNRC_AST_NODE(NODE_ID) NODE_ID,
    #include "derplanner/compiler/ast_node_tags.inl"
    #undef PLNNRC_AST_NODE
};

struct node
{
    node_type type;
    sexpr::node* s_expr;
    node* parent;
    node* first_child;
    node* next_sibling;
    node* prev_sibling_cyclic;
    void* annotation;
};

struct ws_type_ann
{
    int type_tag;
};

struct term_ann
{
    int   type_tag;
    int   var_index;
    node* var_def;
};

struct atom_ann
{
    int index;
};

template <typename T>
T* annotation(node* n)
{
    return static_cast<T*>(n->annotation);
}

inline bool is_logical_op(node_type type)
{
    return type >= node_op_and && type <= node_op_not;
}

inline bool is_logical_op(const node* n)
{
    return is_logical_op(n->type);
}

inline bool is_term(node_type type)
{
    return type >= node_term_variable && type <= node_term_call;
}

inline bool is_term(const node* n)
{
    return is_term(n->type);
}

class tree
{
public:
    tree();
    ~tree();

    inline node* root() { return &_root; }
    inline const node* root() const { return &_root; }

    node* make_node(node_type type, sexpr::node* token=0);
    node* clone_node(node* original);
    node* clone_subtree(node* original);
    char* make_token(size_t length);

    id_table ws_atoms;
    id_table ws_types;
    id_table methods;
    id_table operators;
    node_array type_tag_to_node;

private:
    tree(const tree&);
    const tree& operator=(const tree&);

    pool::handle* _node_pool;
    pool::handle* _token_pool;
    node _root;
};

}
}

#endif
