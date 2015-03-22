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

#ifndef PLNNRC_TOKEN
    #define PLNNRC_TOKEN(TAG)
#endif

#ifndef PLNNRC_KEYWORD
    #define PLNNRC_KEYWORD(TAG, STR) PLNNRC_TOKEN(TAG)
#endif

#ifndef PLNNRC_GROUP
    #define PLNNRC_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG)
#endif

#ifndef PLNNRC_PUNCTUATOR
    #define PLNNRC_PUNCTUATOR(TAG, STR) PLNNRC_TOKEN(TAG)
#endif

#ifndef PLNNRC_OPERATOR
    #define PLNNRC_OPERATOR(TAG, STR) PLNNRC_TOKEN(TAG)
#endif

PLNNRC_TOKEN(Identifier)
PLNNRC_TOKEN(Literal_Integer)
PLNNRC_TOKEN(Literal_Float)

PLNNRC_KEYWORD(Domain,          "domain")
PLNNRC_KEYWORD(World,           "world")
PLNNRC_KEYWORD(Task,            "task")
PLNNRC_KEYWORD(Case,            "case")

PLNNRC_KEYWORD(Int32,           "int32")
PLNNRC_KEYWORD(Float,           "float")

PLNNRC_PUNCTUATOR(L_Curly,      "{")
PLNNRC_PUNCTUATOR(R_Curly,      "}")
PLNNRC_PUNCTUATOR(L_Paren,      "(")
PLNNRC_PUNCTUATOR(R_Paren,      ")")
PLNNRC_PUNCTUATOR(L_Square,     "[")
PLNNRC_PUNCTUATOR(R_Square,     "]")
PLNNRC_PUNCTUATOR(Arrow,        "->")

PLNNRC_OPERATOR(And,            "&")
PLNNRC_OPERATOR(Or,             "|")
PLNNRC_OPERATOR(Not,            "~")

PLNNRC_GROUP(Keyword,   Domain, Float)
PLNNRC_GROUP(Type,      Int32,  Float)
PLNNRC_GROUP(Logical,   And,    Not)
