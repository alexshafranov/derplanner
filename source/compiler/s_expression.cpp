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

#include <stdlib.h>
#include "pool.h"
#include "tree_tools.h"
#include "derplanner/compiler/config.h"
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/s_expression.h"

namespace plnnrc {
namespace sexpr {

namespace
{
    const size_t page_size = DERPLANNER_SEXPR_MEMPAGE_SIZE;

    Node* alloc_node(pool::Handle* pool)
    {
        return static_cast<Node*>(pool::allocate(pool, sizeof(Node), plnnrc_alignof(Node)));
    }

    enum Token_Type
    {
        token_none = 0,
        token_lp,
        token_rp,
        token_symbol,
        token_int,
        token_float,
    };

    struct Token
    {
        Token_Type  type;
        char*       begin;
        int         count;
        int         line;
        int         column;
    };

    void init(Token& token)
    {
        token.type = token_none;
        token.begin = 0;
        token.count = 0;
        token.line = 0;
        token.column = 0;
    }

    struct Parse_State
    {
        pool::Handle* pool;
        int line;
        int column;
        char* cursor;
        char* null;
        Node* parent;
        Node* root;
    };

    void init(Parse_State& state, char* buffer, pool::Handle* pool)
    {
        state.pool = pool;
        state.line = 1;
        state.column = 1;
        state.cursor = buffer;
        state.null = 0;
        state.parent = 0;
        state.root = 0;
    }

    void move(Parse_State& state)
    {
        state.cursor++;
        state.column++;
    }

    void move(Token& token)
    {
        token.count++;
    }

    Node* append_child(Parse_State& state)
    {
        Node* n = alloc_node(state.pool);
        Node* p = state.parent;

        if (!n)
        {
            return 0;
        }

        n->parent = p;
        n->first_child = 0;
        n->next_sibling = 0;
        n->prev_sibling_cyclic = 0;

        if (p)
        {
            Node* first_child = p->first_child;

            if (first_child)
            {
                Node* last_child = first_child->prev_sibling_cyclic;
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

    Node* push_list(Parse_State& state)
    {
        Node* n = append_child(state);

        if (!n)
        {
            return 0;
        }

        n->type = node_list;
        n->line = state.line;
        n->column = state.column;
        n->token = 0;
        n->line_end = -1;
        n->column_end = -1;

        state.parent = n;

        return n;
    }

    Node* pop_list(Parse_State& state)
    {
        Node* n = state.parent;

        if (!n || n == state.root)
        {
            state.parent = 0;
            return 0;
        }

        n->line_end = state.line;
        n->column_end = state.column;

        state.parent = n->parent;
        return n;
    }

    Node* append_node(Parse_State& state, Node_Type type)
    {
        Node* n = append_child(state);

        if (!n)
        {
            return 0;
        }

        n->type = type;
        n->line = state.line;
        n->column = state.column;
        n->token = state.cursor;
        n->line_end = state.line;
        plnnrc_assert(state.null);
        n->column_end = state.column + static_cast<int>(state.null - state.cursor);
        return n;
    }

    void increment_line(Parse_State& state)
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

    bool is_delimeter(char* position)
    {
        switch (*position)
        {
        case '\0':
        case '\n': case '\r':
        case ' ': case '\f': case '\t': case '\v':
        case '(': case ')':
            return true;
        }

        return false;
    }

    Token scan_symbol(Parse_State& state)
    {
        Token result;
        result.type = token_symbol;
        result.begin = state.cursor;
        result.count = 0;

        while (!is_delimeter(result.begin + result.count))
        {
            move(result);
        }

        return result;
    }

    int scan_number_char_class(char* position)
    {
        switch (*position)
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

    int scan_number_transitions[] = {
         1,  3,  2, -1,
        -1,  3,  2, -1,
        -1,  4, -1, -1,
        -1,  3,  4,  5,
        -1,  4, -1,  5,
         6,  7, -1, -1,
        -1,  7, -1, -1,
        -1,  7, -1, -1,
    };

    Token scan_number(Parse_State& state)
    {
        Token result;
        result.type = token_none;
        result.begin = state.cursor;
        result.count = 0;

        int s = 0;

        while (!is_delimeter(result.begin + result.count))
        {
            int c = scan_number_char_class(result.begin + result.count);

            if (c < 0)
            {
                return result;
            }

            int n = scan_number_transitions[s * 4 + c];

            if (n < 0)
            {
                return result;
            }

            s = n;

            move(result);
        }

        switch (s)
        {
        case 3:
            result.type = token_int;
            break;
        case 4: case 7:
            result.type = token_float;
            break;
        }

        return result;
    }

    void null_terminate(Parse_State& state)
    {
        if (state.null)
        {
            *state.null = '\0';
            state.null = 0;
        }
    }

    Token next_token(Parse_State& state)
    {
        Token result;
        init(result);

        while (*state.cursor)
        {
            switch (*state.cursor)
            {
            case '\n': case '\r':
                increment_line(state);
                null_terminate(state);
                break;
            case ' ': case '\f': case '\t': case '\v':
                move(state);
                null_terminate(state);
                break;
            case ';':
                while (*state.cursor != '\n' && *state.cursor != '\r' && *state.cursor != '\0')
                {
                    move(state);
                }
                null_terminate(state);
                break;
            case '(':
                result.type = token_lp;
                result.begin = state.cursor;
                result.count = 1;
                result.line = state.line;
                result.column = state.column;
                null_terminate(state);
                return result;
            case ')':
                result.type = token_rp;
                result.begin = state.cursor;
                result.count = 1;
                result.line = state.line;
                result.column = state.column;
                null_terminate(state);
                return result;
            default:
                result = scan_number(state);

                if (result.type == token_none)
                {
                    result = scan_symbol(state);
                }

                result.line = state.line;
                result.column = state.column;

                state.null = result.begin + result.count;

                return result;
            }
        }

        return result;
    }

    Token lookahead(Parse_State& state)
    {
        Parse_State saved_state = state;
        Token result = next_token(state);
        state = saved_state;
        return result;
    }

} // unnamed namespace

Tree::Tree()
    : _pool(0)
    , _root(0)
{
}

Tree::~Tree()
{
    if (_pool)
    {
        pool::destroy(_pool);
    }
}

Parse_Result Tree::parse(char* buffer)
{
    plnnrc_assert(buffer != 0);

    Parse_Result result;
    result.status = parse_ok;
    result.line = -1;
    result.column = -1;

    if (_pool)
    {
        pool::destroy(_pool);
        _pool = 0;
        _root = 0;
    }

    pool::Handle* pool = pool::create(page_size);

    if (!pool)
    {
        result.status = parse_out_of_memory;
        return result;
    }

    _pool = pool;

    Parse_State state;
    init(state, buffer, _pool);

    _root = push_list(state);

    if (!_root)
    {
        result.status = parse_out_of_memory;
        return result;
    }

    state.root = _root;

    Token root_token = lookahead(state);

    if (root_token.type != token_lp && root_token.type != token_none)
    {
        result.status = parse_expected_lp;
        result.line = root_token.line;
        result.column = root_token.column;
        return result;
    }

    for (;;)
    {
        Token t = next_token(state);

        if (t.type == token_none)
        {
            break;
        }

        switch (t.type)
        {
        case token_lp:
            {
                if (!push_list(state))
                {
                    result.status = parse_out_of_memory;
                    return result;
                }
            }
            break;
        case token_rp:
            {
                if (!pop_list(state))
                {
                    result.status = parse_excess_close;
                    result.line = state.line;
                    result.column = state.column;
                    return result;
                }
            }
            break;
        case token_int:
            {
                if (!append_node(state, node_int))
                {
                    result.status = parse_out_of_memory;
                    return result;
                }
            }
            break;
        case token_float:
            {
                if (!append_node(state, node_float))
                {
                    result.status = parse_out_of_memory;
                    return result;
                }
            }
            break;
        case token_symbol:
            {
                if (!append_node(state, node_symbol))
                {
                    result.status = parse_out_of_memory;
                    return result;
                }
            }
            break;
        default:
            plnnrc_assert(false);
            break;
        }

        state.cursor = t.begin + t.count;
        state.column += t.count;
    }

    if (state.parent != state.root)
    {
        result.status = parse_excess_open;
        result.line = state.parent->line;
        result.column = state.parent->column;
        return result;
    }

    return result;
}

float as_float(const Node* n)
{
    return static_cast<float>(strtod(n->token, 0));
}

int as_int(const Node* n)
{
    return static_cast<int>(strtol(n->token, 0, 10));
}

void glue_tokens(const Node* n)
{
    plnnrc_assert(n->type == node_list);

    for (Node* c = n->first_child; c != 0; c = c->next_sibling)
    {
        if (c->next_sibling)
        {
            if (c->type == node_symbol && c->next_sibling->type == node_symbol)
            {
                char* t = c->token;

                while (*t)
                {
                    ++t;
                }

                *t = ' ';

                detach_node(c->next_sibling);
            }
        }
    }
}

}
}
