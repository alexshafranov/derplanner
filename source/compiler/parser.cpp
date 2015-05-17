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

#include <stdio.h>

#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/parser.h"

// implementation (exposed for unit tests)
namespace plnnrc
{
    ast::Domain*        parse_domain(Parser& state);
    ast::World*         parse_world(Parser& state);
    ast::Primitive*     parse_primitive(Parser& state);
    ast::Task*          parse_task(Parser& state);
    ast::Expr*          parse_precond(Parser& state);
    void                parse_task_list(Parser& state, ast::Case* case_);
    void                parse_predicate(Parser& state, Array<ast::Predicate*>& output);
}

using namespace plnnrc;

// RAII helper to keep AST child nodes allocated in temp memory, moving to persistent in destructor.
template <typename T>
struct Children_Builder
{
    Parser*             state;
    Array<T*>*          output;
    Memory_Stack_Scope  mem_scope;
    Array<T*>           nodes;

    Children_Builder(Parser* state, Array<T*>* output)
        : state(state)
        , output(output)
        , mem_scope(state->scratch)
    {
        init(nodes, state->scratch, 16);
    }

    void push_back(T* node)
    {
        plnnrc::push_back(nodes, node);
    }

    ~Children_Builder()
    {
        const uint32_t node_count = size(nodes);
        if (!node_count)
        {
            return;
        }

        if (!output->memory)
        {
            init(*output, state->tree->pool, node_count);
        }

        if (node_count > 0)
        {
            plnnrc::push_back(*output, &nodes[0], node_count);
        }
    }
};

void plnnrc::init(Parser& state, Lexer* lexer, ast::Root* tree, Memory_Stack* scratch)
{
    plnnrc_assert(lexer);
    plnnrc_assert(tree);
    plnnrc_assert(scratch);

    memset(&state, 0, sizeof(state));
    state.lexer = lexer;
    state.tree = tree;
    state.scratch = scratch;
}

static inline Token make_error_token(Parser& state)
{
    Token tok;
    tok.type = Token_Error;
    tok.loc = get_loc(*state.lexer);
    const char* error_str = "<error>";
    tok.value.str = error_str;
    tok.value.length = sizeof(error_str) - 1;
    return tok;
}

static inline Token expect(Parser& state, Token_Type token_type)
{
    Token tok = state.token;
    if (tok.type != token_type)
    {
        return make_error_token(state);
    }

    state.token = lex(*state.lexer);
    return tok;
}

static inline Token expect(Parser& state, Token_Group token_group)
{
    Token tok = state.token;
    if (tok.type < get_group_first(token_group) || tok.type > get_group_last(token_group))
    {
        return make_error_token(state);
    }

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

static inline Error& emit(Parser& state, Error_Type error_type)
{
    Error err;
    init(err, error_type, get_loc(*state.lexer));
    push_back(state.errs, err);
    return back(state.errs);
}

static inline Token_Type recover(Parser& state, const Token_Type* sync_tokens)
{
    for (;;)
    {
        Token tok = peek(state);
        for (const Token_Type* sync = sync_tokens; *sync != Token_Unknown; ++sync)
        {
            if (tok.type == *sync)
            {
                return *sync;
            }
        }

        if (is_Eof(tok))
        {
            return Token_Eof;
        }

        eat(state);
    }
}

void plnnrc::parse(Parser& state)
{
    // buffer the first token.
    state.token = lex(*state.lexer);
    // initialize errors.
    init(state.errs, state.tree->pool, 16);

    if (is_Error(expect(state, Token_Domain)))
    {
        emit(state, Error_Expected_Declaration) << Token_Domain;
        return;
    }

    plnnrc::parse_domain(state);
}

ast::Domain* plnnrc::parse_domain(Parser& state)
{
    Token name = expect(state, Token_Id);
    if (is_Error(name))
    {
        emit(state, Error_Expected_After) << Token_Id << Token_Domain << peek(state);
    }

    ast::Domain* domain = plnnrc::create_domain(state.tree, name.value);
    state.tree->domain = domain;

    if (is_Error(expect(state, Token_L_Curly)))
    {
        emit(state, Error_Expected) << Token_L_Curly << peek(state);
        Token_Type sync[] = { Token_L_Curly, Token_World, Token_Primitive, Token_Task, Token_Predicate, Token_Unknown };
        recover(state, sync);
    }

    // parse world & tasks.
    {
        Children_Builder<ast::Task> cb(&state, &domain->tasks);

        while (!is_Eof(peek(state)))
        {
            Token tok = eat(state);

            // closing domain block.
            if (is_R_Curly(tok))
            {
                break;
            }

            if (is_World(tok))
            {
                ast::World* world = parse_world(state);
                if (state.tree->world)
                {
                    emit(state, Error_Redefinition) << Token_World;
                }

                state.tree->world = world;
                continue;
            }

            if (is_Primitive(tok))
            {
                ast::Primitive* prim = parse_primitive(state);
                if (state.tree->primitive)
                {
                    emit(state, Error_Redefinition) << Token_Primitive;
                }

                state.tree->primitive = prim;
                continue;
            }

            if (is_Predicate(tok))
            {
                parse_predicate(state, domain->predicates);
                continue;
            }

            if (is_Task(tok))
            {
                ast::Task* task = parse_task(state);
                cb.push_back(task);
                continue;
            }

            emit(state, Error_Unexpected_Token) << tok;
            {
                Token_Type sync[] = { Token_World, Token_Primitive, Token_Task, Token_Predicate, Token_Unknown };
                recover(state, sync);
            }
        }
    }

    if (is_Error(expect(state, Token_Eof)))
    {
        emit(state, Error_Expected) << Token_Eof << peek(state);
    }

    return domain;
}

static void parse_facts(Parser& state, Children_Builder<ast::Fact>& builder)
{
    if (is_R_Curly(peek(state)))
    {
        eat(state);
        return;
    }

    for (;;)
    {
        Token tok = expect(state, Token_Id);
        if (is_Error(tok))
        {
            emit(state, Error_Expected) << Token_Id << peek(state);
            Token_Type sync[] = { Token_R_Curly, Token_Unknown };
            recover(state, sync);
            return;
        }

        ast::Fact* fact = plnnrc::create_fact(state.tree, tok.value);
        // parse param types.
        {
            // parse parameters
            if (is_Error(expect(state, Token_L_Paren)))
            {
                Token_Type sync[] = { Token_R_Curly, Token_Unknown };
                recover(state, sync);
                return;
            }

            Children_Builder<ast::Data_Type> param_builder(&state, &fact->params);

            if (!is_R_Paren(peek(state)))
            {
                for (;;)
                {
                    tok = expect(state, Token_Group_Type);
                    if (is_Error(tok))
                    {
                        emit(state, Error_Expected) << Token_Group_Type << peek(state);
                    }

                    ast::Data_Type* param = plnnrc::create_type(state.tree, tok.type);
                    param_builder.push_back(param);

                    if (!is_Comma(peek(state)))
                    {
                        expect(state, Token_R_Paren);
                        break;
                    }

                    expect(state, Token_Comma);
                }
            }
            else
            {
                eat(state);
            }
        }

        builder.push_back(fact);

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
    if (is_Error(expect(state, Token_L_Curly)))
    {
        emit(state, Error_Expected_After) << Token_L_Curly << Token_World << peek(state);

        Token_Type sync[] = { Token_L_Curly, Token_Unknown };
        if (is_Eof(recover(state, sync)))
        {
            return world;
        }
    }

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

static void parse_params(Parser& state, Array<ast::Param*>& output)
{
    // parse parameters.
    expect(state, Token_L_Paren);
    if (!is_R_Paren(peek(state)))
    {
        Children_Builder<ast::Param> cb(&state, &output);

        for (;;)
        {
            Token tok = expect(state, Token_Id);
            ast::Param* param = plnnrc::create_param(state.tree, tok.value);
            cb.push_back(param);

            if (!is_Comma(peek(state)))
            {
                expect(state, Token_R_Paren);
                break;
            }

            expect(state, Token_Comma);
        }
    }
    else
    {
        eat(state);
    }
}

ast::Task* plnnrc::parse_task(Parser& state)
{
    Token tok = expect(state, Token_Id);
    ast::Task* task = create_task(state.tree, tok.value);

    if (is_Error(tok))
    {
        emit(state, Error_Expected_After) << Token_Id << Token_Task << peek(state);
        Token_Type sync[] = { Token_L_Curly, Token_Unknown };
        recover(state, sync);
        return task;
    }

    parse_params(state, task->params);

    // parse task block
    expect(state, Token_L_Curly);
    if (!is_R_Curly(peek(state)))
    {
        Children_Builder<ast::Case> cases_builder(&state, &task->cases);

        for (;;)
        {
            if (is_Case(peek(state)))
            {
                eat(state);
                ast::Case* case_ = plnnrc::create_case(state.tree);
                cases_builder.push_back(case_);
                case_->task = task;

                ast::Expr* precond = parse_precond(state);
                case_->precond = precond;
                expect(state, Token_Arrow);
                parse_task_list(state, case_);
                continue;
            }

            if (is_Predicate(peek(state)))
            {
                eat(state);
                parse_predicate(state, task->predicates);
                continue;
            }

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
    expect(state, Token_L_Paren);

    // empty expression -> add dummy node.
    if (is_R_Paren(peek(state)))
    {
        eat(state);
        return create_op(state.tree, ast::Node_And);
    }

    ast::Expr* node_Expr = parse_expr(state);
    expect(state, Token_R_Paren);
    return node_Expr;
}

void plnnrc::parse_task_list(Parser& state, ast::Case* case_)
{
    expect(state, Token_L_Square);
    Children_Builder<ast::Expr> cb(&state, &case_->task_list);
    
    if (!is_R_Square(peek(state)))
    {
        for (;;)
        {
            ast::Expr* node_Task = parse_conjunct(state);
            cb.push_back(node_Task);

            if (!is_Comma(peek(state)))
            {
                expect(state, Token_R_Square);
                break;
            }

            expect(state, Token_Comma);
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
        expect(state, Token_R_Paren);
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
                        expect(state, Token_R_Paren);
                        break;
                    }

                    expect(state, Token_Comma);
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

static ast::Predicate* parse_single_predicate(Parser& state)
{
    Token tok = expect(state, Token_Id);
    ast::Predicate* node_Pred = create_predicate(state.tree, tok.value);
    parse_params(state, node_Pred->params);
    expect(state, Token_Equality);
    node_Pred->expression = parse_precond(state);
    return node_Pred;
}

void plnnrc::parse_predicate(Parser& state, Array<ast::Predicate*>& output)
{
    Children_Builder<ast::Predicate> cb(&state, &output);

    // block with predicates
    if (is_L_Curly(peek(state)))
    {
        eat(state);

        for (;;)
        {
            if (!is_R_Paren(peek(state)))
            {
                ast::Predicate* node_Pred = parse_single_predicate(state);
                cb.push_back(node_Pred);
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

        return;
    }

    // single predicate definition
    ast::Predicate* node_Pred = parse_single_predicate(state);
    cb.push_back(node_Pred);
}
