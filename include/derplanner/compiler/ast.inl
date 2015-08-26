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

#ifndef DERPLANNER_COMPILER_AST_INL_
#define DERPLANNER_COMPILER_AST_INL_

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
#undef PLNNRC_NODE_GROUP

#define PLNNRC_ATTRIBUTE(TAG, STR)                                                                              \
    inline bool plnnrc::is_##TAG(plnnrc::Attribute_Type type) { return type == plnnrc::Attribute_##TAG; }       \
    inline bool plnnrc::is_##TAG(plnnrc::ast::Attribute* node) { return plnnrc::is_##TAG(node->attr_type); }    \

    #include "derplanner/compiler/attribute_tags.inl"
#undef PLNNRC_ATTRIBUTE

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

#endif
