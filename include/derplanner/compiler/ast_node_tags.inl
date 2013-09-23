//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
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


#ifndef PLNNRC_AST_NODE_INNER
    #define PLNNRC_AST_NODE_INNER(NODE_ID) PLNNRC_AST_NODE(NODE_ID)
#endif

#ifndef PLNNRC_AST_NODE_LEAF
    #define PLNNRC_AST_NODE_LEAF(NODE_ID) PLNNRC_AST_NODE(NODE_ID)
#endif

PLNNRC_AST_NODE_INNER(node_worldstate)
PLNNRC_AST_NODE_INNER(node_domain)
PLNNRC_AST_NODE_INNER(node_method)
PLNNRC_AST_NODE_INNER(node_branch)
PLNNRC_AST_NODE_INNER(node_operator)
PLNNRC_AST_NODE_INNER(node_atomlist)

PLNNRC_AST_NODE_INNER(node_op_and)
PLNNRC_AST_NODE_INNER(node_op_or)
PLNNRC_AST_NODE_INNER(node_op_not)

PLNNRC_AST_NODE_LEAF(node_worldstate_type)

PLNNRC_AST_NODE_LEAF(node_atom)

PLNNRC_AST_NODE_LEAF(node_term_variable)
PLNNRC_AST_NODE_LEAF(node_term_int)
PLNNRC_AST_NODE_LEAF(node_term_float)
PLNNRC_AST_NODE_LEAF(node_term_call)

PLNNRC_AST_NODE_LEAF(node_error)

#undef PLNNRC_AST_NODE_INNER
#undef PLNNRC_AST_NODE_LEAF
