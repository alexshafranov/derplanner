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
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/transforms.h"
#include "derplanner/compiler/parser.h"

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
    : lexer(0)
{
    memset(&tree, 0, sizeof(tree));
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
    state.lexer = lexer;
    plnnrc::init(state.tree);
}

void plnnrc::destroy(Parser& state)
{
    plnnrc::destroy(state.tree);
    memset(&state, 0, sizeof(Parser));
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
    state.tree.domain = create_domain(state.tree);

    expect(state, Token_Domain);
    Token name = expect(state, Token_Id);
    state.tree.domain->name = name.value;
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
            plnnrc_assert(!state.tree.world);
            state.tree.world = world;
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

    state.tree.domain->tasks = root_task.next;
    expect(state, Token_Eof);

    // convert all preconditions to DNF form.
    if (state.tree.domain)
    {
        for (ast::Task* task = state.tree.domain->tasks; task != 0; task = task->next)
        {
            for (ast::Case* case_ = task->cases; case_ != 0; case_ = case_->next)
            {
                ast::Expr* precond = case_->precond;
                ast::Expr* precond_dnf = plnnrc::convert_to_dnf(state.tree, precond);
                case_->precond = precond_dnf;
            }
        }
    }

    plnnrc::build_lookups(state.tree);
}

ast::World* plnnrc::parse_world(Parser& state)
{
    ast::World* world = create_world(state.tree);
    Token tok = expect(state, Token_L_Curly);

    ast::Fact_Type root_fact = {};
    ast::Fact_Type* last_fact = &root_fact;
    for (;;)
    {
        ast::Fact_Type* fact = create_fact_type(state.tree, last_fact);
        tok = expect(state, Token_Id);
        fact->name = tok.value;
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
                ast::Fact_Param* param = create_fact_param(state.tree, last_param);
                param->type = tok.type;
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
    Token tok = expect(state, Token_Id);
    ast::Task* task = create_task(state.tree, tok.value);

    // parse parameters.
    expect(state, Token_L_Paren);
    if (!is_R_Paren(peek(state)))
    {
        ast::Task_Param root_param = {};
        ast::Task_Param* last_param = &root_param;
        for (;;)
        {
            tok = expect(state, Token_Id);
            ast::Task_Param* task_param = create_task_param(state.tree, tok.value, last_param);
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
            ast::Case* case_ = create_case(state.tree);

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

static ast::Expr* parse_expr(Parser& state);
static ast::Expr* parse_disjunct(Parser& state);
static ast::Expr* parse_conjunct(Parser& state);

ast::Expr* plnnrc::parse_precond(Parser& state)
{
    expect(state, is_L_Paren);

    // empty expression -> add dummy node.
    if (is_R_Paren(peek(state)))
    {
        return create_expr(state.tree, Token_And);
    }

    ast::Expr* node_Expr = parse_expr(state);
    expect(state, is_R_Paren);
    return node_Expr;
}

ast::Expr* plnnrc::parse_task_list(Parser& state)
{
    expect(state, is_L_Square);
    ast::Expr* node_Task_List = create_expr(state.tree, Token_List);

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
        ast::Expr* node_Or = create_expr(state.tree, Token_Or);
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
        ast::Expr* node_And = create_expr(state.tree, Token_And);
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
        ast::Expr* node_Literal = create_expr(state.tree, tok.type);
        node_Literal->value = tok.value;
        return node_Literal;
    }

    if (is_Not(tok))
    {
        ast::Expr* node_Not = create_expr(state.tree, Token_Not);
        ast::Expr* node = parse_conjunct(state);
        plnnrc::append_child(node_Not, node);
        return node_Not;
    }

    if (is_Id(tok))
    {
        ast::Expr* node = 0;

        if (is_L_Paren(peek(state)))
        {
            eat(state);

            node = create_expr(state.tree, Token_Fact);
            node->value = tok.value;

            if (!is_R_Paren(peek(state)))
            {
                for (;;)
                {
                    Token tok = expect(state, Token_Id);
                    ast::Expr* node_Var = create_expr(state.tree, Token_Var);
                    node_Var->value = tok.value;
                    plnnrc::append_child(node, node_Var);

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
        else
        {
            node = create_expr(state.tree, Token_Var);
            node->value = tok.value;
        }

        return node;
    }

    plnnrc_assert(false);
    return 0;
}
