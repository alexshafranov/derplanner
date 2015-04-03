//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
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

#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/parser.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/id_table.h"
#include "pool.h"

using namespace plnnrc;

void plnnrc::init(Parser& state, Lexer* lexer)
{
    plnnrc_assert(lexer != 0);
    memset(&state, 0, sizeof(state));

    // randomly chosen initial number of facts.
    init(state.fact_ids, 1024);
    // randomly chosen initial number of tasks.
    init(state.task_ids, 1024);

    state.lexer = lexer;
    // pool will have 1MB sized pages.
    state.pool = create_paged_pool(1024*1024);
}

void plnnrc::destroy(Parser& state)
{
    destroy(state.pool);
    destroy(state.task_ids);
    destroy(state.fact_ids);
    memset(&state, 0, sizeof(Parser));
}

template <typename T>
static inline T* allocate_node(Parser& state)
{
    T* result = static_cast<T*>(allocate(state.pool, sizeof(T), plnnrc_alignof(T)));
    memset(result, 0, sizeof(T));
    return result;
}

static inline Token expect(Parser& state, Token_Type token_type)
{
    Token tok = state.token;
    plnnrc_assert(tok.type == token_type);
    state.token = lex(*state.lexer);
    return tok;
}

static inline Token expect(Parser& state, bool (test_token)(const Token&))
{
    Token tok = state.token;
    plnnrc_assert(test_token(tok));
    state.token = lex(*state.lexer);
    return tok;
}

static inline Token eat(Parser& state)
{
    Token tok = state.token;
    state.token = lex(*state.lexer);
    return tok;
}

static inline Token peek(Parser& state)
{
    return state.token;
}

static ast::World*  parse_world(Parser& state);
static ast::Task*   parse_task(Parser& state);

void plnnrc::parse(Parser& state)
{
    // initialize buffered token.
    state.token = lex(*state.lexer);
    // initialize output.
    state.domain = allocate_node<ast::Domain>(state);

    expect(state, Token_Domain);
    Token name = expect(state, Token_Identifier);
    state.domain->name = name.value;
    expect(state, Token_L_Curly);

    ast::Task root_task = {};
    ast::Task* last_task = &root_task;

    for (;;)
    {
        Token tok = eat(state);
        if (is_R_Curly(tok))
        {
            break;
        }

        if (is_World(tok))
        {
            ast::World* world = parse_world(state);
            plnnrc_assert(!state.world);
            state.world = world;
            continue;
        }

        if (is_Task(tok))
        {
            ast::Task* task = parse_task(state);
            last_task->next = task;
            last_task = task;
            continue;
        }

        plnnrc_assert(false);
    }

    state.domain->tasks = root_task.next;
    expect(state, Token_Eof);
}

static ast::World* parse_world(Parser& state)
{
    ast::World* world = allocate_node<ast::World>(state);
    Token tok = expect(state, Token_L_Curly);

    ast::Fact_Type root_fact = {};
    ast::Fact_Type* last_fact = &root_fact;
    for (;;)
    {
        ast::Fact_Type* fact = allocate_node<ast::Fact_Type>(state);
        tok = expect(state, Token_Identifier);
        fact->name = tok.value;

        // parse parameters
        expect(state, Token_L_Paren);
        if (!is_R_Paren(peek(state)))
        {
            ast::Fact_Param root_param = {};
            ast::Fact_Param* last_param = &root_param;
            for (;;)
            {
                tok = expect(state, is_Type);
                ast::Fact_Param* param = allocate_node<ast::Fact_Param>(state);
                param->type = tok.type;
                last_param->next = param;
                last_param = param;

                if (!is_Comma(peek(state)))
                {
                    expect(state, is_R_Paren);
                    break;
                }

                expect(state, is_Comma);
            }

            fact->params = root_param.next;
            last_fact->next = fact;
            last_fact = fact;
        }
        else
        {
            eat(state);
        }

        if (is_R_Curly(peek(state)))
        {
            eat(state);
            break;
        }
    }

    world->facts = root_fact.next;
    return world;
}

static ast::Task* parse_task(Parser& state)
{
    Token tok = expect(state, Token_Identifier);

    ast::Task* task = allocate_node<ast::Task>(state);
    task->name = tok.value;

    // parse parameters.
    expect(state, Token_L_Paren);
    if (!is_R_Paren(peek(state)))
    {
        ast::Task_Param root_param = {};
        ast::Task_Param* last_param = &root_param;
        for (;;)
        {
            tok = expect(state, Token_Identifier);
            ast::Task_Param* task_param = allocate_node<ast::Task_Param>(state);
            task_param->name = tok.value;
            last_param->next = task_param;
            last_param = task_param;

            if (!is_Comma(peek(state)))
            {
                expect(state, is_R_Paren);
                break;
            }

            expect(state, is_Comma);
        }

        task->params = root_param.next;
    }
    else
    {
        eat(state);
    }

    // parse cases
    {
        expect(state, Token_L_Curly);
        int open = 1;
        for (;;)
        {
            tok = eat(state);
            if (is_L_Curly(tok))
            {
                ++open;
                continue;
            }

            if (is_R_Curly(tok))
            {
                --open;
                if (open == 0)
                {
                    break;
                }

                continue;
            }
        }
    }

    return task;
}
