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
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/parser.h"

using namespace plnnrc;

// main parsing functions, exposed for unit tests
namespace plnnrc
{
    ast::Domain*        parse_domain(Parser& state);
    ast::World*         parse_world(Parser& state);
    ast::Primitive*     parse_primitive(Parser& state);
    ast::Task*          parse_task(Parser& state);
    ast::Expr*          parse_precond(Parser& state);
}

template <typename T>
struct Children_Builder;

static ast::Expr* parse_expr(Parser& state);
static ast::Expr* parse_disjunct(Parser& state);
static ast::Expr* parse_conjunct(Parser& state);

static ast::Predicate* parse_single_predicate(Parser& state);

static bool parse_param_types(Parser& state, Children_Builder<ast::Data_Type>& builder);
static bool parse_params(Parser& state, Children_Builder<ast::Param>& builder);
static bool parse_facts(Parser& state, Children_Builder<ast::Fact>& builder);
static bool parse_task_body(Parser& state, ast::Task* task);
static bool parse_task_list(Parser& state, ast::Case* case_);
static bool parse_predicate_block(Parser& state, Children_Builder<ast::Predicate>& builder);
static bool parse_predicates(Parser& state, Children_Builder<ast::Predicate>& builder);


void plnnrc::init(Parser& state, Lexer* lexer, ast::Root* tree, Array<Error>* errors, Memory_Stack* scratch)
{
    plnnrc_assert(lexer);
    plnnrc_assert(tree);
    plnnrc_assert(scratch);

    memset(&state, 0, sizeof(state));
    state.lexer = lexer;
    state.tree = tree;
    state.errs = errors;
    state.scratch = scratch;
}

static Token peek(Parser& state)
{
    return state.token;
}

static Token eat(Parser& state)
{
    Token tok = state.token;
    state.token = lex(*state.lexer);
    return tok;
}

static Token make_error_token(Parser& state, Token_Type type)
{
    Token tok;
    tok.error = true;
    tok.type = type;
    tok.loc = get_loc(*state.lexer);
    const char* error_str = "<error>";
    tok.value.str = error_str;
    tok.value.length = sizeof(error_str) - 1;
    return tok;
}

static Error& emit(Parser& state, Error_Type error_type)
{
    Error err;
    init(err, error_type, get_loc(*state.lexer));
    push_back(*state.errs, err);
    return back(*state.errs);
}

static Token expect(Parser& state, Token_Type token_type)
{
    if (peek(state).type != token_type)
    {
        emit(state, Error_Expected) << token_type << peek(state);
        eat(state);
        return make_error_token(state, token_type);
    }

    return eat(state);
}

static Token expect(Parser& state, Token_Group token_group)
{
    Token_Type actual_type = peek(state).type;
    if (actual_type < get_group_first(token_group) || actual_type > get_group_last(token_group))
    {
        emit(state, Error_Expected) << token_group << peek(state);
        eat(state);
        return make_error_token(state, get_group_first(token_group));
    }

    return eat(state);
}

// error recovery: look for sync tokens inside the domain scope.
static bool skip_inside_domain(Parser& state)
{
    eat(state);

    while (!is_Eof(peek(state)))
    {
        Token tok = peek(state);
        if (is_World(tok) || is_Primitive(tok) || is_Predicate(tok) || is_Task(tok))
            return true;

        eat(state);
    }

    return false;
}

// error recovery: look for sync tokens inside the task scope.
static bool skip_inside_task(Parser& state)
{
    eat(state);

    while (!is_Eof(peek(state)))
    {
        Token tok = peek(state);
        if (is_Case(tok) || is_Predicate(tok))
            return true;

        eat(state);
    }

    return false;
}

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

void plnnrc::parse(Parser& state)
{
    // buffer the first token.
    state.token = lex(*state.lexer);
    ast::Domain* domain = plnnrc::parse_domain(state);

    if (!domain)
    {
        Token tok = make_error_token(state, Token_Id);
        state.tree->domain = create_domain(state.tree, tok.value);
    }

    if (!state.tree->world)
    {
        state.tree->world = create_world(state.tree);
    }

    if (!state.tree->primitive)
    {
        state.tree->primitive = create_primitive(state.tree);
    }

    if (domain && !is_Eof(peek(state)))
    {
        emit(state, Error_Expected_End_Of_File);
    }
}

#define plnnrc_check_return(COND)               \
    if (!(COND))                                \
    {                                           \
        return 0;                               \
    }                                           \

#define plnnrc_check_skip(STATE, NODE, SKIP)    \
    if (!NODE)                                  \
    {                                           \
        if (!SKIP(state))                       \
            return 0;                           \
                                                \
        continue;                               \
    }                                           \

#define plnnrc_expect_return(STATE, ARG)        \
    if (is_Error(expect(STATE, ARG)))           \
    {                                           \
        return 0;                               \
    }                                           \

ast::Domain* plnnrc::parse_domain(Parser& state)
{
    plnnrc_expect_return(state, Token_Domain);
    Token name = expect(state, Token_Id);
    plnnrc_check_return(!is_Error(name));

    ast::Domain* domain = plnnrc::create_domain(state.tree, name.value);
    state.tree->domain = domain;

    plnnrc_expect_return(state, Token_L_Curly);
    {
        Children_Builder<ast::Task> task_builder(&state, &domain->tasks);

        while (!is_Eof(peek(state)))
        {
            Token tok = peek(state);

            if (is_R_Curly(tok))
            {
                break;
            }

            if (is_World(tok))
            {
                ast::World* world = parse_world(state);
                plnnrc_check_skip(state, world, skip_inside_domain);

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
                plnnrc_check_skip(state, prim, skip_inside_domain);

                if (state.tree->primitive)
                {
                    emit(state, Error_Redefinition) << Token_Primitive;
                }

                state.tree->primitive = prim;
                continue;
            }

            if (is_Predicate(tok))
            {
                Children_Builder<ast::Predicate> pred_builder(&state, &domain->predicates);
                bool ok = parse_predicates(state, pred_builder);
                plnnrc_check_skip(state, ok, skip_inside_domain);
                continue;
            }

            if (is_Task(tok))
            {
                ast::Task* task = parse_task(state);
                plnnrc_check_skip(state, task, skip_inside_domain)
                task_builder.push_back(task);
                continue;
            }

            emit(state, Error_Unexpected_Token) << tok;
            skip_inside_domain(state);
        }
    }

    plnnrc_expect_return(state, Token_R_Curly);
    return domain;
}

ast::World* plnnrc::parse_world(Parser& state)
{
    plnnrc_expect_return(state, Token_World);
    ast::World* world = plnnrc::create_world(state.tree);
    Children_Builder<ast::Fact> builder(&state, &world->facts);
    plnnrc_check_return(parse_facts(state, builder));
    return world;
}

ast::Primitive* plnnrc::parse_primitive(Parser& state)
{
    plnnrc_expect_return(state, Token_Primitive);
    ast::Primitive* prim = plnnrc::create_primitive(state.tree);
    Children_Builder<ast::Fact> builder(&state, &prim->tasks);
    plnnrc_check_return(parse_facts(state, builder));
    return prim;
}

ast::Task* plnnrc::parse_task(Parser& state)
{
    plnnrc_expect_return(state, Token_Task);
    Token tok = expect(state, Token_Id);
    plnnrc_check_return(!is_Error(tok));
    ast::Task* task = create_task(state.tree, tok.value);
    Children_Builder<ast::Param> param_builder(&state, &task->params);
    plnnrc_check_return(parse_params(state, param_builder));
    plnnrc_check_return(parse_task_body(state, task));
    return task;
}

ast::Expr* plnnrc::parse_precond(Parser& state)
{
    plnnrc_expect_return(state, Token_L_Paren);

    // empty expression -> add dummy node.
    if (is_R_Paren(peek(state)))
    {
        eat(state);
        return create_op(state.tree, ast::Node_And);
    }

    ast::Expr* node_Expr = parse_expr(state);
    plnnrc_check_return(node_Expr);
    plnnrc_expect_return(state, Token_R_Paren);
    return node_Expr;
}

static bool parse_param_types(Parser& state, Children_Builder<ast::Data_Type>& builder)
{
    plnnrc_expect_return(state, Token_L_Paren);

    while (!is_Eof(peek(state)))
    {
        if (is_R_Paren(peek(state)))
        {
            break;
        }

        Token tok = expect(state, Token_Group_Type);
        plnnrc_check_return(!is_Error(tok));

        ast::Data_Type* param = create_type(state.tree, tok.type);
        builder.push_back(param);

        if (!is_Comma(peek(state)))
        {
            break;
        }

        plnnrc_expect_return(state, Token_Comma);
    }

    plnnrc_expect_return(state, Token_R_Paren);
    return true;
}

static bool parse_params(Parser& state, Children_Builder<ast::Param>& builder)
{
    plnnrc_expect_return(state, Token_L_Paren);

    while (!is_Eof(peek(state)))
    {
        if (is_R_Paren(peek(state)))
        {
            break;
        }

        Token tok = expect(state, Token_Id);
        plnnrc_check_return(!is_Error(tok));

        ast::Param* param = create_param(state.tree, tok.value);
        builder.push_back(param);

        if (!is_Comma(peek(state)))
        {
            break;
        }

        plnnrc_expect_return(state, Token_Comma);
    }

    plnnrc_expect_return(state, Token_R_Paren);
    return true;
}

static bool parse_facts(Parser& state, Children_Builder<ast::Fact>& builder)
{
    plnnrc_expect_return(state, Token_L_Curly);

    while (!is_Eof(peek(state)))
    {
        if (is_R_Curly(peek(state)))
        {
            break;
        }

        Token tok = expect(state, Token_Id);
        plnnrc_check_return(!is_Error(tok));

        ast::Fact* fact = plnnrc::create_fact(state.tree, tok.value);

        Children_Builder<ast::Data_Type> param_builder(&state, &fact->params);
        plnnrc_check_return(parse_param_types(state, param_builder));

        builder.push_back(fact);
    }

    plnnrc_expect_return(state, Token_R_Curly);
    return true;
}

static bool parse_task_body(Parser& state, ast::Task* task)
{
    plnnrc_expect_return(state, Token_L_Curly);

    Children_Builder<ast::Case> cases_builder(&state, &task->cases);

    while (!is_Eof(peek(state)))
    {
        Token tok = peek(state);

        if (is_R_Curly(tok))
        {
            break;
        }

        if (is_Case(tok))
        {
            eat(state);
            ast::Case* case_ = create_case(state.tree);
            cases_builder.push_back(case_);
            case_->task = task;

            ast::Expr* precond = parse_precond(state);
            plnnrc_check_skip(state, precond, skip_inside_task);

            case_->precond = precond;
            plnnrc_expect_return(state, Token_Arrow);
            bool ok = parse_task_list(state, case_);
            plnnrc_check_skip(state, ok, skip_inside_task);
            continue;
        }

        if (is_Predicate(tok))
        {
            Children_Builder<ast::Predicate> pred_builder(&state, &task->predicates);
            bool ok = parse_predicates(state, pred_builder);
            plnnrc_check_skip(state, ok, skip_inside_task);
            continue;
        }

        emit(state, Error_Unexpected_Token) << tok;
        skip_inside_task(state);
    }

    plnnrc_expect_return(state, Token_R_Curly);
    return true;
}

static bool parse_task_list(Parser& state, ast::Case* case_)
{
    plnnrc_expect_return(state, Token_L_Square);
    Children_Builder<ast::Expr> builder(&state, &case_->task_list);

    while (!is_Eof(peek(state)))
    {
        Token tok = peek(state);

        if (is_R_Square(tok))
        {
            break;
        }

        ast::Expr* node_Task = parse_conjunct(state);
        builder.push_back(node_Task);

        if (!is_Comma(peek(state)))
        {
            break;
        }

        plnnrc_expect_return(state, Token_Comma);
    }

    plnnrc_expect_return(state, Token_R_Square);
    return true;
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
        plnnrc_expect_return(state, Token_R_Paren);
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

            while (!is_Eof(peek(state)))
            {
                Token tok = peek(state);

                if (is_R_Paren(tok))
                {
                    break;
                }

                if (is_Id(tok))
                {
                    eat(state);
                    ast::Expr* node_Var = plnnrc::create_var(state.tree, tok.value);
                    plnnrc::append_child(node, node_Var);
                    continue;
                }

                if (is_Comma(tok))
                {
                    eat(state);
                    continue;
                }

                emit(state, Error_Unexpected_Token) << tok;
                eat(state);
                break;
            }

            plnnrc_expect_return(state, Token_R_Paren);
            return node;
        }

        return plnnrc::create_var(state.tree, tok.value);
    }

    return 0;
}

static ast::Predicate* parse_single_predicate(Parser& state)
{
    Token tok = expect(state, Token_Id);
    ast::Predicate* pred = create_predicate(state.tree, tok.value);

    Children_Builder<ast::Param> param_builder(&state, &pred->params);
    parse_params(state, param_builder);
    plnnrc_expect_return(state, Token_Equality);
    pred->expression = parse_precond(state);
    return pred;
}

static bool parse_predicate_block(Parser& state, Children_Builder<ast::Predicate>& builder)
{
    plnnrc_expect_return(state, Token_L_Curly);

    while (!is_Eof(peek(state)))
    {
        Token tok = peek(state);

        if (is_R_Curly(tok))
        {
            break;
        }

        if (is_Id(tok))
        {
            ast::Predicate* pred = parse_single_predicate(state);
            builder.push_back(pred);
            continue;
        }

        emit(state, Error_Unexpected_Token) << tok;
        eat(state);
        break;
    }

    plnnrc_expect_return(state, Token_R_Curly);
    return true;
}

static bool parse_predicates(Parser& state, Children_Builder<ast::Predicate>& builder)
{
    plnnrc_expect_return(state, Token_Predicate);

    if (is_L_Curly(peek(state)))
    {
        return parse_predicate_block(state, builder);
    }

    // single predicate definition
    ast::Predicate* pred = parse_single_predicate(state);
    builder.push_back(pred);
    return true;
}
