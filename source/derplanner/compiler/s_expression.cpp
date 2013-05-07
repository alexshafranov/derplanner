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

#include "derplanner/compiler/s_expression.h"
#include <stdlib.h>
#include <ctype.h>

namespace derplanner {
namespace s_expression {

namespace {

static const int chunk_node_count = 2048;

struct node_chunk
{
    node_chunk* next;
    int top;
    node data[chunk_node_count];
};

void free_chunks(void* memory)
{
    for (node_chunk* chunk = reinterpret_cast<node_chunk*>(memory); chunk != 0;)
    {
        node_chunk* next = chunk->next;
        free(chunk);
        chunk = next;
    }
}

node* alloc_node(void*& memory)
{
    node_chunk* chunk = reinterpret_cast<node_chunk*>(memory);

    if (chunk->top >= chunk_node_count)
    {
        node_chunk* new_chunk = reinterpret_cast<node_chunk*>(malloc(sizeof(node_chunk)));

        new_chunk->next = chunk;
        new_chunk->top = 0;

        chunk = new_chunk;
        memory = new_chunk;
    }

    chunk->top++;

    return chunk->data + chunk->top;
}

enum token_type
{
    token_error = 0,
    token_lp,
    token_rp,
    token_symbol,
    token_number,
};

struct parse_state
{
    void* tree_memory;
    int line;
    int column;
    char* cursor;
    node* parent;
};

inline node* push_list(parse_state& state)
{
    node* n = alloc_node(state.tree_memory);
    n->parent = state.parent;
    state.parent = n;
    return state.parent;
}

inline node* pop_list(parse_state& state)
{
    node* n = state.parent;
    state.parent = n->parent;
    return n;
}

inline node* add_child(parse_state& state)
{
    node* n = alloc_node(state.tree_memory);
    node* p = state.parent;

    n->parent = p;
    n->sibling = p->first_child;
    p->first_child = n;

    return n;
}

inline void move(parse_state& state)
{
    state.cursor++;
    state.column++;
}

inline void skip_whitespace(parse_state& state)
{
    while (*state.cursor == ' ' || *state.cursor == '\f' || *state.cursor == '\t' || *state.cursor == '\v')
    {
        move(state);
    }
}

inline bool has_token(parse_state& state)
{
    return true;
}

inline void increment_line(parse_state& state)
{
    char c = *state.cursor;

    move(state);

    if ((c == '\n' || c == '\r') && (*state.cursor != c))
    {
        move(state);
    }

    state.line++;
    state.column = 0;
}

inline token_type next_token(parse_state& state)
{
    while (*state.cursor)
    {
        switch (*state.cursor)
        {
        case '\n': case '\r':
            increment_line(state);
            break;
        case ' ': case '\f': case '\t': case '\v':
            move(state);
            break;
        case ';':
            while (*state.cursor != '\n' || *state.cursor != '\r' || *state.cursor != '\0')
            {
                move(state);
            }
            break;
        }
    }

    return token_error;
}

inline void match_token(parse_state& state, token_type token)
{
}

} // unnamed namespace

tree::tree()
    : root(0)
    , _memory(0)
{
}

tree::~tree()
{
    if (_memory)
    {
        free_chunks(_memory);
    }
}

void tree::parse(char* buffer)
{
    if (_memory)
    {
        free_chunks(_memory);
        _memory = 0;
        root = 0;
    }

    parse_state state;
    state.tree_memory = _memory;
    state.line = 0;
    state.column = 0;
    state.cursor = buffer;
    state.parent = 0;

    root = push_list(state);

    skip_whitespace(state);
    match_token(state, token_lp);

    root->line = state.line;
    root->column = state.column;

    while (has_token(state))
    {
        switch (next_token(state))
        {
        case token_lp:
            {
                push_list(state);
            }
            break;
        case token_rp:
            {
                pop_list(state);
            }
            break;
        case token_number:
            {
                node* n = add_child(state);
                n->type = node_number;
            }
            break;
        case token_symbol:
            {
                node* n = add_child(state);
                n->type = node_symbol;
            }
            break;
        default:
            break;
        }
    }
}

}
}
