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

#ifndef PLNNRC_NODE
    #define PLNNRC_NODE(TAG, TYPE)
#endif

#ifndef PLNNRC_NODE_OP
    #define PLNNRC_NODE_OP(TAG) PLNNRC_NODE(TAG, ::plnnrc::ast::Op)
#endif

#ifndef PLNNRC_NODE_GROUP
    #define PLNNRC_NODE_GROUP(GROUP_TAG, FIRST_NODE_TAG, LAST_NODE_TAG)
#endif

PLNNRC_NODE(World,      ::plnnrc::ast::World)
PLNNRC_NODE(Primitive,  ::plnnrc::ast::Primitive)
PLNNRC_NODE(Predicate,  ::plnnrc::ast::Predicate)
PLNNRC_NODE(Fact,       ::plnnrc::ast::Fact)
PLNNRC_NODE(Param,      ::plnnrc::ast::Param)
PLNNRC_NODE(Domain,     ::plnnrc::ast::Domain)
PLNNRC_NODE(Task,       ::plnnrc::ast::Task)
PLNNRC_NODE(Case,       ::plnnrc::ast::Case)
PLNNRC_NODE(Func,       ::plnnrc::ast::Func)
PLNNRC_NODE(Var,        ::plnnrc::ast::Var)
PLNNRC_NODE(Data_Type,  ::plnnrc::ast::Data_Type)
PLNNRC_NODE(Literal,    ::plnnrc::ast::Literal)

PLNNRC_NODE_OP(Or)
PLNNRC_NODE_OP(And)
PLNNRC_NODE_OP(Not)

PLNNRC_NODE_OP(Equal)
PLNNRC_NODE_OP(NotEqual)
PLNNRC_NODE_OP(Less)
PLNNRC_NODE_OP(LessEqual)
PLNNRC_NODE_OP(Greater)
PLNNRC_NODE_OP(GreaterEqual)

PLNNRC_NODE_OP(Plus)
PLNNRC_NODE_OP(Minus)
PLNNRC_NODE_OP(Mul)
PLNNRC_NODE_OP(Div)

PLNNRC_NODE_OP(Dot)

PLNNRC_NODE_GROUP(Logical, Or, Not)
PLNNRC_NODE_GROUP(Comparison, Equal, GreaterEqual)
PLNNRC_NODE_GROUP(Arithmetic, Plus, Div)

#undef PLNNRC_NODE_OP
#undef PLNNRC_NODE
#undef PLNNRC_NODE_GROUP
