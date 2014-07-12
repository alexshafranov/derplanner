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

#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/node_array.h"

namespace plnnrc {

namespace sexpr { struct Node; }
namespace pool { struct Handle; }

namespace ast {

enum Node_Type
{
    node_none = 0,

    #define PLNNRC_AST_NODE(NODE_ID) node_##NODE_ID,
    #include "derplanner/compiler/ast_node_tags.inl"
    #undef PLNNRC_AST_NODE
};

struct Node
{
    Node_Type type;
    sexpr::Node* s_expr;
    Node* parent;
    Node* first_child;
    Node* next_sibling;
    Node* prev_sibling_cyclic;
    void* annotation;
};

template <typename T>
T* annotation(Node* n)
{
    return static_cast<T*>(n->annotation);
}

class Tree
{
public:
    Tree(int error_node_cache_size=8);
    ~Tree();

    Node* root() { return &_root; }
    const Node* root() const { return &_root; }

    Node* make_node(Node_Type type, sexpr::Node* token=0);
    Node* clone_node(Node* original);
    Node* clone_subtree(Node* original);

    Id_Table ws_atoms;
    Id_Table ws_funcs;
    Id_Table ws_types;
    Id_Table methods;
    Id_Table operators;
    Node_Array type_tag_to_node;
    Node_Array error_node_cache;

private:
    Tree(const Tree&);
    const Tree& operator=(const Tree&);

    pool::Handle* _node_pool;
    Node _root;
};

// annotation types

struct WS_Type_Ann
{
    int type_tag;
};

struct Term_Ann
{
    int   type_tag;
    int   var_index;
    Node* var_def;
};

struct Atom_Ann
{
    int index;
    bool lazy;
};

struct Branch_Ann
{
    bool foreach;
};

struct Method_Ann
{
    bool processed;
};

#define PLNNRC_AST_NODE_GROUP(GROUP_ID, FIRST_ID, LAST_ID)          \
    inline bool is_##GROUP_ID(Node_Type type)                       \
    {                                                               \
        return type >= node_##FIRST_ID && type <= node_##LAST_ID;   \
    }                                                               \
                                                                    \
    inline bool is_##GROUP_ID(const Node* n)                        \
    {                                                               \
        return is_##GROUP_ID(n->type);                              \
    }                                                               \

#define PLNNRC_AST_NODE(NODE_ID)                \
    inline bool is_##NODE_ID(Node_Type type)    \
    {                                           \
        return type == node_##NODE_ID;          \
    }                                           \
                                                \
    inline bool is_##NODE_ID(const Node* n)     \
    {                                           \
        return is_##NODE_ID(n->type);           \
    }                                           \

#include "ast_node_tags.inl"

#undef PLNNRC_AST_NODE
#undef PLNNRC_AST_NODE_GROUP

}
}

#endif
