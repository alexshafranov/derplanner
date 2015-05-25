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

#ifndef PLNNRC_KEYWORD_TOKEN
    #define PLNNRC_KEYWORD_TOKEN(TAG, STR) PLNNRC_TOKEN(TAG)
#endif

#ifndef PLNNRC_TYPE_KEYWORD_TOKEN
    #define PLNNRC_TYPE_KEYWORD_TOKEN(TAG, STR) PLNNRC_KEYWORD_TOKEN(TAG, STR)
#endif

#ifndef PLNNRC_PUNCTUATOR_TOKEN
    #define PLNNRC_PUNCTUATOR_TOKEN(TAG, STR) PLNNRC_TOKEN(TAG)
#endif

#ifndef PLNNRC_OPERATOR_TOKEN
    #define PLNNRC_OPERATOR_TOKEN(TAG, STR) PLNNRC_TOKEN(TAG)
#endif

#ifndef PLNNRC_TOKEN_GROUP
    #define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG)
#endif

PLNNRC_TOKEN(Eos)
PLNNRC_TOKEN(Id)
PLNNRC_TOKEN(Literal_Integer)
PLNNRC_TOKEN(Literal_Float)

PLNNRC_KEYWORD_TOKEN(Domain,            "domain")
PLNNRC_KEYWORD_TOKEN(World,             "world")
PLNNRC_KEYWORD_TOKEN(Primitive,         "primitive")
PLNNRC_KEYWORD_TOKEN(Predicate,         "predicate")
PLNNRC_KEYWORD_TOKEN(Task,              "task")
PLNNRC_KEYWORD_TOKEN(Case,              "case")
PLNNRC_KEYWORD_TOKEN(Const,             "const")

PLNNRC_TYPE_KEYWORD_TOKEN(Int32,        "int32")
PLNNRC_TYPE_KEYWORD_TOKEN(UInt32,       "uint32")
PLNNRC_TYPE_KEYWORD_TOKEN(Int64,        "int64")
PLNNRC_TYPE_KEYWORD_TOKEN(UInt64,       "uint64")
PLNNRC_TYPE_KEYWORD_TOKEN(Float,        "float")
PLNNRC_TYPE_KEYWORD_TOKEN(Vec3,         "vec3")

PLNNRC_PUNCTUATOR_TOKEN(L_Curly,        "{")
PLNNRC_PUNCTUATOR_TOKEN(R_Curly,        "}")
PLNNRC_PUNCTUATOR_TOKEN(L_Paren,        "(")
PLNNRC_PUNCTUATOR_TOKEN(R_Paren,        ")")
PLNNRC_PUNCTUATOR_TOKEN(L_Square,       "[")
PLNNRC_PUNCTUATOR_TOKEN(R_Square,       "]")
PLNNRC_PUNCTUATOR_TOKEN(Comma,          ",")
PLNNRC_PUNCTUATOR_TOKEN(Colon,          ":")
PLNNRC_PUNCTUATOR_TOKEN(Arrow,          "->")
PLNNRC_PUNCTUATOR_TOKEN(Equality,       "=")

// binary
PLNNRC_OPERATOR_TOKEN(Or,               "|")
PLNNRC_OPERATOR_TOKEN(And,              "&")
PLNNRC_OPERATOR_TOKEN(Equal,            "==")
PLNNRC_OPERATOR_TOKEN(NotEqual,         "!=")
PLNNRC_OPERATOR_TOKEN(Less,             "<")
PLNNRC_OPERATOR_TOKEN(LessEqual,        "<=")
PLNNRC_OPERATOR_TOKEN(Greater,          ">")
PLNNRC_OPERATOR_TOKEN(GreaterEqual,     ">=")
PLNNRC_OPERATOR_TOKEN(Plus,             "+")
PLNNRC_OPERATOR_TOKEN(Minus,            "-")
PLNNRC_OPERATOR_TOKEN(Mul,              "*")
PLNNRC_OPERATOR_TOKEN(Div,              "/")

// unary
PLNNRC_OPERATOR_TOKEN(Dot,              ".")
PLNNRC_OPERATOR_TOKEN(Not,              "~")

PLNNRC_TOKEN_GROUP(Keyword, Domain, Vec3)
PLNNRC_TOKEN_GROUP(Block,   Domain, Task)
PLNNRC_TOKEN_GROUP(Type,    Int32,  Vec3)
PLNNRC_TOKEN_GROUP(Literal, Literal_Integer, Literal_Float)
PLNNRC_TOKEN_GROUP(Binary,  Or, Div)

#undef PLNNRC_TOKEN_GROUP
#undef PLNNRC_OPERATOR_TOKEN
#undef PLNNRC_PUNCTUATOR_TOKEN
#undef PLNNRC_TYPE_KEYWORD_TOKEN
#undef PLNNRC_KEYWORD_TOKEN
#undef PLNNRC_TOKEN
