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
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/parser.h"

// implementation (exposed for unit tests)
namespace plnnrc
{
    ast::World*         parse_world(Parser& state);
    ast::Primitive*     parse_primitive(Parser& state);
    ast::Task*          parse_task(Parser& state);
    ast::Expr*          parse_precond(Parser& state);
    void                parse_task_list(Parser& state, ast::Case* case_);
}

using namespace plnnrc;

template <typename T>
struct Children_Builder
{
    Parser*     state;
    Array<T*>*  output;
    uint32_t    scratch_rewind; 

    Children_Builder(Parser* state, Array<T*>* output) : state(state), output(output)
    {
        scratch_rewind = plnnrc::size(state->scratch);
    }

    ~Children_Builder()
    {
        const uint32_t scratch_size = plnnrc::size(state->scratch);
        plnnrc_assert(scratch_size >= scratch_rewind);
        const uint32_t nodes_size = scratch_size - scratch_rewind;

        if (nodes_size > 0)
        {
            T** nodes = plnnrc::allocate<T*>(state->tree.pool, nodes_size);

            for (uint32_t i = scratch_rewind; i < scratch_size; ++i)
            {
                nodes[i - scratch_rewind] = static_cast<T*>(state->scratch[i]);
            }

            plnnrc::init(*output, state->tree.pool, nodes_size);
            plnnrc::push_back(*output, nodes, nodes_size);

            plnnrc::resize(state->scratch, scratch_rewind);
        }
    }

    void push_back(ast::Node* node)
    {
        plnnrc::push_back(state->scratch, node);
    }
};

plnnrc::Parser::Parser()
    : lexer(0)
    , memory(0)
{
    memset(&tree, 0, sizeof(tree));
    memset(&token, 0, sizeof(token));
}

plnnrc::Parser::~Parser()
{
    if (memory)
    {
        destroy(*this);
    }
}

void plnnrc::init(Parser& state, Lexer* lexer, Memory* pool)
{
    plnnrc_assert(lexer);
    plnnrc_assert(pool);
    memset(&state, 0, sizeof(state));
    state.lexer = lexer;
    state.memory = pool;
    plnnrc::init(state.tree, pool);
    plnnrc::init(state.scratch, pool, 1024);
}

void plnnrc::destroy(Parser& state)
{
    plnnrc::destroy(state.scratch);
    plnnrc::destroy(state.tree);
    memset(&state, 0, sizeof(Parser));
}

static inline Token expect(Parser& state, Token_Type token_type)
{
    Token tok = state.token;
    plnnrc_assert(tok.type == token_type);
    (void)(token_type);
    state.token = lex(*state.lexer);
    return tok;
}

static inline Token expect(Parser& state, bool (test_token)(const Token&))
{
    Token tok = state.token;
    plnnrc_assert(test_token(tok));
    (void)(test_token);
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
    // buffer first token.
    state.token = lex(*state.lexer);

    expect(state, Token_Domain);
    Token name = expect(state, Token_Id);
    ast::Domain* domain = plnnrc::create_domain(state.tree, name.value);
    state.tree.domain = domain;
    expect(state, Token_L_Curly);

    plnnrc::clear(state.scratch);

    // parse world & tasks.
    {
        Children_Builder<ast::Task> cb(&state, &domain->tasks);

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

            if (is_Primitive(tok))
            {
                ast::Primitive* prim = parse_primitive(state);
                plnnrc_assert(!state.tree.primitive);
                state.tree.primitive = prim;
                continue;
            }

            if (is_Task(tok))
            {
                ast::Task* task = parse_task(state);
                cb.push_back(task);
                continue;
            }

            plnnrc_assert(false);
        }
    }

    expect(state, Token_Eof);
}

static void parse_facts(Parser& state, Children_Builder<ast::Fact>& builder)
{
    for (;;)
    {
        Token tok = expect(state, Token_Id);
        ast::Fact* fact = plnnrc::create_fact(state.tree, tok.value);
        builder.push_back(fact);

        Children_Builder<ast::Data_Type> cb(&state, &fact->params);
        // parse parameters
        expect(state, Token_L_Paren);
        if (!is_R_Paren(peek(state)))
        {
            for (;;)
            {
                tok = expect(state, is_Type);
                ast::Data_Type* param = plnnrc::create_type(state.tree, tok.type);
                cb.push_back(param);

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

        if (is_R_Curly(peek(state)))
        {
            eat(state);
            break;
        }
    }
}

ast::World* plnnrc::parse_world(Parser& state)
{
    ast::World* world = plnnrc::create_world(state.tree);
    expect(state, Token_L_Curly);
    Children_Builder<ast::Fact> cb(&state, &world->facts);
    parse_facts(state, cb);
    return world;
}

ast::Primitive* plnnrc::parse_primitive(Parser& state)
{
    ast::Primitive* prim = plnnrc::create_primitive(state.tree);
    expect(state, Token_L_Curly);
    Children_Builder<ast::Fact> cb(&state, &prim->tasks);
    parse_facts(state, cb);
    return prim;
}

ast::Task* plnnrc::parse_task(Parser& state)
{
    Token tok = expect(state, Token_Id);
    ast::Task* task = create_task(state.tree, tok.value);

    // parse parameters.
    expect(state, Token_L_Paren);
    if (!is_R_Paren(peek(state)))
    {
        Children_Builder<ast::Param> cb(&state, &task->params);

        for (;;)
        {
            tok = expect(state, Token_Id);
            ast::Param* task_param = plnnrc::create_param(state.tree, tok.value);
            cb.push_back(task_param);

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

    // parse cases
    expect(state, Token_L_Curly);
    if (!is_R_Curly(peek(state)))
    {
        Children_Builder<ast::Case> cb(&state, &task->cases);

        for (;;)
        {
            expect(state, Token_Case);
            ast::Case* case_ = plnnrc::create_case(state.tree);
            cb.push_back(case_);
            case_->task = task;

            ast::Expr* precond = parse_precond(state);
            case_->precond = precond;
            expect(state, Token_Arrow);
            parse_task_list(state, case_);

            if (is_R_Curly(peek(state)))
            {
                eat(state);
                break;
            }
        }
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
        return create_op(state.tree, ast::Node_And);
    }

    ast::Expr* node_Expr = parse_expr(state);
    expect(state, is_R_Paren);
    return node_Expr;
}

void plnnrc::parse_task_list(Parser& state, ast::Case* case_)
{
    expect(state, is_L_Square);
    Children_Builder<ast::Expr> cb(&state, &case_->task_list);
    
    if (!is_R_Square(peek(state)))
    {
        for (;;)
        {
            ast::Expr* node_Task = parse_conjunct(state);
            cb.push_back(node_Task);

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
}

static ast::Expr* parse_expr(Parser& state)
{
    ast::Expr* node = parse_disjunct(state);

    if (is_Or(peek(state)))
    {
        ast::Expr* node_Or = plnnrc::create_op(state.tree, ast::Node_Or);
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
        ast::Expr* node_And = plnnrc::create_op(state.tree, ast::Node_And);
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
        return plnnrc::create_literal(state.tree, tok);
    }

    if (is_Not(tok))
    {
        ast::Expr* node_Not = plnnrc::create_op(state.tree, ast::Node_Not);
        ast::Expr* node = parse_conjunct(state);
        plnnrc::append_child(node_Not, node);
        return node_Not;
    }

    if (is_Id(tok))
    {
        if (is_L_Paren(peek(state)))
        {
            eat(state);

            ast::Func* node = plnnrc::create_func(state.tree, tok.value);

            if (!is_R_Paren(peek(state)))
            {
                for (;;)
                {
                    Token tok = expect(state, Token_Id);
                    ast::Expr* node_Var = plnnrc::create_var(state.tree, tok.value);
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

            return node;
        }

        return plnnrc::create_var(state.tree, tok.value);
    }

    plnnrc_assert(false);
    return 0;
}
