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

// implementation (exposed for unit tests)
namespace plnnrc
{
    ast::World* parse_world(Parser& state);
    ast::Task*  parse_task(Parser& state);
    ast::Expr*  parse_precond(Parser& state);
    ast::Expr*  parse_task_list(Parser& state);
}

using namespace plnnrc;

plnnrc::Parser::Parser()
    : world(0)
    , domain(0)
    , lexer(0)
    , pool(0)
{
    memset(&token, 0, sizeof(token));
}

plnnrc::Parser::~Parser()
{
    destroy(*this);
}

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

ast::World* plnnrc::parse_world(Parser& state)
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
        last_fact->next = fact;
        last_fact = fact;

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

ast::Task* plnnrc::parse_task(Parser& state)
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
    expect(state, Token_L_Curly);
    if (!is_R_Curly(peek(state)))
    {
        ast::Case root_case = {};
        ast::Case* last_case = &root_case;
        for (;;)
        {
            expect(state, Token_Case);
            ast::Case* case_ = allocate_node<ast::Case>(state);

            ast::Expr* precond = parse_precond(state);
            expect(state, Token_Arrow);
            ast::Expr* task_list = parse_task_list(state);

            case_->precond = precond;
            case_->task_list = task_list;

            last_case->next = case_;
            last_case = case_;

            if (is_R_Curly(peek(state)))
            {
                eat(state);
                break;
            }
        }

        task->cases = root_case.next;
    }
    else
    {
        eat(state);
    }

    return task;
}

void plnnrc::append_child(ast::Expr* parent, ast::Expr* child)
{
    plnnrc_assert(parent != 0);
    plnnrc_assert(child != 0);

    child->parent = parent;
    child->prev_sibling_cyclic = 0;
    child->next_sibling = 0;

    ast::Expr* first_child = parent->child;

    if (first_child)
    {
        ast::Expr* last_child = first_child->prev_sibling_cyclic;
        plnnrc_assert(last_child != 0);
        last_child->next_sibling = child;
        child->prev_sibling_cyclic = last_child;
        first_child->prev_sibling_cyclic = child;
    }
    else
    {
        parent->child = child;
        child->prev_sibling_cyclic = child;
    }
}

void plnnrc::insert_child(ast::Expr* after, ast::Expr* child)
{
    plnnrc_assert(after != 0);
    plnnrc_assert(child != 0);
    plnnrc_assert(after->parent != 0);

    ast::Expr* left     = after;
    ast::Expr* right    = after->next_sibling;
    ast::Expr* parent   = after->parent;

    left->next_sibling = child;

    if (right)
    {
        right->prev_sibling_cyclic = child;
    }
    else
    {
        parent->child->prev_sibling_cyclic = child;
    }

    child->prev_sibling_cyclic = left;
    child->next_sibling = right;
    child->parent = parent;
}

void plnnrc::unparent(ast::Expr* node)
{
    plnnrc_assert(node != 0);

    ast::Expr* parent = node->parent;
    ast::Expr* next = node->next_sibling;
    ast::Expr* prev = node->prev_sibling_cyclic;

    plnnrc_assert(parent != 0);
    plnnrc_assert(prev != 0);

    if (next)
    {
        next->prev_sibling_cyclic = prev;
    }
    else
    {
        parent->child->prev_sibling_cyclic = prev;
    }

    if (prev->next_sibling)
    {
        prev->next_sibling = next;
    }
    else
    {
        parent->child = next;
    }

    node->parent = 0;
    node->next_sibling = 0;
    node->prev_sibling_cyclic = 0;
}

static ast::Expr* parse_expr(Parser& state);
static ast::Expr* parse_disjunct(Parser& state);
static ast::Expr* parse_conjunct(Parser& state);

ast::Expr* plnnrc::parse_precond(Parser& state)
{
    expect(state, is_L_Paren);
    ast::Expr* node_Expr = parse_expr(state);
    expect(state, is_R_Paren);
    return node_Expr;
}

ast::Expr* plnnrc::parse_task_list(Parser& state)
{
    expect(state, is_L_Square);
    ast::Expr* node_Task_List = allocate_node<ast::Expr>(state);

    if (!is_R_Square(peek(state)))
    {
        for (;;)
        {
            ast::Expr* node_Task = parse_conjunct(state);
            plnnrc::append_child(node_Task_List, node_Task);

            if (!is_Comma(peek(state)))
            {
                expect(state, is_R_Square);
                break;
            }

            expect(state, is_Comma);
        }
    }
    else
    {
        eat(state);
    }

    return node_Task_List;
}

static ast::Expr* parse_expr(Parser& state)
{
    ast::Expr* node = parse_disjunct(state);

    if (is_Or(peek(state)))
    {
        ast::Expr* node_Or = allocate_node<ast::Expr>(state);
        node_Or->type = Token_Or;
        plnnrc::append_child(node_Or, node);

        while (is_Or(peek(state)))
        {
            eat(state);
            ast::Expr* node = parse_disjunct(state);
            plnnrc::append_child(node_Or, node);
        }

        return node_Or;
    }

    return node;
}

static ast::Expr* parse_disjunct(Parser& state)
{
    ast::Expr* node = parse_conjunct(state);

    if (is_And(peek(state)))
    {
        ast::Expr* node_And = allocate_node<ast::Expr>(state);
        node_And->type = Token_And;
        plnnrc::append_child(node_And, node);

        while (is_And(peek(state)))
        {
            eat(state);
            ast::Expr* node = parse_conjunct(state);
            plnnrc::append_child(node_And, node);
        }

        return node_And;
    }

    return node;
}

static ast::Expr* parse_conjunct(Parser& state)
{
    Token tok = eat(state);

    if (is_L_Paren(tok))
    {
        ast::Expr* node_Expr = parse_expr(state);
        expect(state, is_R_Paren);
        return node_Expr;
    }

    if (is_Literal(tok))
    {
        ast::Expr* node_Literal = allocate_node<ast::Expr>(state);
        node_Literal->type = tok.type;
        node_Literal->value = tok.value;
        return node_Literal;
    }

    if (is_Identifier(tok))
    {
        ast::Expr* node_Fact = allocate_node<ast::Expr>(state);
        node_Fact->type = tok.type;
        node_Fact->value = tok.value;

        // actually a fact if there's arguments.
        if (is_L_Paren(peek(state)))
        {
            eat(state);

            if (!is_R_Paren(peek(state)))
            {
                for (;;)
                {
                    Token tok = expect(state, Token_Identifier);
                    ast::Expr* node_Var = allocate_node<ast::Expr>(state);
                    node_Var->type = tok.type;
                    node_Var->value = tok.value;
                    plnnrc::append_child(node_Fact, node_Var);

                    if (!is_Comma(peek(state)))
                    {
                        expect(state, is_R_Paren);
                        break;
                    }

                    expect(state, is_Comma);
                }
            }
            else
            {
                eat(state);
            }
        }

        return node_Fact;
    }

    plnnrc_assert(false);
    return 0;
}
