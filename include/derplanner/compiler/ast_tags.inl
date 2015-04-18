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

#ifndef PLNNRC_NODE_GROUP
    #define PLNNRC_NODE_GROUP(GROUP_TAG, FIRST_NODE_TAG, LAST_NODE_TAG)
#endif

PLNNRC_NODE(World,      ::plnnrc::ast::World)
PLNNRC_NODE(Fact,       ::plnnrc::ast::Fact)
PLNNRC_NODE(Param,      ::plnnrc::ast::Param)
PLNNRC_NODE(Domain,     ::plnnrc::ast::Domain)
PLNNRC_NODE(Task,       ::plnnrc::ast::Task)
PLNNRC_NODE(Case,       ::plnnrc::ast::Case)
PLNNRC_NODE(Func,       ::plnnrc::ast::Func)
PLNNRC_NODE(Var,        ::plnnrc::ast::Var)
PLNNRC_NODE(Data_Type,  ::plnnrc::ast::Data_Type)
PLNNRC_NODE(Literal,    ::plnnrc::ast::Literal)

PLNNRC_NODE(And,        ::plnnrc::ast::Op)
PLNNRC_NODE(Or,         ::plnnrc::ast::Op)
PLNNRC_NODE(Not,        ::plnnrc::ast::Op)
PLNNRC_NODE(Plus,       ::plnnrc::ast::Op)
PLNNRC_NODE(Minus,      ::plnnrc::ast::Op)

PLNNRC_NODE_GROUP(Logical, And, Not)

#undef PLNNRC_NODE
#undef PLNNRC_NODE_GROUP
