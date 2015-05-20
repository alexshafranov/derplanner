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
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/ast.h"

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

void plnnrc::init(ast::Root& root, Memory_Stack* mem_pool, Memory_Stack* mem_scratch)
{
    memset(&root, 0, sizeof(root));
    root.pool = mem_pool;
    root.scratch = mem_scratch;
}

template <typename T>
static T* pool_alloc(plnnrc::ast::Root& tree)
{
    T* result = allocate<T>(tree.pool);
    memset(result, 0, sizeof(T));
    return result;
}

template <typename T>
static T* pool_alloc(plnnrc::ast::Root* tree)
{
    T* result = allocate<T>(tree->pool);
    memset(result, 0, sizeof(T));
    return result;
}

ast::World* plnnrc::create_world(ast::Root* tree)
{
    ast::World* node = pool_alloc<ast::World>(tree);
    node->type = ast::Node_World;
    return node;
}

ast::Primitive* plnnrc::create_primitive(ast::Root* tree)
{
    ast::Primitive* node = pool_alloc<ast::Primitive>(tree);
    node->type = ast::Node_Primitive;
    return node;
}

ast::Predicate* plnnrc::create_predicate(ast::Root* tree, const Token_Value& name)
{
    ast::Predicate* node = pool_alloc<ast::Predicate>(tree);
    node->type = ast::Node_Predicate;
    node->name = name;
    return node;
}

ast::Fact* plnnrc::create_fact(ast::Root* tree, const Token_Value& name)
{
    ast::Fact* node = pool_alloc<ast::Fact>(tree);
    node->type = ast::Node_Fact;
    node->name = name;
    return node;
}

ast::Param* plnnrc::create_param(ast::Root* tree, const Token_Value& name)
{
    ast::Param* node = pool_alloc<ast::Param>(tree);
    node->type = ast::Node_Param;
    node->name = name;
    return node;
}

ast::Domain* plnnrc::create_domain(ast::Root* tree, const Token_Value& name)
{
    ast::Domain* node = pool_alloc<ast::Domain>(tree);
    node->type = ast::Node_Domain;
    node->name = name;
    return node;
}

ast::Task* plnnrc::create_task(ast::Root* tree, const Token_Value& name)
{
    ast::Task* node = pool_alloc<ast::Task>(tree);
    node->type = ast::Node_Task;
    node->name = name;
    return node;
}

ast::Case* plnnrc::create_case(ast::Root* tree)
{
    ast::Case* node = pool_alloc<ast::Case>(tree);
    node->type = ast::Node_Case;
    return node;
}

ast::Func* plnnrc::create_func(ast::Root* tree, const Token_Value& name)
{
    ast::Func* node = pool_alloc<ast::Func>(tree);
    node->type = ast::Node_Func;
    node->name = name;
    return node;
}

ast::Op* plnnrc::create_op(ast::Root* tree, ast::Node_Type operation)
{
    ast::Op* node = pool_alloc<ast::Op>(tree);
    node->type = operation;
    return node;
}

ast::Var* plnnrc::create_var(ast::Root* tree, const Token_Value& name)
{
    ast::Var* node = pool_alloc<ast::Var>(tree);
    node->type = ast::Node_Var;
    node->name = name;
    return node;
}

ast::Data_Type* plnnrc::create_type(ast::Root* tree, Token_Type data_type)
{
    ast::Data_Type* node = pool_alloc<ast::Data_Type>(tree);
    node->type = ast::Node_Data_Type;
    node->data_type = data_type;
    return node;
}

ast::Literal* plnnrc::create_literal(ast::Root* tree, const Token& token)
{
    ast::Literal* node = pool_alloc<ast::Literal>(tree);
    node->type = ast::Node_Literal;
    node->value = token.value;
    node->data_type = token.type;
    return node;
}

ast::Fact* plnnrc::get_fact(ast::Root& tree, const Token_Value& name)
{
    return get(tree.fact_lookup, name);
}

ast::Task* plnnrc::get_task(ast::Root& tree, const Token_Value& name)
{
    return get(tree.task_lookup, name);
}

ast::Fact* plnnrc::get_primitive(ast::Root& tree, const Token_Value& name)
{
    return get(tree.primitive_lookup, name);
}

void plnnrc::flatten(ast::Expr* root)
{
    plnnrc_assert(root);

    for (ast::Expr* node_U = root; node_U != 0; node_U = preorder_next(root, node_U))
    {
        if (!is_And(node_U) && !is_Or(node_U))
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
                        unparent(child);
                        insert_child(after, child);
                        after = child;
                        child = next_child;
                    }

                    // now when it's child-less remove `node_V` from the tree.
                    unparent(node_V);
                    done = false;
                }

                node_V = sibling;
            }

            // no nodes were collapsed -> exit.
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
        if (!is_Not(node_Not))
        {
            node_Not = preorder_next(new_root, node_Not);
            continue;
        }

        ast::Expr* node_Logical = node_Not->child;
        plnnrc_assert(node_Logical != 0);

        // eliminate double negation: Not { Not { <expr...> } } -> <expr...>
        if (is_Not(node_Logical))
        {
            ast::Expr* expr = node_Logical->child;
            plnnrc_assert(expr != 0);

            // get rid of `node_Not` and `node_Logical`, replacing `node_Not` with `expr`.
            unparent(expr);

            if (node_Not->parent)
            {
                insert_child(node_Not, expr);
                unparent(node_Not);
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
        if (is_And(node_Logical) || is_Or(node_Logical))
        {
            // node_Not becomes `And` or `Or`.
            ast::Expr* node_Op = node_Not;
            node_Op->type = is_And(node_Logical) ? ast::Node_Or : ast::Node_And;

            // all chilren of `node_Logical` are parented under a new `Not` which is parented under `node_Op`.
            ast::Expr* after = node_Logical;
            for (ast::Expr* expr = node_Logical->child; expr != 0; )
            {
                ast::Expr* next_expr = expr->next_sibling;
                unparent(expr);

                ast::Expr* new_Not = create_op(&tree, ast::Node_Not);
                append_child(new_Not, expr);
                insert_child(after, new_Not);
                after = new_Not;

                expr = next_expr;
            }

            // `node_Logical` now has no children and can be safely unparented.
            unparent(node_Logical);
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
    ast::Expr* new_root = create_op(&tree, ast::Node_Or);

    // empty expression.
    if (is_And(root) && !root->child)
    {
        return new_root;
    }

    // convert `root` to Negative-Normal-Form and put it under a new Or node.
    ast::Expr* nnf_root = convert_to_nnf(tree, root);
    append_child(new_root, nnf_root);
    flatten(new_root);

    // now we have a flattened Or expression
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
static bool is_trivial_conjunct(ast::Expr* node)
{
    // assert expression is NNF.
    plnnrc_assert(!is_Not(node) || !is_Logical(node->child));
    return is_Not(node) || is_Var(node) || is_Literal(node) || is_Func(node);
}

// check if expression is either trivial (~x, x) or conjunction of trivials.
static bool is_conjunct(ast::Expr* node)
{
    if (is_trivial_conjunct(node))
    {
        return true;
    }

    // check if expression is conjunction of trivials.
    if (!is_And(node))
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

static ast::Expr* find_child(ast::Expr* root, ast::Node_Type type)
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

struct Cloner
{
    ast::Root* tree;

    template <typename T>
    ast::Expr* make_clone(const T* node)
    {
        T* clone = pool_alloc<T>(*tree);
        memcpy(clone, node, sizeof(T));
        clone->parent = 0;
        clone->child = 0;
        clone->next_sibling = 0;
        clone->prev_sibling_cyclic = 0;
        return clone;
    }

    ast::Expr* visit(const ast::Func*   node)    { return make_clone(node); }
    ast::Expr* visit(const ast::Var*    node)    { return make_clone(node); }
    ast::Expr* visit(const ast::Op*     node)    { return make_clone(node); }
    ast::Expr* visit(const ast::Node*)           { plnnrc_assert(false); return 0; }
};

static ast::Expr* clone_node(ast::Root& tree, const ast::Expr* node)
{
    Cloner cloner = { &tree };
    return visit_node<ast::Expr*>(node, &cloner);
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
            append_child(clone, child_clone);
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
        insert_child(clone, sibling_clone);

        node = sibling;
        clone = sibling_clone;
    }

    return root_clone;
}

// apply distributive law to make `Or` root of the expression.
static void distribute_And_over_Or(ast::Root& tree, ast::Expr* node_And)
{
    plnnrc_assert(node_And && is_And(node_And));

    // find the first `Or` argument of `node_And`.
    ast::Expr* node_Or = find_child(node_And, ast::Node_Or);
    plnnrc_assert(node_Or);

    ast::Expr* after = node_And;
    for (ast::Expr* or_arg = node_Or->child; or_arg != 0; )
    {
        ast::Expr* next_or_arg = or_arg->next_sibling;
        ast::Expr* new_And = create_op(&tree, ast::Node_And);

        for (ast::Expr* and_arg = node_And->child; and_arg != 0; )
        {
            ast::Expr* next_and_arg = and_arg->next_sibling;

            if (and_arg != node_Or)
            {
                ast::Expr* and_arg_clone = clone_tree(tree, and_arg);
                append_child(new_And, and_arg_clone);
            }
            else
            {
                unparent(or_arg);
                append_child(new_And, or_arg);
            }

            and_arg = next_and_arg;
        }

        flatten(new_And);
        insert_child(after, new_And);
        after = new_And;
        or_arg = next_or_arg;
    }

    unparent(node_And);
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

void plnnrc::convert_to_dnf(ast::Root& tree)
{
    ast::Domain* domain = tree.domain;

    for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
    {
        ast::Task* task = domain->tasks[task_idx];
        for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
        {
            ast::Case* case_ = task->cases[case_idx];
            ast::Expr* precond = case_->precond;
            case_->precond = convert_to_dnf(tree, precond);
        }
    }
}

static ast::Expr* inline_predicate(ast::Root& tree, ast::Func* func, ast::Predicate* pred)
{
    // clone and replace variables with the arguments from `func`.
    ast::Expr* pred_clone = clone_tree(tree, pred->expression);

    for (ast::Expr* node = pred_clone; node != 0; )
    {
        ast::Expr* next_node = preorder_next(pred_clone, node);
        ast::Var* var = as_Var(node);
        node = next_node;

        if (var)
        {
            uint32_t var_hash = hash(var->name);

            uint32_t arg_index = 0;
            for (ast::Expr* arg = func->child; arg != 0; arg = arg->next_sibling, ++arg_index)
            {
                plnnrc_assert(arg_index < size(pred->params));
                ast::Param* param = pred->params[arg_index];
                uint32_t param_hash = hash(param->name);

                if (var_hash == param_hash && equal(var->name, param->name))
                {
                    ast::Expr* arg_clone = clone_tree(tree, arg);
                    insert_child(var, arg_clone);
                    unparent(var);
                }
            }
        }
    }

    // replace `func` with `pred_clone`.
    insert_child(func, pred_clone);
    unparent(func);

    return pred_clone;
}

static ast::Predicate* lookup_referenced_predicate(ast::Domain* domain, ast::Task* task, const Token_Value& name)
{
    if (task != 0)
    {
        if (ast::Predicate* ref_pred = get(task->predicate_lookup, name))
        {
            return ref_pred;
        }
    }

    if (ast::Predicate* ref_pred = get(domain->predicate_lookup, name))
    {
        return ref_pred;
    }

    return 0;
}

static void disallow_recursive_predicate(ast::Root& tree, ast::Task* task_scope, ast::Predicate* pred)
{
    Memory_Stack_Scope scratch_scope(tree.scratch);
    Memory_Stack_Scope pool_scope(tree.pool);

    ast::Domain* domain_scope = tree.domain;

    Id_Table<ast::Predicate*> preds_seen;
    init(preds_seen, tree.scratch, 8);
    set(preds_seen, pred->name, pred);

    ast::Expr* root = create_op(&tree, ast::Node_Or);
    append_child(root, pred->expression);

    for (ast::Expr* node = root->child; node != 0; node = preorder_next(root, node))
    {
        if (ast::Func* func = as_Func(node))
        {
            if (ast::Predicate* ref_pred = lookup_referenced_predicate(domain_scope, task_scope, func->name))
            {
                if (get(preds_seen, func->name) != 0)
                {
                    plnnrc_assert(false);
                    break;
                }

                set(preds_seen, func->name, ref_pred);
                insert_child(node, ref_pred->expression);
            }
        }
    }

    for (ast::Expr* node = root->child; node != 0; )
    {
        ast::Expr* next = node->next_sibling;
        unparent(node);
        node = next;
    }
}

void plnnrc::inline_predicates(ast::Root& tree)
{
    ast::Domain* domain = tree.domain;

    // build lookups.
    init(domain->predicate_lookup, tree.pool, size(domain->predicates));
    for (uint32_t pred_idx = 0; pred_idx < size(domain->predicates); ++pred_idx)
    {
        ast::Predicate* pred = domain->predicates[pred_idx];
        set(domain->predicate_lookup, pred->name, pred);
    }

    for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
    {
        ast::Task* task = domain->tasks[task_idx];
        init(task->predicate_lookup, tree.pool, size(task->predicates));

        for (uint32_t pred_idx = 0; pred_idx < size(task->predicates); ++pred_idx)
        {
            ast::Predicate* pred = task->predicates[pred_idx];
            set(task->predicate_lookup, pred->name, pred);
        }
    }

    // check recursion.
    for (uint32_t pred_idx = 0; pred_idx < size(domain->predicates); ++pred_idx)
    {
        ast::Predicate* pred = domain->predicates[pred_idx];
        disallow_recursive_predicate(tree, 0, pred);
    }

    for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
    {
        ast::Task* task = domain->tasks[task_idx];
        for (uint32_t pred_idx = 0; pred_idx < size(task->predicates); ++pred_idx)
        {
            ast::Predicate* pred = task->predicates[pred_idx];
            disallow_recursive_predicate(tree, task, pred);
        }
    }

    // inline predicates in each precondition.
    for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
    {
        ast::Task* task = domain->tasks[task_idx];
        for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
        {
            ast::Case* case_ = task->cases[case_idx];
            ast::Expr* precond = case_->precond;
            // add the dummy root node to simplify `inline_predicates`, as it may need to replace the precondition root.
            ast::Expr* new_root = create_op(&tree, ast::Node_And);
            append_child(new_root, precond);
            case_->precond = new_root;

            for (ast::Expr* node = new_root->child; node != 0; )
            {
                if (ast::Func* func = as_Func(node))
                {
                    // look up the predicate in the `task` scope first.
                    if (ast::Predicate* pred = get(task->predicate_lookup, func->name))
                    {
                        node = inline_predicate(tree, func, pred);
                        continue;
                    }

                    // look up in the global, `domain` scope next.
                    if (ast::Predicate* pred = get(domain->predicate_lookup, func->name))
                    {
                        node = inline_predicate(tree, func, pred);
                        continue;
                    }
                }

                node = preorder_next(new_root, node);
            }
        }
    }
}

static void build_var_lookup(ast::Expr* expr, Array<ast::Var*>& out_vars, Id_Table<ast::Var*>& out_lookup)
{
    for (ast::Expr* node = expr; node != 0; node = preorder_next(expr, node))
    {
        if (ast::Var* var = as_Var(node))
        {
            push_back(out_vars, var);

            ast::Var* def = get(out_lookup, var->name);
            if (def != 0)
            {
                plnnrc_assert(!var->definition);
                var->definition = def;
                continue;
            }

            set(out_lookup, var->name, var);
        }
    }
}

void plnnrc::annotate(ast::Root& tree)
{
    ast::World* world = tree.world;
    if (world)
    {
        const uint32_t num_facts = size(world->facts);
        init(tree.fact_lookup, tree.pool, num_facts);

        for (uint32_t i = 0; i < num_facts; ++i)
        {
            ast::Fact* fact = world->facts[i];
            set(tree.fact_lookup, fact->name, fact);
        }
    }

    ast::Primitive* prim = tree.primitive;
    if (prim)
    {
        const uint32_t num_tasks = size(prim->tasks);
        init(tree.primitive_lookup, tree.pool, num_tasks);

        for (uint32_t i = 0; i < num_tasks; ++i)
        {
            ast::Fact* task = prim->tasks[i];
            set(tree.primitive_lookup, task->name, task);
        }
    }

    ast::Domain* domain = tree.domain;
    if (domain)
    {
        const uint32_t num_tasks = size(domain->tasks);
        init(tree.task_lookup, tree.pool, num_tasks);

        uint32_t num_cases = 0;
        for (uint32_t i = 0; i < num_tasks; ++i)
        {
            ast::Task* task = domain->tasks[i];
            num_cases += size(task->cases);
            set(tree.task_lookup, task->name, task);
        }

        // collect all `ast::Case` nodes.
        if (num_cases > 0)
        {
            init(tree.cases, tree.pool, num_cases);
            for (uint32_t i = 0; i < num_tasks; ++i)
            {
                ast::Task* task = domain->tasks[i];
                for (uint32_t j = 0; j < size(task->cases); ++j)
                {
                    ast::Case* case_ = task->cases[j];
                    push_back(tree.cases, case_);
                }
            }
        }

        // build param_lookup for each task.
        for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
        {
            ast::Task* task = domain->tasks[task_idx];
            init(task->param_lookup, tree.pool, 8);

            for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
            {
                ast::Param* param = task->params[param_idx];
                set(task->param_lookup, param->name, param);
            }
        }

        // build ast::Var lookups for each case.
        for (uint32_t case_idx = 0; case_idx < size(tree.cases); ++case_idx)
        {
            ast::Case* case_ = tree.cases[case_idx];
            ast::Task* task = case_->task;

            init(case_->precond_var_lookup, tree.pool, 8);
            init(case_->precond_vars, tree.pool, 8);
            init(case_->task_list_var_lookup, tree.pool, 8);
            init(case_->task_list_vars, tree.pool, 8);

            build_var_lookup(case_->precond, case_->precond_vars, case_->precond_var_lookup);

            for (uint32_t task_idx = 0; task_idx < size(case_->task_list); ++task_idx)
            {
                ast::Expr* task_expr = case_->task_list[task_idx];
                build_var_lookup(task_expr, case_->task_list_vars, case_->task_list_var_lookup);
            }

            // set `definition` for vars referencing task parameters.
            for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
            {
                ast::Param* param = task->params[param_idx];
                if (ast::Var* var = get(case_->precond_var_lookup, param->name))
                {
                    plnnrc_assert(!var->definition);
                    var->definition = param;
                    plnnrc_assert(param->data_type == Token_Unknown || param->data_type == var->data_type);
                }
            }

            for (ast::Expr* node = case_->precond; node != 0; node = preorder_next(case_->precond, node))
            {
                if (ast::Var* var = as_Var(node))
                {
                    if (ast::Var* def = as_Var(var->definition))
                    {
                        if (ast::Param* param = as_Param(def->definition))
                        {
                            var->definition = param;
                        }
                    }
                }
            }

            // set task list var definitions.
            for (uint32_t var_idx = 0; var_idx < size(case_->task_list_vars); ++var_idx)
            {
                ast::Var* var = case_->task_list_vars[var_idx];

                if (ast::Param* def = get(task->param_lookup, var->name))
                {
                    var->definition = def;
                    continue;
                }

                if (ast::Var* def = get(case_->precond_var_lookup, var->name))
                {
                    var->definition = def;
                    continue;
                }

                // unbound var used in task list.
                plnnrc_assert(false);
            }

            // build `precond_facts`
            init(case_->precond_facts, tree.pool, 8);
            for (ast::Expr* node = case_->precond; node != 0; node = preorder_next(case_->precond, node))
            {
                if (ast::Func* func = as_Func(node))
                {
                    ast::Fact* fact = get(tree.fact_lookup, func->name);
                    push_back(case_->precond_facts, fact);
                }
            }
        }
    }
}

void plnnrc::infer_types(ast::Root& tree)
{
    if (!tree.domain || !tree.world) { return; }

    // seed types in preconditions using fact declarations.
    for (uint32_t case_idx = 0; case_idx < size(tree.cases); ++case_idx)
    {
        ast::Case* case_ = tree.cases[case_idx];
        for (ast::Expr* node = case_->precond; node != 0; node = preorder_next(case_->precond, node))
        {
            if (ast::Func* func = as_Func(node))
            {
                ast::Fact* fact = get_fact(tree, func->name);
                plnnrc_assert(fact);
                // currently assuming all children are vars.
                ast::Var* var = as_Var(func->child);
                for (uint32_t param_idx = 0; param_idx < size(fact->params); ++param_idx)
                {
                    plnnrc_assert(var);
                    ast::Data_Type* param_type = fact->params[param_idx];
                    var->data_type = param_type->data_type;
                    var = as_Var(var->next_sibling);
                }

                plnnrc_assert(!var);
            }
        }
    }

    // set task parameter types based on their usage in preconditions.
    for (uint32_t case_idx = 0; case_idx < size(tree.cases); ++case_idx)
    {
        ast::Case* case_ = tree.cases[case_idx];
        ast::Task* task = case_->task;

        for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
        {
            ast::Param* param = task->params[param_idx];
            if (ast::Var* var = get(case_->precond_var_lookup, param->name))
            {
                param->data_type = var->data_type;
            }
        }
    }

    // propagate types "down" to task lists.
    for (uint32_t case_idx = 0; case_idx < size(tree.cases); ++case_idx)
    {
        ast::Case* case_ = tree.cases[case_idx];
        for (uint32_t task_idx = 0; task_idx < size(case_->task_list); ++task_idx)
        {
            ast::Func* func = as_Func(case_->task_list[task_idx]);
            plnnrc_assert(func);
            ast::Task* task = get_task(tree, func->name);

            uint32_t param_idx = 0;
            for (ast::Expr* node = func->child; node != 0; node = node->next_sibling)
            {
                ast::Var* var = as_Var(node);
                plnnrc_assert(var && var->definition);
                ast::Node* def = var->definition;
                Token_Type data_type = as_Param(def) ? as_Param(def)->data_type : as_Var(def)->data_type;
                var->data_type = data_type;

                if (task)
                {
                    ast::Param* param = task->params[param_idx];
                    param->data_type = data_type;
                }

                ++param_idx;
            }
        }
    }
}

static const char* node_type_names[] =
{
    "None",
    #define PLNNRC_NODE(TAG, TYPE) #TAG,
    #include "derplanner/compiler/ast_tags.inl"
    #undef PLNNRC_NODE
    "Count",
};

const char* plnnrc::get_type_name(ast::Node_Type node_type)
{
    return node_type_names[node_type];
}

struct Debug_Output_Visitor
{
    Formatter* fmtr;

    template <typename T>
    void print_children(const plnnrc::Array<T*>& nodes)
    {
        for (uint32_t i = 0; i < size(nodes); ++i)
        {
            Indent_Scope s(*fmtr);
            visit_node<void>(nodes[i], this);
        }
    }

    void print_children(const ast::Expr* node)
    {
        for (const ast::Expr* child = node->child; child != 0; child = child->next_sibling)
        {
            Indent_Scope s(*fmtr);
            visit_node<void>(child, this);
        }
    }

    void print_expr(const ast::Expr* node)
    {
        Indent_Scope s(*fmtr);
        visit_node<void>(node, this);
    }

    void print(const ast::Node* node) { writeln(*fmtr, "%s", get_type_name(node->type)); }

    template <typename T>
    void print_named(const T* node) { writeln(*fmtr, "%s[%n]", get_type_name(node->type), node->name); }

    template <typename T>
    void print_data_type(const T* node)
    {
        Indent_Scope s(*fmtr);
        writeln(*fmtr, ":%s", get_type_name(node->data_type));
    }

    void visit(const ast::World* node) { print(node); print_children(node->facts); }
    void visit(const ast::Primitive* node) { print(node); print_children(node->tasks); }
    void visit(const ast::Domain* node) { print_named(node); print_children(node->predicates); print_children(node->tasks); }
    void visit(const ast::Fact* node) { print_named(node); print_children(node->params); }
    void visit(const ast::Predicate* node) { print_named(node); print_children(node->params); print_expr(node->expression); }
    void visit(const ast::Task* node) { print_named(node); print_children(node->params); print_children(node->predicates); print_children(node->cases); }
    void visit(const ast::Case* node) { print(node); print_expr(node->precond); print_children(node->task_list); }
    void visit(const ast::Param* node) { print_named(node); print_data_type(node); }
    void visit(const ast::Var* node) { print_named(node); print_data_type(node); }
    void visit(const ast::Func* node) { print_named(node); print_children(node); }
    void visit(const ast::Expr* node) { print(node); print_children(node); }
    void visit(const ast::Data_Type* node) { print(node); print_data_type(node); }
    void visit(const ast::Literal* node) { print(node); }
};

void plnnrc::debug_output_ast(const ast::Root& tree, Writer* output)
{
    Formatter fmtr;
    init(fmtr, "  ", "\n", output);
    newline(fmtr);

    Debug_Output_Visitor visitor = { &fmtr };

    if (tree.world)
    {
        visit_node<void>(tree.world, &visitor);
    }

    if (tree.primitive)
    {
        visit_node<void>(tree.primitive, &visitor);
    }

    if (tree.domain)
    {
        visit_node<void>(tree.domain, &visitor);
    }

    newline(fmtr);
}
