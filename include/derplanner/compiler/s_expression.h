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

#ifndef DERPLANNER_COMPILER_S_EXPRESSION_H_
#define DERPLANNER_COMPILER_S_EXPRESSION_H_

namespace plnnrc {

namespace pool { struct Handle; }

namespace sexpr {

enum Node_Type
{
    node_none = 0,
    node_list,
    node_symbol,
    node_int,
    node_float,
};

struct Node
{
    Node_Type type;
    int line;
    int column;
    int line_end;
    int column_end;
    char* token;
    Node* parent;
    Node* first_child;
    Node* next_sibling;
    Node* prev_sibling_cyclic;
};

int as_int(const Node* n);
float as_float(const Node* n);

void glue_tokens(const Node* n);

enum Parse_Status
{
    parse_ok = 0,
    parse_excess_open,
    parse_excess_close,
    parse_expected_lp,
    parse_out_of_memory,
};

struct Parse_Result
{
    Parse_Status status;
    int line;
    int column;
};

class Tree
{
public:
    Tree();
    ~Tree();

    Parse_Result parse(char* buffer);
    Node* root() const { return _root; }

private:
    Tree(const Tree&);
    const Tree& operator=(const Tree&);

    pool::Handle* _pool;
    Node* _root;
};

inline bool is_list(const Node* n)
{
    return n->type == node_list;
}

inline bool is_symbol(const Node* n)
{
    return n->type == node_symbol;
}

inline bool is_int(const Node* n)
{
    return n->type == node_int;
}

inline bool is_float(const Node* n)
{
    return n->type == node_float;
}

}
}

#endif
