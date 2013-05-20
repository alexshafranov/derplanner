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

namespace plnnrc {

namespace sexpr
{
    struct node;
}

namespace ast {

enum node_type
{
    node_none = 0,

    node_domain,

    node_op_and,
    node_op_or,
    node_op_not,

    node_atom,

    node_term_variable,
    node_term_int,
    node_term_float,
    node_term_call,
};

struct node
{
    node_type type;
    sexpr::node* s_expr;
    node* parent;
    node* first_child;
    node* next_sibling;
    node* prev_sibling_cyclic;
};

inline bool is_logical_op(const node* n)
{
    return n->type >= node_op_and && n->type <= node_op_not;
}

inline bool is_term(const node* n)
{
    return n->type >= node_term_variable && n->type <= node_term_call;
}

void append_child(node* parent, node* child);
void prepend_child(node* parent, node* child);
void insert_child(node* after, node* child);

void detach_node(node* n);

class tree
{
public:
    tree();
    ~tree();

    inline node* root() { return &_root; }
    inline const node* root() const { return &_root; }

    node* make_node(node_type type, sexpr::node* token);
    node* clone_node(node* original);
    node* clone_subtree(node* original);

private:
    void* _memory;
    node _root;
};

}
}

#endif
