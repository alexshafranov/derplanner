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

#include <limits>
#include <stdlib.h>

#include "derplanner/compiler/io.h"
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/string_buffer.h"
#include "derplanner/compiler/function_table.h"
#include "derplanner/compiler/ast.h"

using namespace plnnrc;

// implementation (exposed for unit tests)
namespace plnnrc
{
    // minimize expression tree depth by collapsing redundant operaion nodes.
    // e.g.: Op { <head...> Op{ <children...> }  <rest...> } -> Op { <head...> <children...> <rest...> }
    void        flatten(ast::Expr* root);
    // converts expression `root` to Negation-Normal-Form.
    ast::Expr*  convert_to_nnf(const ast::Root* tree, ast::Expr* root);
}

void plnnrc::init(ast::Root* self, Array<Error>* errors, Memory_Stack* mem_pool, Memory_Stack* mem_scratch)
{
    memset(self, 0, sizeof(ast::Root));
    self->errs = errors;
    self->pool = mem_pool;
    self->scratch = mem_scratch;

    init(self->symbols, self->pool, 16);
    init(self->names, self->pool, 64, 512);

    // add instrinsics to the function table.
    init(self->functions, self->pool, 16);
    add_function(self->functions, "empty", Token_Int8, Token_Fact_Ref);

    add_function(self->functions, "abs", Token_Float, Token_Float);
    add_function(self->functions, "cos", Token_Float, Token_Float);
    add_function(self->functions, "sin", Token_Float, Token_Float);
    add_function(self->functions, "rad", Token_Float, Token_Float);
    add_function(self->functions, "deg", Token_Float, Token_Float);
    add_function(self->functions, "pi", Token_Float);
    add_function(self->functions, "clamp", Token_Float, Token_Float, Token_Float, Token_Float);

    add_function(self->functions, "vec3", Token_Vec3, Token_Float, Token_Float, Token_Float);
    add_function(self->functions, "x", Token_Float, Token_Vec3);
    add_function(self->functions, "y", Token_Float, Token_Vec3);
    add_function(self->functions, "z", Token_Float, Token_Vec3);
    add_function(self->functions, "dot", Token_Float, Token_Vec3, Token_Vec3);
    add_function(self->functions, "cross", Token_Vec3, Token_Vec3, Token_Vec3);
    add_function(self->functions, "len", Token_Float, Token_Vec3);
    add_function(self->functions, "dist", Token_Float, Token_Vec3, Token_Vec3);
    add_function(self->functions, "norm", Token_Vec3, Token_Vec3);
}

template <typename T>
static T* pool_alloc(const plnnrc::ast::Root* tree)
{
    T* result = allocate<T>(tree->pool);
    memset(result, 0, sizeof(T));
    return result;
}

static Error& emit(const ast::Root* tree, const Location& loc, Error_Type error_type)
{
    Error err;
    init(err, error_type, loc);
    push_back(*(tree->errs), err);
    return back(*(tree->errs));
}

static Token_Value make_unique_name(ast::Root* tree)
{
    const uint32_t next_idx = size(tree->names);
    begin_string(tree->names);
    write(tree->names.buffer, "$%d", next_idx);
    end_string(tree->names);
    return get(tree->names, next_idx);
}

ast::World* plnnrc::create_world(const ast::Root* tree)
{
    ast::World* node = pool_alloc<ast::World>(tree);
    node->type = ast::Node_World;
    return node;
}

ast::Primitive* plnnrc::create_primitive(const ast::Root* tree)
{
    ast::Primitive* node = pool_alloc<ast::Primitive>(tree);
    node->type = ast::Node_Primitive;
    return node;
}

ast::Attribute* plnnrc::create_attribute(const ast::Root* tree, const Token_Value& name, const Location& loc)
{
    ast::Attribute* node = pool_alloc<ast::Attribute>(tree);
    node->type = ast::Node_Attribute;
    node->name = name;
    node->loc = loc;
    return node;
}

ast::Macro* plnnrc::create_macro(const ast::Root* tree, const Token_Value& name, const Location& loc)
{
    ast::Macro* node = pool_alloc<ast::Macro>(tree);
    node->type = ast::Node_Macro;
    node->name = name;
    node->loc = loc;
    return node;
}

ast::Fact* plnnrc::create_fact(const ast::Root* tree, const Token_Value& name, const Location& loc)
{
    ast::Fact* node = pool_alloc<ast::Fact>(tree);
    node->type = ast::Node_Fact;
    node->name = name;
    node->loc = loc;
    return node;
}

ast::Param* plnnrc::create_param(const ast::Root* tree, const Token_Value& name, const Location& loc)
{
    ast::Param* node = pool_alloc<ast::Param>(tree);
    node->type = ast::Node_Param;
    node->name = name;
    node->loc = loc;
    return node;
}

ast::Domain* plnnrc::create_domain(const ast::Root* tree, const Token_Value& name)
{
    ast::Domain* node = pool_alloc<ast::Domain>(tree);
    node->type = ast::Node_Domain;
    node->name = name;
    return node;
}

ast::Task* plnnrc::create_task(const ast::Root* tree, const Token_Value& name, const Location& loc)
{
    ast::Task* node = pool_alloc<ast::Task>(tree);
    node->type = ast::Node_Task;
    node->name = name;
    node->loc = loc;
    return node;
}

ast::Case* plnnrc::create_case(const ast::Root* tree)
{
    ast::Case* node = pool_alloc<ast::Case>(tree);
    node->type = ast::Node_Case;
    return node;
}

ast::Data_Type* plnnrc::create_type(const ast::Root* tree, Token_Type data_type)
{
    ast::Data_Type* node = pool_alloc<ast::Data_Type>(tree);
    node->type = ast::Node_Data_Type;
    node->data_type = data_type;
    return node;
}

ast::Func* plnnrc::create_func(const ast::Root* tree, const Token_Value& name, const Location& loc)
{
    ast::Func* node = pool_alloc<ast::Func>(tree);
    node->type = ast::Node_Func;
    node->name = name;
    node->loc = loc;
    return node;
}

ast::Op* plnnrc::create_op(const ast::Root* tree, ast::Node_Type operation)
{
    ast::Op* node = pool_alloc<ast::Op>(tree);
    node->type = operation;
    return node;
}

ast::Op* plnnrc::create_op(const ast::Root* tree, ast::Node_Type operation, const Location& loc)
{
    ast::Op* node = pool_alloc<ast::Op>(tree);
    node->type = operation;
    node->loc = loc;
    return node;
}

ast::Var* plnnrc::create_var(const ast::Root* tree, const Token_Value& name, const Location& loc)
{
    ast::Var* node = pool_alloc<ast::Var>(tree);
    node->type = ast::Node_Var;
    node->name = name;
    node->original_name = name;
    node->loc = loc;
    return node;
}

ast::Literal* plnnrc::create_literal(const ast::Root* tree, const Token& token, const Location& loc)
{
    ast::Literal* node = pool_alloc<ast::Literal>(tree);
    node->type = ast::Node_Literal;
    node->value = token.value;
    node->value_type = token.type;
    node->loc = loc;
    return node;
}

ast::Fact* plnnrc::get_fact(const ast::Root* self, const Token_Value& name)
{
    return get(self->fact_lookup, name);
}

ast::Task* plnnrc::get_task(const ast::Root* self, const Token_Value& name)
{
    return get(self->task_lookup, name);
}

ast::Fact* plnnrc::get_primitive(const ast::Root* self, const Token_Value& name)
{
    return get(self->primitive_lookup, name);
}

int64_t plnnrc::as_int(const ast::Literal* node)
{
    #ifdef PLNNRC_MSVC_VERSION
        return int64_t(_strtoi64(node->value.str, 0, 10));
    #else
        return int64_t(strtoll(node->value.str, 0, 10));
    #endif
}

float plnnrc::as_float(const ast::Literal* node)
{
    return float(strtod(node->value.str, 0));
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

ast::Expr* plnnrc::convert_to_nnf(const ast::Root* tree, ast::Expr* root)
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

        // Not{ Or{ <x...> <y...> } }  -> And{ Not{ <x...> } Not{ <y...> } }
        // Not{ And{ <x...> <y...> } } ->  Or{ Not{ <x...> } Not{ <y...> } }
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

                ast::Expr* new_Not = create_op(tree, ast::Node_Not);
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

static bool         is_conjunct(const ast::Expr* node);
static void         distribute_and_over_or(const ast::Root* tree, ast::Expr* node_And);
static ast::Expr*   convert_or_to_dnf(const ast::Root* tree, ast::Expr* node_Or);

ast::Expr* plnnrc::convert_to_dnf(const ast::Root* tree, ast::Expr* root)
{
    ast::Expr* new_root = create_op(tree, ast::Node_Or);

    // empty expression.
    if (is_And(root) && !root->child)
    {
        return new_root;
    }

    // convert `root` to Negative-Normal-Form and put it under a new Or node.
    ast::Expr* nnf_root = convert_to_nnf(tree, root);
    append_child(new_root, nnf_root);
    // flatten if nnf_root happens to be Or node.
    flatten(new_root);

    // convert to DNF form: Expr = C0 | C1 | ... | CN, Ck (conjunct) = either ~X or X where X is a term.
    new_root = convert_or_to_dnf(tree, new_root);
    return new_root;
}

// convert Or expression to DNF by applying distributive law repeatedly, until no change could be made.
static ast::Expr* convert_or_to_dnf(const ast::Root* tree, ast::Expr* node_Or)
{
    plnnrc_assert(node_Or && is_Or(node_Or));

    for (;;)
    {
        bool done = true;

        for (ast::Expr* arg = node_Or->child; arg != 0; )
        {
            ast::Expr* next_arg = arg->next_sibling;

            if (!is_conjunct(arg))
            {
                done = false;
                distribute_and_over_or(tree, arg);
            }

            arg = next_arg;
        }

        // all arguments of node_Or are conjuncts (literal or conjunction of literals) -> exit.
        if (done)
        {
            break;
        }
    }

    return node_Or;
}

// check if trivial conjunct `~x` or `x`. expression is assumed to be in Negative-Normal-Form.
static bool is_trivial_conjunct(const ast::Expr* node)
{
    // assert expression is NNF.
    plnnrc_assert(!is_Not(node) || !is_Logical(node->child));
    return is_Not(node) || !is_Logical(node);
}

// check if expression is either trivial (~x, x) or conjunction of trivials.
static bool is_conjunct(const ast::Expr* node)
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
    const ast::Root* tree;

    template <typename T>
    ast::Expr* make_clone(const T* node)
    {
        T* clone = pool_alloc<T>(tree);
        memcpy(clone, node, sizeof(T));
        clone->parent = 0;
        clone->child = 0;
        clone->next_sibling = 0;
        clone->prev_sibling_cyclic = 0;
        return clone;
    }

    ast::Expr* visit(const ast::Func*       node) { return make_clone(node); }

    ast::Expr* visit(const ast::Var*        node) { return make_clone(node); }

    ast::Expr* visit(const ast::Op*         node) { return make_clone(node); }

    ast::Expr* visit(const ast::Literal*    node) { return make_clone(node); }

    ast::Expr* visit(const ast::Node*) { plnnrc_assert(false); return 0; }
};

static ast::Expr* clone_node(const ast::Root* tree, const ast::Expr* node)
{
    Cloner cloner = { tree };
    return visit_node<ast::Expr*>(node, &cloner);
}

static ast::Expr* clone_tree(const ast::Root* tree, ast::Expr* root)
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
static void distribute_and_over_or(const ast::Root* tree, ast::Expr* node_And)
{
    plnnrc_assert(node_And && is_And(node_And));

    // find the first `Or` argument of `node_And`.
    ast::Expr* node_Or = find_child(node_And, ast::Node_Or);
    plnnrc_assert(node_Or);

    ast::Expr* after = node_And;
    for (ast::Expr* or_arg = node_Or->child; or_arg != 0; )
    {
        ast::Expr* next_or_arg = or_arg->next_sibling;
        ast::Expr* new_And = create_op(tree, ast::Node_And);

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

ast::Expr* plnnrc::preorder_next(const ast::Expr* root, const ast::Expr* current)
{
    const ast::Expr* node = current;

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

struct Find_Attribute
{
    Attribute_Type type;

    ast::Attribute* find(const Array<ast::Attribute*>& attrs)
    {
        for (uint32_t attr_idx = 0; attr_idx < size(attrs); ++attr_idx)
        {
            ast::Attribute* attr = attrs[attr_idx];
            if (attr->attr_type == type)
                return attr;
        }

        return 0;
    }

    ast::Attribute* visit(const ast::Fact* node) { return find(node->attrs); }

    ast::Attribute* visit(const ast::Case* node) { return find(node->attrs); }

    ast::Attribute* visit(const ast::Node*) { return 0; }
};

ast::Attribute* plnnrc::find_attribute(const ast::Node* node, Attribute_Type type)
{
    Find_Attribute visitor = { type };
    return visit_node<ast::Attribute*>(node, &visitor);
}

void plnnrc::convert_to_dnf(const ast::Root* tree)
{
    ast::Domain* domain = tree->domain;

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

static ast::Macro* lookup_referenced_macro(const ast::Domain* domain, const ast::Task* task, const Token_Value& name);

static ast::Expr* inline_macro(ast::Root* tree, const ast::Task* task, const ast::Macro* macro, ast::Func* func)
{
    Memory_Stack_Scope scratch_scope(tree->scratch);

    Id_Table<bool> refs;
    init(refs, tree->scratch, size(macro->params));

    for (uint32_t param_idx = 0; param_idx < size(macro->params); ++param_idx)
    {
        const ast::Param* param = macro->params[param_idx];
        set(refs, param->name, true);
    }

    const bool defined_inside_task = (task != 0 && get(task->macro_lookup, macro->name));
    if (defined_inside_task)
    {
        for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
        {
            const ast::Param* param = task->params[param_idx];
            set(refs, param->name, true);
        }
    }

    // clone and replace variables with the arguments from `func`.
    ast::Expr* macro_clone = clone_tree(tree, macro->expression);

    for (ast::Expr* node = macro_clone; node != 0; node = preorder_next(macro_clone, node))
    {
        if (ast::Var* var = as_Var(node))
        {
            // constants
            const ast::Macro* ref_macro = lookup_referenced_macro(tree->domain, defined_inside_task ? task : 0, var->name);
            if (ref_macro)
            {
                set(refs, ref_macro->name, true);
            }
        }
    }

    // set unique names for the "local" variables used in the macro.
    Id_Table<Token_Value> unique_names;
    init(unique_names, tree->scratch, 8);

    for (ast::Expr* node = macro_clone; node != 0; node = preorder_next(macro_clone, node))
    {
        if (ast::Var* var = as_Var(node))
        {
            if (get(refs, var->name))
                continue;

            const Token_Value* unique_name = get(unique_names, var->name);

            if (!unique_name)
            {
                const Token_Value new_name = make_unique_name(tree);
                set(unique_names, var->name, new_name);
                var->name = new_name;
            }
            else
            {
                var->name = *unique_name;
            }
        }
    }

    for (ast::Expr* node = macro_clone; node != 0; )
    {
        ast::Expr* next_node = preorder_next(macro_clone, node);
        ast::Var* var = as_Var(node);
        node = next_node;

        if (var)
        {
            const uint32_t var_hash = hash(var->name);

            uint32_t arg_index = 0;
            for (ast::Expr* arg = func->child; arg != 0; arg = arg->next_sibling, ++arg_index)
            {
                plnnrc_assert(arg_index < size(macro->params));
                const ast::Param* param = macro->params[arg_index];
                const uint32_t param_hash = hash(param->name);

                if (var_hash == param_hash && equal(var->name, param->name))
                {
                    ast::Expr* arg_clone = clone_tree(tree, arg);
                    insert_child(var, arg_clone);
                    unparent(var);
                }
            }
        }
    }

    // replace `func` with `macro_clone`.
    insert_child(func, macro_clone);
    unparent(func);

    return macro_clone;
}

static ast::Macro* lookup_referenced_macro(const ast::Domain* domain, const ast::Task* task, const Token_Value& name)
{
    if (task != 0)
    {
        if (ast::Macro* ref_macro = get(task->macro_lookup, name))
        {
            return ref_macro;
        }
    }

    if (ast::Macro* ref_macro = get(domain->macro_lookup, name))
    {
        return ref_macro;
    }

    return 0;
}

static bool disallow_recursive_macro(const ast::Root* tree, const ast::Task* task_scope, const ast::Macro* macro)
{
    Memory_Stack_Scope scratch_scope(tree->scratch);

    ast::Domain* domain_scope = tree->domain;

    Id_Table<bool> macros_seen;
    init(macros_seen, tree->scratch, 8);
    set(macros_seen, macro->name, true);

    ast::Expr* root = create_op(tree, ast::Node_Or);
    append_child(root, macro->expression);

    for (ast::Expr* node = root->child; node != 0; node = preorder_next(root, node))
    {
        if (ast::Func* func = as_Func(node))
        {
            if (ast::Macro* ref_macro = lookup_referenced_macro(domain_scope, task_scope, func->name))
            {
                if (get(macros_seen, func->name))
                {
                    emit(tree, macro->loc, Error_Recursive_Macro) << macro->name;
                    return true;
                }

                set(macros_seen, func->name, true);
                insert_child(node, ref_macro->expression);
            }
        }
    }

    for (ast::Expr* node = root->child; node != 0; )
    {
        ast::Expr* next = node->next_sibling;
        unparent(node);
        node = next;
    }

    return false;
}

static void inline_macros_in_expr(ast::Root* tree, const ast::Domain* domain, const ast::Task* task, ast::Expr* root)
{
    for (ast::Expr* node = root; node != 0; )
    {
        if (ast::Func* func = as_Func(node))
        {
            if (ast::Macro* macro = lookup_referenced_macro(domain, task, func->name))
            {
                node = inline_macro(tree, task, macro, func);
                if (root == func)
                    return;

                continue;
            }
        }

        // syntax sugar for zero-sized argument macros (e.g. constants).
        if (ast::Var* var = as_Var(node))
        {
            if (ast::Macro* macro = lookup_referenced_macro(domain, task, var->name))
            {
                ast::Func* func = create_func(tree, var->name, var->loc);
                insert_child(var, func);
                unparent(var);

                node = inline_macro(tree, task, macro, func);
                if (root == var)
                    return;

                continue;
            }
        }

        node = preorder_next(root, node);
    }
}

void plnnrc::inline_macros(ast::Root* tree)
{
    Memory_Stack_Scope scratch_scope(tree->scratch);

    ast::Domain* domain = tree->domain;

    Array<ast::Macro*> redefines;
    init(redefines, tree->scratch, 16);

    // build lookups.
    init(domain->macro_lookup, tree->pool, size(domain->macros));
    for (uint32_t macro_idx = 0; macro_idx < size(domain->macros); ++macro_idx)
    {
        ast::Macro* macro = domain->macros[macro_idx];
        if (set(domain->macro_lookup, macro->name, macro))
            push_back(redefines, macro);
    }

    for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
    {
        ast::Task* task = domain->tasks[task_idx];
        init(task->macro_lookup, tree->pool, size(task->macros));

        for (uint32_t macro_idx = 0; macro_idx < size(task->macros); ++macro_idx)
        {
            ast::Macro* macro = task->macros[macro_idx];
            if (set(task->macro_lookup, macro->name, macro))
                push_back(redefines, macro);
        }
    }

    bool has_errors = !empty(redefines);

    // check recursion.
    for (uint32_t macro_idx = 0; macro_idx < size(domain->macros); ++macro_idx)
    {
        ast::Macro* macro = domain->macros[macro_idx];
        has_errors |= disallow_recursive_macro(tree, 0, macro);
    }

    for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
    {
        ast::Task* task = domain->tasks[task_idx];
        for (uint32_t macro_idx = 0; macro_idx < size(task->macros); ++macro_idx)
        {
            ast::Macro* macro = task->macros[macro_idx];
            has_errors |= disallow_recursive_macro(tree, task, macro);
        }
    }

    for (uint32_t redef_idx = 0; redef_idx < size(redefines); ++redef_idx)
    {
        ast::Macro* macro = redefines[redef_idx];
        emit(tree, macro->loc, Error_Redefinition) << macro->name;
    }

    if (has_errors)
        return;

    for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
    {
        ast::Task* task = domain->tasks[task_idx];
        // inline macros in each preconditions and task lists.
        for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
        {
            ast::Case* case_ = task->cases[case_idx];
            ast::Expr* precond = case_->precond;
            // add the dummy root node as `inline_macros` may need to replace the precondition root.
            ast::Expr* new_root = create_op(tree, ast::Node_And);
            append_child(new_root, precond);
            case_->precond = new_root;
            inline_macros_in_expr(tree, domain, task, new_root);

            for (uint32_t task_list_idx = 0; task_list_idx < size(case_->task_list); ++task_list_idx)
            {
                ast::Func* callee = as_Func(case_->task_list[task_list_idx]);
                plnnrc_assert(callee);
                for (ast::Expr* arg = callee->child; arg != 0; arg = arg->next_sibling)
                {
                    inline_macros_in_expr(tree, domain, task, arg);
                }
            }
        }
    }

    // inline macros in attributes
    for (uint32_t fact_idx = 0; fact_idx < size(tree->world->facts); ++fact_idx)
    {
        ast::Fact* fact = tree->world->facts[fact_idx];
        for (uint32_t attr_idx = 0; attr_idx < size(fact->attrs); ++attr_idx)
        {
            ast::Attribute* attr = fact->attrs[attr_idx];
            for (uint32_t arg_idx = 0; arg_idx < size(attr->args); ++arg_idx)
            {
                ast::Expr* arg = attr->args[arg_idx];
                ast::Expr* new_root = create_op(tree, ast::Node_And);
                append_child(new_root, arg);
                inline_macros_in_expr(tree, domain, 0, new_root);
            }
        }
    }

    for (uint32_t case_idx = 0; case_idx < size(tree->cases); ++case_idx)
    {
        const ast::Case* case_ = tree->cases[case_idx];
        const ast::Task* task = case_->task;
        for (uint32_t attr_idx = 0; attr_idx < size(case_->attrs); ++attr_idx)
        {
            ast::Attribute* attr = case_->attrs[attr_idx];
            for (uint32_t arg_idx = 0; arg_idx < size(attr->args); ++arg_idx)
            {
                ast::Expr* arg = attr->args[arg_idx];
                ast::Expr* new_root = create_op(tree, ast::Node_And);
                append_child(new_root, arg);
                inline_macros_in_expr(tree, domain, task, new_root);
            }
        }
    }
}

static void replace_underscores_with_unique_names(ast::Root* tree, ast::Expr* expr)
{
    Token_Value underscore;
    underscore.str = "_";
    underscore.length = 1;

    for (ast::Expr* node = expr; node != 0; node = preorder_next(expr, node))
    {
        if (ast::Var* var = as_Var(node))
        {
            if (equal(underscore, var->name))
            {
                const Token_Value name = make_unique_name(tree);
                var->name = name;
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

static void create_fact_ref_literals(const ast::Root* tree, ast::Expr* expr)
{
    // find fact refs and replace with literal nodes.
    for (ast::Expr* node = expr; node != 0; )
    {
        ast::Expr* next = preorder_next(expr, node);

        if (ast::Var* var = as_Var(node))
        {
            if (get_fact(tree, var->name))
            {
                Token tok;
                tok.value = var->name;
                tok.type = Token_Literal_Fact;
                ast::Literal* literal = create_literal(tree, tok, var->loc);
                insert_child(var, literal);
                unparent(var);
            }
        }

        node = next;
    }
}

void plnnrc::annotate(ast::Root* tree)
{
    ast::World* world = tree->world;
    if (world)
    {
        const uint32_t num_facts = size(world->facts);
        init(tree->fact_lookup, tree->pool, num_facts);

        for (uint32_t i = 0; i < num_facts; ++i)
        {
            ast::Fact* fact = world->facts[i];
            if (set(tree->fact_lookup, fact->name, fact))
                emit(tree, fact->loc, Error_Redefinition) << fact->name;
        }
    }

    ast::Primitive* prim = tree->primitive;
    if (prim)
    {
        const uint32_t num_tasks = size(prim->tasks);
        init(tree->primitive_lookup, tree->pool, num_tasks);

        for (uint32_t i = 0; i < num_tasks; ++i)
        {
            ast::Fact* task = prim->tasks[i];
            if (set(tree->primitive_lookup, task->name, task))
                emit(tree, task->loc, Error_Redefinition) << task->name;
        }
    }

    ast::Domain* domain = tree->domain;
    if (domain)
    {
        const uint32_t num_tasks = size(domain->tasks);
        init(tree->task_lookup, tree->pool, num_tasks);

        uint32_t num_cases = 0;
        for (uint32_t i = 0; i < num_tasks; ++i)
        {
            ast::Task* task = domain->tasks[i];
            num_cases += size(task->cases);
            if (set(tree->task_lookup, task->name, task))
                emit(tree, task->loc, Error_Redefinition) << task->name;
        }

        // collect all `ast::Case` nodes.
        if (num_cases > 0)
        {
            init(tree->cases, tree->pool, num_cases);
            for (uint32_t i = 0; i < num_tasks; ++i)
            {
                ast::Task* task = domain->tasks[i];
                for (uint32_t j = 0; j < size(task->cases); ++j)
                {
                    ast::Case* case_ = task->cases[j];
                    push_back(tree->cases, case_);
                }
            }
        }

        // build param_lookup for each task.
        for (uint32_t task_idx = 0; task_idx < size(domain->tasks); ++task_idx)
        {
            ast::Task* task = domain->tasks[task_idx];
            init(task->param_lookup, tree->pool, 8);

            for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
            {
                ast::Param* param = task->params[param_idx];
                if (set(task->param_lookup, param->name, param))
                    emit(tree, param->loc, Error_Redefinition) << param->name;
            }
        }

        // build ast::Var lookups for each case.
        for (uint32_t case_idx = 0; case_idx < size(tree->cases); ++case_idx)
        {
            ast::Case* case_ = tree->cases[case_idx];
            plnnrc_assert(is_Or(case_->precond)); // expecting precondition in the DNF form.

            ast::Task* task = case_->task;

            create_fact_ref_literals(tree, case_->precond);

            init(case_->precond_var_lookup, tree->pool, 8);
            init(case_->precond_vars, tree->pool, 8);
            init(case_->task_list_var_lookup, tree->pool, 8);
            init(case_->task_list_vars, tree->pool, 8);

            replace_underscores_with_unique_names(tree, case_->precond);
            build_var_lookup(case_->precond, case_->precond_vars, case_->precond_var_lookup);

            for (uint32_t task_idx = 0; task_idx < size(case_->task_list); ++task_idx)
            {
                ast::Expr* task_expr = case_->task_list[task_idx];

                create_fact_ref_literals(tree, task_expr);

                replace_underscores_with_unique_names(tree, task_expr);
                build_var_lookup(task_expr, case_->task_list_vars, case_->task_list_var_lookup);

                for (ast::Expr* node = task_expr; node != 0; node = preorder_next(task_expr, node))
                {
                    ast::Func* func = as_Func(node);
                    if (!func)
                        continue;

                    init(func->args, tree->pool, 8);
                    for (ast::Expr* child = func->child; child != 0; child = child->next_sibling)
                    {
                        push_back(func->args, child);
                    }
                }
            }

            // set `definition` for vars referencing task parameters.
            for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
            {
                ast::Param* param = task->params[param_idx];
                if (ast::Var* var = get(case_->precond_var_lookup, param->name))
                {
                    plnnrc_assert(!var->definition);
                    var->definition = param;
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
            }

            // build `precond_facts`
            init(case_->precond_facts, tree->pool, 8);
            for (ast::Expr* node = case_->precond; node != 0; node = preorder_next(case_->precond, node))
            {
                ast::Func* func = as_Func(node);
                if (!func)
                    continue;

                init(func->args, tree->pool, 8);
                for (ast::Expr* child = func->child; child != 0; child = child->next_sibling)
                {
                    push_back(func->args, child);
                }

                ast::Fact* fact = get(tree->fact_lookup, func->name);
                if (!fact)
                    continue;

                push_back(case_->precond_facts, fact);
            }

            // mark ast::Var binding occurrences.
            {
                Memory_Stack_Scope scratch_scope(tree->scratch);
                Id_Table<ast::Var*> seen;
                init(seen, tree->scratch, size(case_->precond_var_lookup));

                for (ast::Expr* conjunct = case_->precond->child; conjunct != 0; conjunct = conjunct->next_sibling)
                {
                    for (ast::Expr* node = conjunct; node != 0; node = preorder_next(conjunct, node))
                    {
                        ast::Var* var = as_Var(node);
                        if (!var)
                            continue;

                        if (as_Param(var->definition) != 0)
                            continue;

                        if (get(seen, var->name) == 0)
                        {
                            var->binding = true;
                            set(seen, var->name, var);
                        }
                    }

                    clear(seen);
                }
            }
        }
    }
}

static const uint32_t Num_Types = Token_Group_Type_Last - Token_Group_Type_First + 1;

static Token_Type s_unification_table[Num_Types][Num_Types] =
{
//                  Id32                  Id64                Int8                    Int32               Int64               Float               Vec3              Fact_Ref,           Any               
/* Id32 */      { Token_Id32,           Token_Not_A_Type,   Token_Not_A_Type,       Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Id32,     },
/* Id64 */      { Token_Not_A_Type,     Token_Id64,         Token_Not_A_Type,       Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Id64,     },
/* Int8 */      { Token_Not_A_Type,     Token_Not_A_Type,   Token_Int8,             Token_Int32,        Token_Int64,        Token_Float,        Token_Not_A_Type,   Token_Not_A_Type,   Token_Int8,     },
/* Int32 */     { Token_Not_A_Type,     Token_Not_A_Type,   Token_Int32,            Token_Int32,        Token_Int64,        Token_Float,        Token_Not_A_Type,   Token_Not_A_Type,   Token_Int32,    },
/* Int64 */     { Token_Not_A_Type,     Token_Not_A_Type,   Token_Int64,            Token_Int64,        Token_Int64,        Token_Float,        Token_Not_A_Type,   Token_Not_A_Type,   Token_Int64,    },
/* Float */     { Token_Not_A_Type,     Token_Not_A_Type,   Token_Float,            Token_Float,        Token_Float,        Token_Float,        Token_Not_A_Type,   Token_Not_A_Type,   Token_Float,    },
/* Vec3 */      { Token_Not_A_Type,     Token_Not_A_Type,   Token_Not_A_Type,       Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Vec3,         Token_Not_A_Type,   Token_Vec3,     },
/* Fact_Ref */  { Token_Not_A_Type,     Token_Not_A_Type,   Token_Not_A_Type,       Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Not_A_Type,   Token_Fact_Ref,     Token_Fact_Ref, },
/* Any */       { Token_Id32,           Token_Id64,         Token_Int8,             Token_Int32,        Token_Int64,        Token_Float,        Token_Vec3,         Token_Not_A_Type,   Token_Any_Type, },
};

Token_Type plnnrc::unify(Token_Type a, Token_Type b)
{
    plnnrc_assert(is_Type(a));
    plnnrc_assert(is_Type(b));
    return s_unification_table[a - Token_Group_Type_First][b - Token_Group_Type_First];
}

static bool assign_fact_types(const ast::Root* tree, Id_Table<Token_Type>& var_types, ast::Func* const func, const ast::Fact* fact)
{
    // assign variable types based on the fact they're used in.
    for (uint32_t param_idx = 0; param_idx < size(fact->params); ++param_idx)
    {
        const ast::Data_Type* param_type = fact->params[param_idx];
        ast::Expr* arg = func->args[param_idx];

        if (ast::Var* var = as_Var(arg))
        {
            Token_Type* curr_type = get(var_types, var->name);
            plnnrc_assert(curr_type);

            Token_Type unified_type = unify(*curr_type, param_type->data_type);
            var->data_type = unified_type;

            if (is_Not_A_Type(unified_type))
            {
                emit(tree, var->loc, Error_Failed_To_Unify_Type) << var->name << *curr_type << param_type->data_type;
                return false;
            }

            set(var_types, var->name, unified_type);
        }
    }

    return true;
}

static void init_var_types(Id_Table<Token_Type>& var_types, Memory* mem, const ast::Task* task, const ast::Case* case_)
{
    init(var_types, mem, size(case_->precond_var_lookup) + size(case_->task_list_var_lookup) + size(task->params));

    for (uint32_t var_idx = 0; var_idx < size(case_->precond_vars); ++var_idx)
    {
        ast::Var* var = case_->precond_vars[var_idx];
        set(var_types, var->name, Token_Any_Type);
    }

    for (uint32_t var_idx = 0; var_idx < size(case_->task_list_vars); ++var_idx)
    {
        ast::Var* var = case_->task_list_vars[var_idx];
        set(var_types, var->name, Token_Any_Type);
    }

    for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
    {
        ast::Param* param = task->params[param_idx];
        set(var_types, param->name, Token_Any_Type);
    }
}

static bool is_assignment_target(const ast::Var* var)
{
    const ast::Expr* parent = var->parent;
    return is_Assign(parent) && (parent->child == var);
}

static bool check_all_precond_vars_bound(const ast::Root* tree, const ast::Case* case_)
{
    const uint32_t err_count = size(*tree->errs);

    for (uint32_t var_idx = 0; var_idx < size(case_->precond_vars); ++var_idx)
    {
        const ast::Var* var = case_->precond_vars[var_idx];

        // this var is not a parameter usage, nor used inside fact or primitive task.
        if (var->binding && is_Unknown(var->data_type))
            emit(tree, var->loc, Error_Unbound_Var) << var->original_name;
    }

    return err_count == size(*tree->errs);
}

static bool has_var_in_all_conjuncts(const ast::Expr* precond, const ast::Var* var)
{
    const uint32_t hash_val = hash(var->name);
    plnnrc_assert(is_Or(precond)); // must be in Disjunctive-Normal-Form.

    for (ast::Expr* conjunct = precond->child; conjunct != 0; conjunct = conjunct->next_sibling)
    {
        bool found = false;
        for (ast::Expr* n = conjunct; n != 0; n = preorder_next(conjunct, n))
        {
            ast::Var* v = as_Var(n);
            if (!v)
                continue;

            const uint32_t h = hash(v->name);
            if (h != hash_val)
                continue;

            if (equal(v->name, var->name))
                found = true;
        }

        if (!found)
            return false;
    }

    return true;
}

static bool init_precond_variable_assignments(const ast::Root* tree)
{
    const uint32_t err_count = size(*tree->errs);

    for (uint32_t task_idx = 0; task_idx < size(tree->domain->tasks); ++task_idx)
    {
        const ast::Task* task = tree->domain->tasks[task_idx];

        for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
        {
            const ast::Case* case_ = task->cases[case_idx];

            for (uint32_t var_idx = 0; var_idx < size(case_->precond_vars); ++var_idx)
            {
                ast::Var* var = case_->precond_vars[var_idx];

                // variable is an assignment target.
                if (is_assignment_target(var))
                {
                    if (!var->binding)
                        emit(tree, var->loc, Error_Already_Bound) << var->name;

                    var->data_type = Token_Any_Type;
                }
            }
        }
    }

    return size(*tree->errs) == err_count;
}

static bool check_all_task_list_vars_bound(const ast::Root* tree)
{
    bool failed = false;

    for (uint32_t task_idx = 0; task_idx < size(tree->domain->tasks); ++task_idx)
    {
        const ast::Task* task = tree->domain->tasks[task_idx];

        for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
        {
            const ast::Case* case_ = task->cases[case_idx];

            for (uint32_t var_idx = 0; var_idx < size(case_->task_list_vars); ++var_idx)
            {
                ast::Var* var = case_->task_list_vars[var_idx];

                if (var->definition)
                {
                    if (is_Param(var->definition))
                        continue;

                    if (!has_var_in_all_conjuncts(case_->precond, var))
                    {
                        emit(tree, var->loc, Error_Not_Bound_In_All_Conjuncts) << var->name;
                        failed = true;
                    }

                    continue;
                }

                emit(tree, var->loc, Error_Unbound_Var) << var->original_name;
                failed = true;
            }
        }
    }

    return !failed;
}

static void update_var_occurrence_types(const Id_Table<Token_Type>& var_types, const Array<ast::Var*>& vars)
{
    for (uint32_t var_idx = 0; var_idx < size(vars); ++var_idx)
    {
        ast::Var* var = vars[var_idx];

        Token_Type* new_type = get(var_types, var->name);
        plnnrc_assert(new_type != 0);

        if (is_Any_Type(*new_type))
            continue;

        var->data_type = *new_type;
    }
}

static bool update_var_occurrence_types_after_params_update(const Array<ast::Var*>& vars)
{
    for (uint32_t var_idx = 0; var_idx < size(vars); ++var_idx)
    {
        ast::Var* var = vars[var_idx];
        const ast::Param* param = as_Param(var->definition);

        if (!param)
            continue;

        var->data_type = param->data_type;
    }

    return true;
}

static bool infer_local_types(const ast::Root* tree, const ast::Task* task)
{
    for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
    {
        ast::Param* param = task->params[param_idx];
        param->data_type = Token_Any_Type;
    }

    for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
    {
        const ast::Case* case_ = task->cases[case_idx];

        Memory_Stack_Scope scratch_scope(tree->scratch);
        Id_Table<Token_Type> var_types;
        init_var_types(var_types, tree->scratch, task, case_);

        // seed types in precondition using fact declarations.
        for (ast::Expr* node = case_->precond; node != 0; node = preorder_next(case_->precond, node))
        {
            ast::Func* func = as_Func(node);
            if (!func)
                continue;

            const ast::Fact* fact = get_fact(tree, func->name);
            if (!fact)
                continue;

            if (size(fact->params) != size(func->args))
            {
                emit(tree, func->loc, Error_Mismatching_Number_Of_Args) << func->name;
                return false;
            }

            if (!assign_fact_types(tree, var_types, func, fact))
                return false;
        }

        // seed types in task list using primitive task declarations.
        for (uint32_t task_list_idx = 0; task_list_idx < size(case_->task_list); ++task_list_idx)
        {
            ast::Expr* expr = case_->task_list[task_list_idx];
            ast::Func* func = as_Func(expr);
            plnnrc_assert(func);

            if (!get_primitive(tree, func->name) && !get_task(tree, func->name))
            {
                emit(tree, func->loc, Error_Undeclared_Task) << func->name;
                break;
            }

            if (const ast::Task* callee = get_task(tree, func->name))
            {
                if (size(callee->params) != size(func->args))
                {
                    emit(tree, func->loc, Error_Mismatching_Number_Of_Args) << func->name;
                    return false;
                }

                continue;
            }

            const ast::Fact* prim = get_primitive(tree, func->name);
            if (!prim)
                continue;

            if (size(prim->params) != size(func->args))
            {
                emit(tree, func->loc, Error_Mismatching_Number_Of_Args) << func->name;
                return false;
            }

            if (!assign_fact_types(tree, var_types, func, prim))
                return false;
        }

        check_all_precond_vars_bound(tree, case_);

        // unify task parameters for each new case.
        for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
        {
            ast::Param* param = task->params[param_idx];
            const Token_Type* new_type = get(var_types, param->name);

            const Token_Type unified_type = unify(*new_type, param->data_type);

            if (is_Not_A_Type(unified_type))
            {
                emit(tree, param->loc, Error_Failed_To_Unify_Type) << param->name << param->data_type << *new_type;
                return false;
            }

            param->data_type = unified_type;
        }

        // assign new types to all variable occurences.
        update_var_occurrence_types(var_types, case_->precond_vars);
        update_var_occurrence_types(var_types, case_->task_list_vars);
    }

    return true;
}

static Token_Type get_op_token_type(ast::Node_Type node_type)
{
    switch (node_type)
    {
    #define PLNNRC_NODE_OP(TAG) \
        case ast::Node_##TAG: return Token_##TAG;
    #include "derplanner/compiler/ast_tags.inl"
    #undef PLNNRC_NODE_OP
        default:
            plnnrc_assert(false);
            return Token_Unknown;
    }
}

static bool resolve_function_call(const ast::Root* tree, ast::Func* func);

struct Compute_Expr_Result_Type
{
    const ast::Root* tree;

    Token_Type visit(ast::Var* node)
    {
        if (is_Unknown(node->data_type))
            return Token_Not_A_Type;

        return node->data_type;
    }

    Token_Type visit(ast::Literal* node)
    {
        plnnrc_assert(is_Literal(node->value_type));

        if (is_Literal_Fact(node->value_type))
            return Token_Fact_Ref;

        if (is_Literal_Symbol(node->value_type))
            return Token_Id32;

        if (is_Literal_Float(node->value_type))
            return Token_Float;

        if (is_Literal_Integer(node->value_type))
            return get_integral_literal_type(node);

        plnnrc_assert(false);
        return Token_Not_A_Type;
    }

    Token_Type get_integral_literal_type(const ast::Literal* node)
    {
        const int64_t val = as_int(node);

        if (val >= std::numeric_limits<int8_t>::min() && val <= std::numeric_limits<int8_t>::max())
            return Token_Int8;

        if (val >= std::numeric_limits<int32_t>::min() && val <= std::numeric_limits<int32_t>::max())
            return Token_Int32;

        return Token_Int64;
    }

    Token_Type visit(ast::Op* node)
    {
        if (is_Logical(node) || is_Comparison(node))
        {
            for (ast::Expr* child = node->child; child != 0; child = child->next_sibling)
            {
                const Token_Type arg_type = visit_node<Token_Type>(child, this);

                if (is_Not_A_Type(arg_type))
                    return Token_Not_A_Type;

                // must be unifiable with Int8
                const Token_Type unified_type = unify(Token_Int8, arg_type);

                if (is_Not_A_Type(unified_type))
                {
                    emit(tree, child->loc, Error_Expected_Argument_Type) << Token_Int8 << get_op_token_type(node->type) << arg_type;
                    break;
                }
            }

            return Token_Int8;
        }

        if (is_Arithmetic(node))
        {
            Token_Type result_type = Token_Any_Type;

            for (ast::Expr* child = node->child; child != 0; child = child->next_sibling)
            {
                Token_Type arg_type = visit_node<Token_Type>(child, this);

                if (is_Not_A_Type(arg_type))
                    return Token_Not_A_Type;

                result_type = unify(result_type, arg_type);

                if (is_Not_A_Type(result_type))
                {
                    emit(tree, child->loc, Error_Expected_Argument_Type) << Token_Int8 << get_op_token_type(node->type) << arg_type;
                    break;
                }
            }

            return result_type;
        }

        plnnrc_assert(false);
        return Token_Not_A_Type;
    }

    Token_Type visit(ast::Func* node)
    {
        if (!resolve_function_call(tree, node))
            return Token_Not_A_Type;

        return get_return_type(tree->functions, node->signature_index);
    }

    Token_Type visit(ast::Node*) { plnnrc_assert(false); return Token_Not_A_Type; }
};

static bool infer_global_types(const ast::Root* tree)
{
    bool updated = false;

    for (uint32_t task_idx = 0; task_idx < size(tree->domain->tasks); ++task_idx)
    {
        const ast::Task* task = tree->domain->tasks[task_idx];
        for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
        {
            const ast::Case* case_ = task->cases[case_idx];

            for (uint32_t task_list_idx = 0; task_list_idx < size(case_->task_list); ++task_list_idx)
            {
                const ast::Func* func = as_Func(case_->task_list[task_list_idx]);
                plnnrc_assert(func);
                const ast::Task* callee = get_task(tree, func->name);
                if (!callee)
                    continue;

                for (uint32_t param_idx = 0; param_idx < size(callee->params); ++param_idx)
                {
                    ast::Param* param = callee->params[param_idx];
                    ast::Expr* arg = func->args[param_idx];

                    if (const ast::Var* var = as_Var(arg))
                    {
                        if (is_Unknown(var->data_type))
                            continue;

                        const Token_Type unified_type = unify(var->data_type, param->data_type);
                        if (is_Not_A_Type(unified_type))
                        {
                            emit(tree, param->loc, Error_Failed_To_Unify_Type) << param->name << var->data_type << param->data_type;
                            return false;
                        }

                        updated |= (param->data_type != unified_type);
                        param->data_type = unified_type;
                        continue;
                    }

                    // expression is used as an argument
                    {
                        Compute_Expr_Result_Type visitor = { tree };
                        const Token_Type arg_type = visit_node<Token_Type>(arg, &visitor);
                        if (is_Not_A_Type(arg_type))
                            continue;

                        const Token_Type unified_type = unify(arg_type, param->data_type);
                        if (is_Not_A_Type(unified_type))
                            continue;

                        updated |= (param->data_type != unified_type);
                        param->data_type = unified_type;
                        continue;
                    }
                }
            }

            for (ast::Expr* node = case_->precond; node != 0; node = preorder_next(case_->precond, node))
            {
                if (!is_Assign(node))
                    continue;

                ast::Var* var = as_Var(node->child);
                plnnrc_assert(var);
                ast::Expr* rhs = node->child->next_sibling;
                plnnrc_assert(rhs);

                Compute_Expr_Result_Type visitor = { tree };
                const Token_Type rhs_type = visit_node<Token_Type>(rhs, &visitor);
                if (is_Not_A_Type(rhs_type))
                    continue;

                const Token_Type unified_type = unify(rhs_type, var->data_type);
                if (is_Not_A_Type(unified_type))
                    continue;

                updated |= (var->data_type != unified_type);
                var->data_type = unified_type;
                continue;
            }

            update_var_occurrence_types_after_params_update(case_->precond_vars);
            update_var_occurrence_types_after_params_update(case_->task_list_vars);
        }
    }

    return updated;
}

static bool infer_params_from_local_task_lists(const ast::Root* tree)
{
    for (uint32_t task_idx = 0; task_idx < size(tree->domain->tasks); ++task_idx)
    {
        const ast::Task* task = tree->domain->tasks[task_idx];
        for (uint32_t case_idx = 0; case_idx < size(task->cases); ++case_idx)
        {
            const ast::Case* case_ = task->cases[case_idx];
            for (uint32_t task_list_idx = 0; task_list_idx < size(case_->task_list); ++task_list_idx)
            {
                const ast::Func* func = as_Func(case_->task_list[task_list_idx]);
                plnnrc_assert(func);
                const ast::Task* callee = get_task(tree, func->name);
                if (!callee)
                    continue;

                for (uint32_t param_idx = 0; param_idx < size(callee->params); ++param_idx)
                {
                    const ast::Param* param = callee->params[param_idx];
                    ast::Expr* arg = func->args[param_idx];
                    ast::Var* var = as_Var(arg);

                    if (is_Any_Type(param->data_type))
                        continue;

                    if (!var)
                        continue;

                    if (!is_Any_Type(var->data_type))
                        continue;

                    var->data_type = param->data_type;
                    ast::Param* def = as_Param(var->definition);
                    plnnrc_assert(def);

                    const Token_Type unified_type = unify(def->data_type, var->data_type);
                    if (is_Not_A_Type(unified_type))
                    {
                        emit(tree, def->loc, Error_Failed_To_Unify_Type) << def->name << def->data_type << var->data_type;
                        return false;
                    }

                    def->data_type = unified_type;
                }
            }
        }
    }

    return true;
}

static bool check_all_task_params_inferred(const ast::Root* tree)
{
    bool has_undefined = false;

    for (uint32_t task_idx = 0; task_idx < size(tree->domain->tasks); ++task_idx)
    {
        const ast::Task* task = tree->domain->tasks[task_idx];
        for (uint32_t param_idx = 0; param_idx < size(task->params); ++param_idx)
        {
            const ast::Param* param = task->params[param_idx];
            if (is_Any_Type(param->data_type))
            {
                emit(tree, param->loc, Error_Failed_To_Infer_Type) << param->name;
                has_undefined = true;
            }
        }
    }

    return !has_undefined;
}

template <typename Def>
static bool check_arguments(const ast::Root* tree, const ast::Func* func, const Def* definition)
{
    Compute_Expr_Result_Type visitor = { tree };
    for (uint32_t arg_idx = 0; arg_idx < size(func->args); ++arg_idx)
    {
        ast::Expr* arg = func->args[arg_idx];
        const Token_Type data_type = definition->params[arg_idx]->data_type;
        const Token_Type type = visit_node<Token_Type>(arg, &visitor);

        if (is_Not_A_Type(type))
        {
            return false;
        }

        if (is_Not_A_Type(unify(type, data_type)))
        {
            emit(tree, arg->loc, Error_Expected_Argument_Type) << data_type << func->name << type;
            return false;
        }
    }

    return true;
}

static bool resolve_function_call(const ast::Root* tree, ast::Func* func)
{
    // cache argument types.
    init(func->arg_types, tree->pool, size(func->args));

    Compute_Expr_Result_Type visitor = { tree };
    for (uint32_t arg_idx = 0; arg_idx < size(func->args); ++arg_idx)
    {
        ast::Expr* arg = func->args[arg_idx];
        const Token_Type type = visit_node<Token_Type>(arg, &visitor);

        if (is_Not_A_Type(type))
            return false;

        push_back(func->arg_types, type);
    }

    func->signature_index = resolve(tree->functions, func->name, func->arg_types);

    if (func->signature_index < num_signatures(tree->functions))
    {
        return true;
    }

    emit(tree, func->loc, Error_Failed_To_Resolve_Call) << func;
    return false;
}

static bool check_expression_types(const ast::Root* tree)
{
    const uint32_t err_count = size(*tree->errs);

    for (uint32_t case_idx = 0; case_idx < size(tree->cases); ++case_idx)
    {
        const ast::Case* case_ = tree->cases[case_idx];
        const ast::Expr* precond = case_->precond;

        for (ast::Expr* node = precond->child; node != 0; node = preorder_next(precond->child, node))
        {
            if (ast::Func* func = as_Func(node))
            {
                const ast::Fact* fact = get_fact(tree, func->name);

                if (fact)
                {
                    check_arguments(tree, func, fact);
                    continue;
                }

                if (!resolve_function_call(tree, func))
                    break;
            }
        }

        for (uint32_t task_list_idx = 0; task_list_idx < size(case_->task_list); ++task_list_idx)
        {
            const ast::Func* callee = as_Func(case_->task_list[task_list_idx]);
            plnnrc_assert(callee);

            if (const ast::Fact* prim = get_primitive(tree, callee->name))
                check_arguments(tree, callee, prim);

            if (const ast::Task* task = get_task(tree, callee->name))
                check_arguments(tree, callee, task);
        }
    }

    return size(*tree->errs) == err_count;
}

struct Assign_Var_Defs_And_Types
{
    const ast::Root* tree;
    const ast::Case* case_;

    bool visit(ast::Var* node)
    {
        ast::Task* task = case_->task;

        if (ast::Param* def = get(task->param_lookup, node->name))
        {
            node->definition = def;
            node->data_type = def->data_type;
            return true;
        }

        if (ast::Var* def = get(case_->precond_var_lookup, node->name))
        {
            node->definition = def;
            node->data_type = def->data_type;
            return true;
        }

        emit(tree, node->loc, Error_Unbound_Var) << node->original_name;
        return false;
    }

    bool visit(ast::Expr* node)
    {
        for (ast::Expr* child = node->child; child != 0; child = child->next_sibling)
        {
            if (!visit_node<bool>(child, this))
                return false;
        }

        return true;
    }

    bool visit(ast::Node*) { plnnrc_assert(false); return false; }
};

struct Resolve_Function_Calls
{
    const ast::Root* tree;

    bool visit(ast::Func* node)
    {
        return resolve_function_call(tree, node);
    }

    bool visit(ast::Expr* node)
    {
        for (ast::Expr* child = node->child; child != 0; child = child->next_sibling)
        {
            if (!visit_node<bool>(child, this))
                return false;
        }

        return true;
    }

    bool visit(ast::Node*) { plnnrc_assert(false); return false; }
};

struct Is_Const_Expr
{
    bool visit(const ast::Var*)
    {
        return false;
    }

    bool visit(const ast::Expr* node)
    {
        for (ast::Expr* child = node->child; child != 0; child = child->next_sibling)
        {
            if (!visit_node<bool>(child, this))
                return false;
        }

        return true;
    }

    bool visit(const ast::Node*) { plnnrc_assert(false); return false; }
};

static bool process_attributes(const ast::Root* tree, const Array<ast::Attribute*>& attrs)
{
    const uint32_t err_count = size(*tree->errs);

    uint32_t attr_counts[Attribute_Count];
    memset(attr_counts, 0, sizeof(uint32_t) * Attribute_Count);

    for (uint32_t attr_idx = 0; attr_idx < size(attrs); ++attr_idx)
    {
        ast::Attribute* attr = attrs[attr_idx];
        Attribute_Type attr_type = attr->attr_type;

        if (attr_type >= Attribute_Count)
            continue;

        attr_counts[attr_type] += 1;

        if (attr_counts[attr_type] > 1)
        {
            emit(tree, attr->loc, Error_Only_Single_Attr_Allowed) << attr->name;
            continue;
        }

        const uint32_t attr_num_args = get_num_args(attr_type);
        if (size(attr->args) != attr_num_args)
        {
            emit(tree, attr->loc, Error_Mismatching_Number_Of_Args) << attr->name;
            continue;
        }

        const Attribute_Arg_Class* arg_classes = get_arg_classes(attr_type);
        for (uint32_t arg_idx = 0; arg_idx < attr_num_args; ++arg_idx)
        {
            if (arg_classes[arg_idx] != Attribute_Arg_Constant_Expression)
                continue;

            Is_Const_Expr visitor;
            if (!visit_node<bool>(attr->args[arg_idx], &visitor))
            {
                emit(tree, attr->loc, Error_Only_Const_Expr_Allowed) << attr->name;
                break;
            }
        }
    }

    return err_count == size(*tree->errs);
}

static bool process_attributes(const ast::Root* tree, const Array<ast::Attribute*>& attrs, const ast::Case* case_)
{
    const uint32_t err_count = size(*tree->errs);

    process_attributes(tree, attrs);

    Assign_Var_Defs_And_Types var_visitor = { tree, case_ };
    Resolve_Function_Calls func_visitor = { tree };
    Compute_Expr_Result_Type type_visitor = { tree };

    for (uint32_t attr_idx = 0; attr_idx < size(attrs); ++attr_idx)
    {
        ast::Attribute* attr = attrs[attr_idx];
        const Attribute_Type attr_type = attr->attr_type;

        if (attr_type >= Attribute_Count)
            continue;

        const uint32_t attr_num_args = get_num_args(attr_type);
        const Attribute_Arg_Class* arg_classes = get_arg_classes(attr_type);
        for (uint32_t arg_idx = 0; arg_idx < attr_num_args; ++arg_idx)
        {
            if (arg_classes[arg_idx] != Attribute_Arg_Expression)
                continue;

            ast::Expr* arg = attr->args[arg_idx];
            visit_node<bool>(arg, &var_visitor);
            visit_node<bool>(arg, &func_visitor);

            for (const ast::Expr* node = arg; node != 0; node = preorder_next(arg, node))
            {
                const ast::Var* var = as_Var(node);
                if (!var)
                    continue;

                if (is_Param(var->definition))
                    continue;

                if (!has_var_in_all_conjuncts(case_->precond, var))
                    emit(tree, var->loc, Error_Not_Bound_In_All_Conjuncts) << var->name;
            }
        }

        init(attr->types, tree->pool, size(attr->args));
        resize(attr->types, size(attr->args));
        for (uint32_t arg_idx = 0; arg_idx < size(attr->args); ++arg_idx)
        {
            ast::Expr* arg = attr->args[arg_idx];
            const Token_Type arg_type = visit_node<Token_Type>(arg, &type_visitor);
            attr->types[arg_idx] = arg_type;
        }
    }

    return err_count == size(*tree->errs);
}

static bool process_attributes(const ast::Root* tree)
{
    const uint32_t err_count = size(*tree->errs);

    for (uint32_t fact_idx = 0; fact_idx < size(tree->world->facts); ++fact_idx)
    {
        const ast::Fact* fact = tree->world->facts[fact_idx];
        process_attributes(tree, fact->attrs);
    }

    for (uint32_t case_idx = 0; case_idx < size(tree->cases); ++case_idx)
    {
        const ast::Case* case_ = tree->cases[case_idx];
        process_attributes(tree, case_->attrs, case_);
    }

    return size(*tree->errs) == err_count;
}

bool plnnrc::infer_types(const ast::Root* tree)
{
    const uint32_t err_count = size(*tree->errs);

    if (!check_all_task_list_vars_bound(tree))
        return false;

    if (!init_precond_variable_assignments(tree))
        return false;

    // the first step is local: we set and unify variable types based on their usage in facts and primitive tasks.
    // task parameter types are also set in this stage, if it's possible to derive them from their usage in the task's preconditions.
    for (uint32_t task_idx = 0; task_idx < size(tree->domain->tasks); ++task_idx)
    {
        const ast::Task* task = tree->domain->tasks[task_idx];
        infer_local_types(tree, task);
    }

    if (size(*tree->errs) > err_count)
        return false;

    // the second step is global: we infer task parameter types based on the usage in the task lists in entire domain.
    // variables referencing the updated parameters are also updated.
    for (;;)
    {
        const bool has_updates = infer_global_types(tree);
        if (!has_updates)
            break;
    }

    if (size(*tree->errs) > err_count)
        return false;

    // some task parameters could still have unknown type, try to set it based on the usage in the task's task lists.
    if (!infer_params_from_local_task_lists(tree))
        return false;

    // loop through tasks and give errors for type-less params.
    if (!check_all_task_params_inferred(tree))
        return false;

    // compute expression types and check them against expected parameter types.
    if (!check_expression_types(tree))
        return false;

    if (!process_attributes(tree))
        return false;

    return true;
}

struct Attribute_Args_Spec
{
    Attribute_Arg_Class classes[2];
};

static const Attribute_Args_Spec s_attribute_specs[] =
{
    { { } },
    #define PLNNRC_ATTRIBUTE(TAG, STR) { {
    #define PLNNRC_ATTRIBUTE_ARG(TYPE) Attribute_Arg_##TYPE,
    #define PLNNRC_ATTRIBUTE_END } },
    #include "derplanner/compiler/attribute_tags.inl"
    #undef PLNNRC_ATTRIBUTE_END
    #undef PLNNRC_ATTRIBUTE_ARG
    #undef PLNNRC_ATTRIBUTE
};

static const uint32_t s_attribute_arg_count[] =
{
    0,
    #define PLNNRC_ATTRIBUTE(TAG, STR) (0
    #define PLNNRC_ATTRIBUTE_ARG(TYPE) +1
    #define PLNNRC_ATTRIBUTE_END ),
    #include "derplanner/compiler/attribute_tags.inl"
    #undef PLNNRC_ATTRIBUTE_END
    #undef PLNNRC_ATTRIBUTE_ARG
    #undef PLNNRC_ATTRIBUTE
};

uint32_t plnnrc::get_num_args(Attribute_Type type)
{
    if (type >= Attribute_Count)
        return 0;

    return s_attribute_arg_count[type];
}

const Attribute_Arg_Class* plnnrc::get_arg_classes(Attribute_Type type)
{
    if (type >= Attribute_Count)
        return 0;

    return s_attribute_specs[type].classes;
}

static const char* s_node_type_names[] =
{
    "None",
    #define PLNNRC_NODE(TAG, TYPE) #TAG,
    #include "derplanner/compiler/ast_tags.inl"
    #undef PLNNRC_NODE
    "Count",
};

const char* plnnrc::get_type_name(ast::Node_Type node_type)
{
    return s_node_type_names[node_type];
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
    void print_value(const T* node) { writeln(*fmtr, "%s[%n]", get_type_name(node->type), node->value); }

    template <typename T>
    void print_data_type(const T* node)
    {
        Indent_Scope s(*fmtr);
        writeln(*fmtr, ":%s", get_type_name(node->data_type));
    }

    void visit(const ast::World* node) { print(node); print_children(node->facts); }

    void visit(const ast::Primitive* node) { print(node); print_children(node->tasks); }

    void visit(const ast::Attribute* node) { print_named(node); print_children(node->args); }

    void visit(const ast::Domain* node) { print_named(node); print_children(node->macros); print_children(node->tasks); }

    void visit(const ast::Fact* node) { print_named(node); print_children(node->params); print_children(node->attrs); }

    void visit(const ast::Macro* node) { print_named(node); print_children(node->params); print_expr(node->expression); }

    void visit(const ast::Task* node) { print_named(node); print_children(node->params); print_children(node->macros); print_children(node->cases); }

    void visit(const ast::Case* node) { print(node); print_children(node->attrs); print_expr(node->precond); print_children(node->task_list); }

    void visit(const ast::Param* node) { print_named(node); print_data_type(node); }

    void visit(const ast::Var* node) { print_named(node); print_data_type(node); }

    void visit(const ast::Func* node) { print_named(node); print_children(node); }

    void visit(const ast::Expr* node) { print(node); print_children(node); }

    void visit(const ast::Data_Type* node) { print(node); print_data_type(node); }

    void visit(const ast::Literal* node) { print_value(node); }
};

void plnnrc::debug_output_ast(const ast::Root* tree, Writer* output)
{
    Formatter fmtr;
    init(fmtr, "  ", "\n", output);
    newline(fmtr);

    Debug_Output_Visitor visitor = { &fmtr };

    if (tree->world)
    {
        visit_node<void>(tree->world, &visitor);
    }

    if (tree->primitive)
    {
        visit_node<void>(tree->primitive, &visitor);
    }

    if (tree->domain)
    {
        visit_node<void>(tree->domain, &visitor);
    }

    newline(fmtr);
}
