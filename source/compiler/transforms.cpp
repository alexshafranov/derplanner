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

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/io.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/transforms.h"
#include "pool.h"

using namespace plnnrc;

// implementation (exposed for unit tests)
namespace plnnrc
{
    // minimize expression tree depth by collapsing redundant operaion nodes.
    // e.g.: Op { <head...> Op{ <children...> }  <rest...> } -> Op { <head...> <children...> <rest...> }
    void        flatten(ast::Expr* root);

    // converts expression `root` to Negation-Normal-Form.
    ast::Expr*  convert_to_nnf(ast::Root& tree, ast::Expr* root);
}

plnnrc::ast::Root::Root()
    : pool(0)
{
}

plnnrc::ast::Root::~Root()
{
    if (pool)
    {
        plnnrc::destroy(*this);
    }
}

void plnnrc::init(ast::Root& root)
{
    // randomly chosen initial number of facts.
    init(root.fact_lookup, 1024);
    // randomly chosen initial number of tasks.
    init(root.task_lookup, 1024);
    // pool will have 1MB sized pages.
    root.pool = create_paged_pool(1024*1024);
}

void plnnrc::destroy(ast::Root& root)
{
    destroy(root.pool);
    destroy(root.task_lookup);
    destroy(root.fact_lookup);
    memset(&root, 0, sizeof(root));
}

void plnnrc::build_lookups(ast::Root& root)
{
    if (root.world)
    {
        for (ast::Fact_Type* fact = root.world->facts; fact != 0; fact = fact->next)
        {
            plnnrc_assert(!plnnrc::get_fact(root, fact->name)); // error: fact is already defined.
            plnnrc::set(root.fact_lookup, fact->name, fact);
        }
    }

    if (root.domain)
    {
        for (ast::Task* task = root.domain->tasks; task != 0; task = task->next)
        {
            plnnrc_assert(!plnnrc::get_task(root, task->name)); // error: multiple definitions of task.
            plnnrc::set(root.task_lookup, task->name, task);
        }
    }
}

template <typename T>
static inline T* allocate_node(plnnrc::ast::Root& tree)
{
    T* result = static_cast<T*>(plnnrc::allocate(tree.pool, sizeof(T), plnnrc_alignof(T)));
    memset(result, 0, sizeof(T));
    return result;
}

ast::World* plnnrc::create_world(ast::Root& tree)
{
    return allocate_node<ast::World>(tree);
}

ast::Fact_Type* plnnrc::create_fact_type(ast::Root& tree, ast::Fact_Type* previous)
{
    ast::Fact_Type* node = allocate_node<ast::Fact_Type>(tree);
    if (previous) { previous->next = node; }
    return node;
}

ast::Fact_Param* plnnrc::create_fact_param(ast::Root& tree, ast::Fact_Param* previous)
{
    ast::Fact_Param* node = allocate_node<ast::Fact_Param>(tree);
    if (previous) { previous->next = node; }
    return node;
}

ast::Domain* plnnrc::create_domain(ast::Root& tree)
{
    return allocate_node<ast::Domain>(tree);
}

ast::Task* plnnrc::create_task(ast::Root& tree, const Token_Value& name)
{
    ast::Task* node = allocate_node<ast::Task>(tree);
    node->name = name;
    return node;
}

ast::Task_Param* plnnrc::create_task_param(ast::Root& tree, const Token_Value& name, ast::Task_Param* previous)
{
    ast::Task_Param* node = allocate_node<ast::Task_Param>(tree);
    node->name = name;
    if (previous) { previous->next = node; }
    return node;
}

ast::Case* plnnrc::create_case(ast::Root& tree)
{
    return allocate_node<ast::Case>(tree);
}

ast::Expr* plnnrc::create_expr(ast::Root& tree, Token_Type type)
{
    ast::Expr* node = allocate_node<ast::Expr>(tree);
    node->type = type;
    return node;
}

ast::Fact_Type* plnnrc::get_fact(ast::Root& tree, const Token_Value& token_value)
{
    ast::Fact_Type* const* ptr = plnnrc::get(tree.fact_lookup, token_value.str, token_value.length);

    if (ptr)
    {
        return *ptr;
    }

    return 0;
}

ast::Task* plnnrc::get_task(ast::Root& tree, const Token_Value& token_value)
{
    ast::Task* const* ptr = plnnrc::get(tree.task_lookup, token_value.str, token_value.length);

    if (ptr)
    {
        return *ptr;
    }

    return 0;
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

ast::Expr* plnnrc::convert_to_nnf(ast::Root& tree, ast::Expr* root)
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

                ast::Expr* new_Not = create_expr(tree, Token_Not);
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
static void         distribute_And_over_Or(ast::Root& tree, ast::Expr* root);
static ast::Expr*   convert_Or_to_dnf(ast::Root& tree, ast::Expr* root);

ast::Expr* plnnrc::convert_to_dnf(ast::Root& tree, ast::Expr* root)
{
    // convert `root` to Negative-Normal-Form and put it under a new Or node.
    ast::Expr* nnf_root = convert_to_nnf(tree, root);
    ast::Expr* new_root = create_expr(tree, Token_Or);
    plnnrc::append_child(new_root, nnf_root);
    flatten(new_root);

    // now we have flattened Or expression
    // convert it to DNF form:
    // Expr = C0 | C1 | ... | CN, Ck (conjunct) = either ~X or X where X is variable or fact.
    new_root = convert_Or_to_dnf(tree, new_root);
    return new_root;
}

// convert Or expression to DNF by applying distributive law repeatedly, until no change could be made.
static ast::Expr* convert_Or_to_dnf(ast::Root& tree, ast::Expr* root)
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
                distribute_And_over_Or(tree, arg);
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
    // assert expression is NNF.
    plnnrc_assert(!is_Not(node->type) || !is_Logical(node->child->type));
    return is_Not(node->type) || is_Var(node->type) || is_Fact(node->type);
}

// check if expression is either trivial (~x, x) or conjunction of trivials.
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

static inline ast::Expr* clone_node(ast::Root& tree, ast::Expr* node)
{
    ast::Expr* clone = create_expr(tree, node->type);
    clone->value = node->value;
    return clone;
}

static ast::Expr* clone_tree(ast::Root& tree, ast::Expr* root)
{
    ast::Expr* root_clone = clone_node(tree, root);

    ast::Expr* node  = root;
    ast::Expr* clone = root_clone;

    for (;;)
    {
        if (node->child)
        {
            ast::Expr* child = node->child;
            ast::Expr* child_clone = clone_node(tree, child);
            plnnrc::append_child(clone, child_clone);
            node = child;
            clone = child_clone;
            continue;
        }

        while (node != root && !node->next_sibling)
        {
            node = node->parent;
            clone = clone->parent;
        }

        if (node == root)
        {
            break;
        }

        ast::Expr* sibling = node->next_sibling;
        ast::Expr* sibling_clone = clone_node(tree, sibling);
        plnnrc::insert_child(clone, sibling_clone);

        node = sibling;
        clone = sibling_clone;
    }

    return root_clone;
}

static inline ast::Expr* find_child(ast::Expr* root, Token_Type type)
{
    for (ast::Expr* node = root->child; node != 0; node = node->next_sibling)
    {
        if (node->type == type)
        {
            return node;
        }
    }

    return 0;
}

// apply distributive law to make `Or` root of the expression.
static void distribute_And_over_Or(ast::Root& tree, ast::Expr* node_And)
{
    plnnrc_assert(node_And && is_And(node_And->type));

    // find the first `Or` argument of `node_And`.
    ast::Expr* node_Or = find_child(node_And, Token_Or);
    plnnrc_assert(node_Or);

    ast::Expr* after = node_And;
    for (ast::Expr* or_arg = node_Or->child; or_arg != 0; )
    {
        ast::Expr* next_or_arg = or_arg->next_sibling;
        ast::Expr* new_And = create_expr(tree, Token_And);

        for (ast::Expr* and_arg = node_And->child; and_arg != 0; )
        {
            ast::Expr* next_and_arg = and_arg->next_sibling;

            if (and_arg != node_Or)
            {
                ast::Expr* and_arg_clone = clone_tree(tree, and_arg);
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

static void debug_output_expr(ast::Expr* root, Formatter& fmtr)
{
    plnnrc::write(fmtr, "%i%s", plnnrc::get_type_name(root->type));

    if (root->value.str)
    {
        plnnrc::write(fmtr, "[%n]", root->value);
    }

    plnnrc::newline(fmtr);

    for (ast::Expr* child = root->child; child != 0; child = child->next_sibling)
    {
        Indent_Scope s(fmtr);
        debug_output_expr(child, fmtr);
    }
}

void plnnrc::debug_output_ast(const ast::Root& tree, Writer* output)
{
    Formatter fmtr;
    plnnrc::init(fmtr, "  ", "\n", output);

    if (tree.world)
    {
        plnnrc::newline(fmtr);
        plnnrc::writeln(fmtr, "World");

        Indent_Scope s(fmtr);
        for (ast::Fact_Type* fact = tree.world->facts; fact != 0; fact = fact->next)
        {
            plnnrc::write(fmtr, "%i%n[", fact->name);
            for (ast::Fact_Param* param = fact->params; param != 0; param = param->next)
            {
                plnnrc::write(fmtr, "%s", plnnrc::get_type_name(param->type));
                if (param->next)
                {
                    plnnrc::write(fmtr, ", ");
                }
            }
            plnnrc::write(fmtr, "]");
            plnnrc::newline(fmtr);
        }
    }

    if (tree.domain)
    {
        plnnrc::newline(fmtr);
        plnnrc::writeln(fmtr, "Domain[%n]", tree.domain->name);

        Indent_Scope s(fmtr);
        for (ast::Task* task = tree.domain->tasks; task != 0; task = task->next)
        {
            plnnrc::write(fmtr, "%iTask[%n]", task->name);
            Indent_Scope s(fmtr);

            if (task->params)
            {
                plnnrc::write(fmtr, " ");
            }

            for (ast::Task_Param* param = task->params; param != 0; param = param->next)
            {
                plnnrc::write(fmtr, "Param[%n] ", param->name);
            }

            plnnrc::newline(fmtr);

            for (ast::Case* case_ = task->cases; case_ != 0; case_ = case_->next)
            {
                plnnrc::writeln(fmtr, "Case");
                Indent_Scope s(fmtr);
                debug_output_expr(case_->precond, fmtr);

                for (ast::Expr* task_inst = case_->task_list; task_inst != 0; task_inst = task_inst->next_sibling)
                {
                    debug_output_expr(task_inst, fmtr);
                }
            }
        }
    }
}
