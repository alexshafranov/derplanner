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

#ifndef DERPLANNER_COMPILER_AST_H_
#define DERPLANNER_COMPILER_AST_H_

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/types.h"

namespace plnnrc {

// create AST.
void init(ast::Root& root, Array<Error>* errors, Memory_Stack* mem_pool, Memory_Stack* mem_scratch);

/// `create_*` functions for `ast` node types.

ast::World*         create_world(ast::Root* tree);
ast::Primitive*     create_primitive(ast::Root* tree);
ast::Predicate*     create_predicate(ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Fact*          create_fact(ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Param*         create_param(ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Domain*        create_domain(ast::Root* tree, const Token_Value& name);
ast::Task*          create_task(ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Case*          create_case(ast::Root* tree);
ast::Func*          create_func(ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Op*            create_op(ast::Root* tree, ast::Node_Type operation_type);
ast::Var*           create_var(ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Data_Type*     create_type(ast::Root* tree, Token_Type data_type);
ast::Literal*       create_literal(ast::Root* tree, const Token& token);

// lookup `ast::Task` node by name.
ast::Task*      get_task(ast::Root& tree, const Token_Value& name);
// lookup `ast::Fact` node by name.
ast::Fact*      get_fact(ast::Root& tree, const Token_Value& name);
// lookup `ast::Fact` node for primitive task by name.
ast::Fact*      get_primitive(ast::Root& tree, const Token_Value& name);

/// ast::Expr

// make node `child` the last child of node `parent`.
void        append_child(ast::Expr* parent, ast::Expr* child);
// make `child` the next sibling of `after`.
void        insert_child(ast::Expr* after, ast::Expr* child);
// unparent `node` from it's current parent.
void        unparent(ast::Expr* node);
// returns the next node in pre-order (visit node then visit it's children) traversal.
ast::Expr*  preorder_next(const ast::Expr* root, ast::Expr* current);

// calls a proper `visit` method overload depending on the node type.
template <typename Return_Type, typename Visitor_Type>
Return_Type visit_node(ast::Node* node, Visitor_Type* visitor);

template <typename Return_Type, typename Visitor_Type>
Return_Type visit_node(const ast::Node* node, Visitor_Type* visitor);

/// Expression transformations.

// converts expression `root` to Disjunctive-Normal-Form.
ast::Expr* convert_to_dnf(ast::Root& tree, ast::Expr* root);

// inline predicates into the case preconditions.
void inline_predicates(ast::Root& tree);

// converts all preconditions to DNF.
void convert_to_dnf(ast::Root& tree);

// build various look-ups and traversal info.
void annotate(ast::Root& tree);

// figure out types of parameters and variables.
bool infer_types(ast::Root& tree);

// true is `var` is wasn't defined (first occurence in scope).
bool is_bound(ast::Var* var);
// true if all arguments are bound.
bool all_bound(ast::Func* node);
// true if all arguments are unbound.
bool all_unbound(ast::Func* node);

// gets token type name as a string to aid debugging.
const char* get_type_name(ast::Node_Type token_type);
// writes formatted Abstract-Syntax-Tree to `output`.
void        debug_output_ast(const ast::Root& root, Writer* output);

#define PLNNRC_NODE(TAG, TYPE)                          \
    bool            is_##TAG(ast::Node_Type type);      \
    bool            is_##TAG(const ast::Node* node);    \
    TYPE*           as_##TAG(ast::Node* node);          \
    const TYPE*     as_##TAG(const ast::Node* node);    \

    #include "derplanner/compiler/ast_tags.inl"
#undef PLNNRC_NODE

#define PLNNRC_NODE_GROUP(GROUP_TAG, FIRST_NODE_TAG, LAST_NODE_TAG) \
    bool is_##GROUP_TAG(ast::Node_Type type);                       \
    bool is_##GROUP_TAG(const ast::Node* node);                     \

    #include "derplanner/compiler/ast_tags.inl"
#undef PLNNRC_NODE_GROUP

}

/// Inline Code.

#define PLNNRC_NODE(TAG, TYPE)                                                                                                                      \
    inline bool plnnrc::is_##TAG(ast::Node_Type type) { return type == plnnrc::ast::Node_##TAG; }                                                   \
    inline bool plnnrc::is_##TAG(const ast::Node* node) { return plnnrc::is_##TAG(node->type); }                                                    \
    inline TYPE* plnnrc::as_##TAG(ast::Node* node) { return (node && plnnrc::is_##TAG(node)) ? static_cast<TYPE*>(node) : 0; }                      \
    inline const TYPE* plnnrc::as_##TAG(const ast::Node* node) { return (node && plnnrc::is_##TAG(node)) ? static_cast<const TYPE*>(node) : 0; }    \

    #include "derplanner/compiler/ast_tags.inl"
#undef PLNNRC_NODE

#define PLNNRC_NODE_GROUP(GROUP_TAG, FIRST_NODE_TAG, LAST_NODE_TAG)                                                                                             \
    inline bool plnnrc::is_##GROUP_TAG(ast::Node_Type type) { return type >= plnnrc::ast::Node_##FIRST_NODE_TAG && type <= plnnrc::ast::Node_##LAST_NODE_TAG; } \
    inline bool plnnrc::is_##GROUP_TAG(const ast::Node* node) { return is_##GROUP_TAG(node->type); }                                                            \

    #include "derplanner/compiler/ast_tags.inl"
#endif

template <typename Return_Type, typename Visitor_Type>
inline Return_Type plnnrc::visit_node(plnnrc::ast::Node* node, Visitor_Type* visitor)
{
    switch (node->type)
    {
    #define PLNNRC_NODE(TAG, TYPE) case plnnrc::ast::Node_##TAG: return visitor->visit(static_cast<TYPE*>(node));
    #include "derplanner/compiler/ast_tags.inl"
    #undef PLNNRC_NODE
    default:
        plnnrc_assert(false);
        return Return_Type();
    }
}

template <typename Return_Type, typename Visitor_Type>
inline Return_Type plnnrc::visit_node(const plnnrc::ast::Node* node, Visitor_Type* visitor)
{
    switch (node->type)
    {
    #define PLNNRC_NODE(TAG, TYPE) case plnnrc::ast::Node_##TAG: return visitor->visit(static_cast<const TYPE*>(node));
    #include "derplanner/compiler/ast_tags.inl"
    #undef PLNNRC_NODE
    default:
        plnnrc_assert(false);
        return Return_Type();
    }
}

inline bool plnnrc::is_bound(plnnrc::ast::Var* var)
{
    return var->definition != 0;
}

inline bool plnnrc::all_bound(plnnrc::ast::Func* node)
{
    for (ast::Expr* n = node->child; n != 0; n = preorder_next(node, n))
    {
        if (ast::Var* var = as_Var(n))
        {
            if (!is_bound(var))
            {
                return false;
            }
        }
    }

    return true;
}

inline bool plnnrc::all_unbound(plnnrc::ast::Func* node)
{
    for (ast::Expr* n = node->child; n != 0; n = preorder_next(node, n))
    {
        if (ast::Var* var = as_Var(n))
        {
            if (is_bound(var))
            {
                return false;
            }
        }
    }

    return true;
}
