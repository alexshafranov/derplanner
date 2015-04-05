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

    // minimize expression tree depth by collapsing redundant operaion nodes.
    // e.g.: Op { <head...> Op{ <children...> }  <rest...> } -> Op { <head...> <children...> <rest...> }
    void        flatten(ast::Expr* root);

    // converts expression `root` to Negation-Normal-Form.
    ast::Expr*  convert_to_nnf(Parser& state, ast::Expr* root);
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
    Token name = expect(state, Token_Id);
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
        tok = expect(state, Token_Id);
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
    Token tok = expect(state, Token_Id);

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
            tok = expect(state, Token_Id);
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

    // empty expression -> add dummy node.
    if (is_R_Paren(peek(state)))
    {
        ast::Expr* node = allocate_node<ast::Expr>(state);
        node->type = Token_And;
        return node;
    }

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

    if (is_Not(tok))
    {
        ast::Expr* node_Not = allocate_node<ast::Expr>(state);
        node_Not->type = Token_Not;
        ast::Expr* node = parse_conjunct(state);
        plnnrc::append_child(node_Not, node);
        return node_Not;
    }

    if (is_Id(tok))
    {
        ast::Expr* node_Fact = allocate_node<ast::Expr>(state);
        node_Fact->type = tok.type;
        node_Fact->value = tok.value;

        if (is_L_Paren(peek(state)))
        {
            eat(state);

            if (!is_R_Paren(peek(state)))
            {
                for (;;)
                {
                    Token tok = expect(state, Token_Id);
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

ast::Expr* plnnrc::preorder_next(const ast::Expr* root, ast::Expr* current)
{
    ast::Expr* node = current;

    // visit children first.
    if (node->child)
    {
        return node->child;
    }

    // leaf node -> go up until a node with siblings is found.
    while (node != root && !node->next_sibling) { node = node->parent; }

    // done traversal.
    if (node == root)
    {
        return 0;
    }

    return node->next_sibling;
}

void plnnrc::flatten(ast::Expr* root)
{
    plnnrc_assert(root);

    for (ast::Expr* node_U = root; node_U != 0; node_U = preorder_next(root, node_U))
    {
        if (!is_And(node_U->type) && !is_Or(node_U->type))
        {
            continue;
        }

        for (;;)
        {
            bool done = true;

            for (ast::Expr* node_V = node_U->child; node_V != 0; )
            {
                ast::Expr* sibling = node_V->next_sibling;

                // collapse: And{ Id[x] And{ Id[y] } Id[z] } -> And{ Id[x] Id[y] Id[z] }; Or{ Id[x] Or{ Id[y] } Id[z] } -> Or{ Id[x] Id[y] Id[z] }
                if (node_V->type == node_U->type)
                {
                    ast::Expr* after = node_V;

                    // re-parent all children of `node_V` under `node_U`.
                    for (ast::Expr* child = node_V->child; child != 0; )
                    {
                        ast::Expr* next_child = child->next_sibling;
                        plnnrc::unparent(child);
                        plnnrc::insert_child(after, child);
                        after = child;
                        child = next_child;
                    }

                    // now when it's child-less remove `node_V` from the tree.
                    plnnrc::unparent(node_V);
                    done = false;
                }

                node_V = sibling;
            }

            // no nodes where collapsed -> exit.
            if (done)
            {
                break;
            }
        }
    }
}

ast::Expr* plnnrc::convert_to_nnf(Parser& state, ast::Expr* root)
{
    ast::Expr* new_root = root;

    // iterate over all `Not` nodes where argument (i.e. child node) is a logical operation.
    for (ast::Expr* node_Not = root; node_Not != 0; )
    {
        if (!is_Not(node_Not->type))
        {
            node_Not = preorder_next(new_root, node_Not);
            continue;
        }

        ast::Expr* node_Logical = node_Not->child;
        plnnrc_assert(node_Logical != 0);

        // eliminate double negation: Not { Not { <expr...> } } -> <expr...>
        if (is_Not(node_Logical->type))
        {
            ast::Expr* expr = node_Logical->child;
            plnnrc_assert(expr != 0);

            // get rid of `node_Not` and `node_Logical`, replacing `node_Not` with `expr`.
            plnnrc::unparent(expr);

            if (node_Not->parent)
            {
                plnnrc::insert_child(node_Not, expr);
                plnnrc::unparent(node_Not);
            }

            // update root if it's being replaced.
            if (node_Not == new_root)
            {
                new_root = expr;
            }

            // now consider `expr` in next iteration.
            node_Not = expr;
            continue;
        }

        // De-Morgan's law:
        //      Not{ Or{ <x...> <y...> } }  -> And{ Not{ <x...> } Not{ <y...> } }
        //      Not{ And{ <x...> <y...> } } ->  Or{ Not{ <x...> } Not{ <y...> } }
        if (is_And(node_Logical->type) || is_Or(node_Logical->type))
        {
            // node_Not becomes `And` or `Or`.
            ast::Expr* node_Op = node_Not;
            node_Op->type = is_And(node_Logical->type) ? Token_Or : Token_And;

            // all chilren of `node_Logical` are parented under a new `Not` which is parented under `node_Op`.
            ast::Expr* after = node_Logical;
            for (ast::Expr* expr = node_Logical->child; expr != 0; )
            {
                ast::Expr* next_expr = expr->next_sibling;
                plnnrc::unparent(expr);

                ast::Expr* new_Not = allocate_node<ast::Expr>(state);
                new_Not->type = Token_Not;
                plnnrc::append_child(new_Not, expr);
                plnnrc::insert_child(after, new_Not);
                after = new_Not;

                expr = next_expr;
            }

            // `node_Logical` now has no children and can be safely unparented.
            plnnrc::unparent(node_Logical);
        }

        // move to next.
        node_Not = preorder_next(new_root, node_Not);
    }

    return new_root;
}

static bool         is_conjunct(ast::Expr* node);
static void         distribute_and(Parser& state, ast::Expr* root);
static ast::Expr*   convert_to_dnf_or(Parser& state, ast::Expr* root);

ast::Expr* plnnrc::convert_to_dnf(Parser& state, ast::Expr* root)
{
    // convert `root` to Negative-Normal-Form and put it under a new Or node.
    ast::Expr* nnf_root = convert_to_nnf(state, root);
    ast::Expr* new_root = allocate_node<ast::Expr>(state);
    new_root->type = Token_Or;
    plnnrc::append_child(new_root, nnf_root);
    flatten(new_root);

    // now we have flattened Or expression
    // convert it to DNF form:
    // Expr = C0 | C1 | ... | CN, Ck (conjunct) = either ~X or X where X is variable or fact.
    new_root = convert_to_dnf_or(state, new_root);
    return new_root;
}

// convert Or expression to DNF by applying distributive law repeatedly, until no change could be made.
static ast::Expr* convert_to_dnf_or(Parser& state, ast::Expr* root)
{
    plnnrc_assert(root && is_Or(root->type));

    for (;;)
    {
        bool done = true;

        for (ast::Expr* arg = root->child; arg != 0; )
        {
            ast::Expr* next_arg = arg->next_sibling;

            if (!is_conjunct(arg))
            {
                done = false;
                distribute_and(state, arg);
            }

            arg = next_arg;
        }

        // all arguments of root `Or` are conjuncts (literal or conjunction of literals) -> exit.
        if (done)
        {
            break;
        }
    }

    return root;
}

// check if trivial conjunct `~x` or `x`. expression is assumed to be in Negative-Normal-Form.
static inline bool is_trivial_conjunct(ast::Expr* node)
{
    // assert NNF.
    plnnrc_assert(!is_Not(node->type) || !is_Logical(node->child->type));
    return is_Not(node->type) || is_Id(node->type);
}

static inline bool is_conjunct(ast::Expr* node)
{
    if (is_trivial_conjunct(node))
    {
        return true;
    }

    // check if conjunction trivials.
    if (!is_And(node->type))
    {
        return false;
    }

    for (ast::Expr* arg = node->child; arg != 0; arg = arg->next_sibling)
    {
        if (!is_trivial_conjunct(arg))
        {
            return false;
        }
    }

    return true;
}

static ast::Expr* clone_tree(Parser& state, ast::Expr* root)
{
    ast::Expr* root_clone = allocate_node<ast::Expr>(state);
    root_clone->type = root->type;
    root_clone->value = root->value;

    for (ast::Expr* child = root->child; child != 0; child = child->next_sibling)
    {
        ast::Expr* child_clone = clone_tree(state, child);
        plnnrc::append_child(root_clone, child_clone);
    }

    return root_clone;
}

// apply distributive law to make `Or` root of the expression.
static void distribute_and(Parser& state, ast::Expr* node_And)
{
    plnnrc_assert(node_And && is_And(node_And->type));

    // find the first `Or` argument of `node_And`.
    ast::Expr* node_Or = node_And->child;
    for ( ; node_Or != 0; node_Or = node_Or->next_sibling)
    {
        if (is_Or(node_Or->type))
        {
            break;
        }
    }

    plnnrc_assert(node_Or);

    ast::Expr* after = node_And;
    for (ast::Expr* or_arg = node_Or->child; or_arg != 0; )
    {
        ast::Expr* next_or_arg = or_arg->next_sibling;
        ast::Expr* new_And = allocate_node<ast::Expr>(state);
        new_And->type = Token_And;

        for (ast::Expr* and_arg = node_And->child; and_arg != 0; )
        {
            ast::Expr* next_and_arg = and_arg->next_sibling;

            if (and_arg != node_Or)
            {
                ast::Expr* and_arg_clone = clone_tree(state, and_arg);
                plnnrc::append_child(new_And, and_arg_clone);
            }
            else
            {
                plnnrc::unparent(or_arg);
                plnnrc::append_child(new_And, or_arg);
            }

            and_arg = next_and_arg;
        }

        plnnrc::flatten(new_And);
        plnnrc::insert_child(after, new_And);
        after = new_And;
        or_arg = next_or_arg;
    }

    plnnrc::unparent(node_And);
}
