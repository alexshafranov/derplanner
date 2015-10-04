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

#include "derplanner/compiler/types.h"

namespace plnnrc {

// create AST.
void init(ast::Root* self, Array<Error>* errors, Memory_Stack* mem_pool, Memory_Stack* mem_scratch);

/// `create_*` functions for `ast` node types.

ast::World*         create_world(const ast::Root* tree);
ast::Primitive*     create_primitive(const ast::Root* tree);
ast::Attribute*     create_attribute(const ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Macro*         create_macro(const ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Fact*          create_fact(const ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Param*         create_param(const ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Domain*        create_domain(const ast::Root* tree, const Token_Value& name);
ast::Task*          create_task(const ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Case*          create_case(const ast::Root* tree);
ast::Data_Type*     create_type(const ast::Root* tree, Token_Type data_type);

ast::Op*            create_op(const ast::Root* tree, ast::Node_Type operation_type);
ast::Op*            create_op(const ast::Root* tree, ast::Node_Type operation_type, const Location& loc);
ast::Var*           create_var(const ast::Root* tree, const Token_Value& name, const Location& loc);
ast::Literal*       create_literal(const ast::Root* tree, const Token& token, const Location& loc);
ast::Func*          create_func(const ast::Root* tree, const Token_Value& name, const Location& loc);


// lookup `ast::Task` node by name.
ast::Task*      get_task(const ast::Root* self, const Token_Value& name);
// lookup `ast::Fact` node by name.
ast::Fact*      get_fact(const ast::Root* self, const Token_Value& name);
// lookup `ast::Fact` node for primitive task by name.
ast::Fact*      get_primitive(const ast::Root* self, const Token_Value& name);

/// ast::Expr

// make node `child` the last child of node `parent`.
void        append_child(ast::Expr* parent, ast::Expr* child);
// make `child` the next sibling of `after`.
void        insert_child(ast::Expr* after, ast::Expr* child);
// unparent `node` from it's current parent.
void        unparent(ast::Expr* node);
// returns the next node in pre-order (visit node then visit it's children) traversal.
ast::Expr*  preorder_next(const ast::Expr* root, const ast::Expr* current);

// calls a proper `visit` method overload depending on the node type.
template <typename Return_Type, typename Visitor_Type>
Return_Type visit_node(ast::Node* node, Visitor_Type* visitor);

template <typename Return_Type, typename Visitor_Type>
Return_Type visit_node(const ast::Node* node, Visitor_Type* visitor);

// finds the first attribute of a given type.
ast::Attribute* find_attribute(const ast::Node* node, Attribute_Type type);

/// Attributes.

// returns the number of arguments allowed for an attribute of a given `type`.
uint32_t                        get_num_args(Attribute_Type type);
// returns an array of argument classes allowed for an attribute of a given `type`.
const Attribute_Arg_Class*      get_arg_classes(Attribute_Type type);

/// Expression transformations.

// converts expression `root` to Disjunctive-Normal-Form.
ast::Expr*  convert_to_dnf(const ast::Root* tree, ast::Expr* root);

// inline macros into the case preconditions.
void        inline_macros(ast::Root* tree);

// converts all preconditions to DNF.
void        convert_to_dnf(const ast::Root* tree);

// build various look-ups and traversal info.
void        annotate(ast::Root* tree);

// figure out types of parameters and variables.
bool        infer_types(const ast::Root* tree);

// convert literal token value to integer.
int64_t as_int(const ast::Literal* node);
// convert literal token value to float.
float   as_float(const ast::Literal* node);

// returns the type which is unification (i.e. the common type to cast to) of 'a' and 'b'.
Token_Type unify(Token_Type a, Token_Type b);

// gets token type name as a string to help debugging.
const char*     get_type_name(ast::Node_Type token_type);
// writes formatted Abstract-Syntax-Tree to `output`.
void            debug_output_ast(const ast::Root* root, Writer* output);

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

#define PLNNRC_ATTRIBUTE(TAG, STR)          \
    bool is_##TAG(Attribute_Type type);     \
    bool is_##TAG(ast::Attribute* node);    \

    #include "derplanner/compiler/attribute_tags.inl"
#undef PLNNRC_ATTRIBUTE

}

#include "derplanner/compiler/ast.inl"

#endif
