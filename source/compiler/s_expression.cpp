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

#include <assert.h>
#include "derplanner/compiler/s_expression.h"
#include <stdlib.h>
#include <ctype.h>

namespace derplanner {
namespace s_expression {

namespace {

const int chunk_node_count = 2048;

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

    if (!chunk || chunk->top >= chunk_node_count)
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
    token_none = 0,
    token_lp,
    token_rp,
    token_symbol,
    token_int,
    token_float,
};

struct parse_state
{
    void** tree_memory;
    int line;
    int column;
    char* cursor;
    char* cursor_next;
    char* term_loc;
    node* parent;
};

void init_state(parse_state& state, char* buffer, void** memory)
{
    state.tree_memory = memory;
    state.line = 1;
    state.column = 1;
    state.cursor = buffer;
    state.cursor_next = buffer;
    state.term_loc = 0;
    state.parent = 0;
}

void move(parse_state& state)
{
    state.cursor++;
    state.column++;
}

node* append_child(parse_state& state)
{
    node* n = alloc_node(*state.tree_memory);
    node* p = state.parent;

    n->parent = p;
    n->first_child = 0;
    n->next_sibling = 0;
    n->prev_sibling_cyclic = 0;

    if (p)
    {
        node* first_child = p->first_child;

        if (first_child)
        {
            node* last_child = first_child->prev_sibling_cyclic;
            last_child->next_sibling = n;
            n->prev_sibling_cyclic = last_child;
            first_child->prev_sibling_cyclic = n;
        }
        else
        {
            p->first_child = n;
            n->prev_sibling_cyclic = n;
        }
    }

    return n;
}

node* push_list(parse_state& state)
{
    node* n = append_child(state);

    n->type = node_list;
    n->line = state.line;
    n->column = state.column;
    n->token = 0;

    state.parent = n;

    return n;
}

node* pop_list(parse_state& state)
{
    node* n = state.parent;
    state.parent = n->parent;
    return n;
}

node* append_node(parse_state& state, node_type type)
{
    node* n = append_child(state);
    n->type = type;
    n->line = state.line;
    n->column = state.column;
    n->token = state.cursor;
    return n;
}

void skip_whitespace(parse_state& state)
{
    while (*state.cursor == ' ' || *state.cursor == '\f' || *state.cursor == '\t' || *state.cursor == '\v')
    {
        move(state);
    }
}

bool buffer_left(parse_state& state)
{
    return *state.cursor_next != '\0';
}

void increment_line(parse_state& state)
{
    char c = *state.cursor;

    move(state);

    if ((*state.cursor == '\n' || *state.cursor == '\r') && (*state.cursor != c))
    {
        move(state);
    }

    state.line++;
    state.column = 1;
}

void scan_symbol(parse_state& state)
{
    char* begin = state.cursor;
    int line = state.line;
    int column = state.column;

    while (true)
    {
        switch (*state.cursor)
        {
        case '\0':
        case '\n': case '\r':
        case ' ': case '\f': case '\t': case '\v':
        case '(': case ')':
            state.cursor_next = state.cursor;
            state.term_loc = state.cursor;
            state.cursor = begin;
            state.line = line;
            state.column = column;
            return;
        default:
            move(state);
            break;
        }
    }
}

int scan_number_char_class(parse_state& state)
{
    switch (*state.cursor)
    {
    case '+': case '-':
        return 0;
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return 1;
    case '.':
        return 2;
    case 'e': case 'E':
        return 3;
    }

    return -1;
}

static int scan_number_transitions[] = {
     1,  3,  2, -1,
    -1,  3,  2, -1,
    -1,  4, -1, -1,
    -1,  3,  4,  5,
    -1,  4, -1,  5,
     6,  7, -1, -1,
    -1,  7, -1, -1,
    -1,  7, -1, -1,
};

token_type scan_number(parse_state& state)
{
    char* begin = state.cursor;
    int line = state.line;
    int column = state.column;

    int s = 0;

    while (true)
    {
        switch (*state.cursor)
        {
        case '\0':
        case '\n': case '\r':
        case ' ': case '\f': case '\t': case '\v':
        case '(': case ')':
            state.cursor_next = state.cursor;
            state.term_loc = state.cursor;
            state.cursor = begin;
            state.line = line;
            state.column = column;

            if (s == 3) { return token_int; }
            if (s == 4 || s == 7) { return token_float; }
            return token_none;
        }

        int c = scan_number_char_class(state);

        if (c < 0)
        {
            return token_none;
        }

        int n = scan_number_transitions[s * 4 + c];

        if (n < 0)
        {
            return token_none;
        }

        s = n;

        move(state);
    }

    return token_none;
}

void terminate(parse_state& state)
{
    if (state.term_loc)
    {
        *state.term_loc = '\0';
        state.term_loc = 0;
    }
}

token_type next_token(parse_state& state)
{
    state.cursor = state.cursor_next;

    while (*state.cursor)
    {
        switch (*state.cursor)
        {
        case '\n': case '\r':
            increment_line(state);
            terminate(state);
            break;
        case ' ': case '\f': case '\t': case '\v':
            move(state);
            terminate(state);
            break;
        case ';':
            while (*state.cursor != '\n' && *state.cursor != '\r' && *state.cursor != '\0')
            {
                move(state);
            }
            terminate(state);
            break;
        case '(':
            move(state);
            state.cursor_next = state.cursor;
            terminate(state);
            return token_lp;
        case ')':
            move(state);
            state.cursor_next = state.cursor;
            terminate(state);
            return token_rp;
        default:
            {
                token_type t = scan_number(state);

                if (t != token_none)
                {
                    return t;
                }

                scan_symbol(state);
                return token_symbol;
            }
        }
    }

    return token_none;
}

void match_token(parse_state& state, token_type token)
{
    next_token(state);
}

} // unnamed namespace

tree::tree()
    : root(0)
    , memory(0)
{
}

tree::~tree()
{
    if (memory)
    {
        free_chunks(memory);
    }
}

parse_status tree::parse(char* buffer)
{
    if (memory)
    {
        free_chunks(memory);
        memory = 0;
        root = 0;
    }

    parse_state state;
    init_state(state, buffer, &memory);

    skip_whitespace(state);
    match_token(state, token_lp);

    root = push_list(state);

    while (buffer_left(state))
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
        case token_int:
            {
                append_node(state, node_int);
            }
            break;
        case token_float:
            {
                append_node(state, node_float);
            }
            break;
        case token_symbol:
            {
                append_node(state, node_symbol);
            }
            break;
        default:
            assert(false);
            break;
        }
    }

    return parse_ok;
}

float as_float(const node& n)
{
    return static_cast<float>(strtod(n.token, 0));
}

int as_int(const node& n)
{
    return static_cast<int>(strtol(n.token, 0, 10));
}

}
}
