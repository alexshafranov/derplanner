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

#ifndef PLNNRC_AST_NODE
    #define PLNNRC_AST_NODE(NODE_ID)
#endif

#ifndef PLNNRC_AST_NODE_WITH_TOKEN
    #define PLNNRC_AST_NODE_WITH_TOKEN(NODE_ID) PLNNRC_AST_NODE(NODE_ID)
#endif

#ifndef PLNNRC_AST_NODE_GROUP
    #define PLNNRC_AST_NODE_GROUP(GROUP_ID, FIRST_ID, LAST_ID)
#endif

PLNNRC_AST_NODE(root)
PLNNRC_AST_NODE(worldstate)
PLNNRC_AST_NODE(function)
PLNNRC_AST_NODE(domain)
PLNNRC_AST_NODE(method)
PLNNRC_AST_NODE(branch)
PLNNRC_AST_NODE(operator)
PLNNRC_AST_NODE(task_list)

PLNNRC_AST_NODE(add_list)
PLNNRC_AST_NODE(delete_list)
PLNNRC_AST_NODE_GROUP(effect_list, add_list, delete_list)

PLNNRC_AST_NODE(op_and)
PLNNRC_AST_NODE(op_or)
PLNNRC_AST_NODE(op_not)
PLNNRC_AST_NODE_GROUP(logical_op, op_and, op_not)

PLNNRC_AST_NODE(op_eq)
PLNNRC_AST_NODE(op_ne)
PLNNRC_AST_NODE(op_le)
PLNNRC_AST_NODE(op_ge)
PLNNRC_AST_NODE(op_lt)
PLNNRC_AST_NODE(op_gt)
PLNNRC_AST_NODE_GROUP(comparison_op, op_eq, op_gt)

PLNNRC_AST_NODE_WITH_TOKEN(namespace)
PLNNRC_AST_NODE_WITH_TOKEN(worldstate_type)
PLNNRC_AST_NODE_WITH_TOKEN(atom)

PLNNRC_AST_NODE_WITH_TOKEN(term_variable)
PLNNRC_AST_NODE_WITH_TOKEN(term_int)
PLNNRC_AST_NODE_WITH_TOKEN(term_float)
PLNNRC_AST_NODE_WITH_TOKEN(term_call)
PLNNRC_AST_NODE_GROUP(term, term_variable, term_call)

PLNNRC_AST_NODE_WITH_TOKEN(error)

#undef PLNNRC_AST_NODE_GROUP
#undef PLNNRC_AST_NODE_WITH_TOKEN
#undef PLNNRC_AST_NODE
