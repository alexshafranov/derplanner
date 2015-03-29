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

#ifndef DERPLANNER_COMPILER_AST_TYPES_H_
#define DERPLANNER_COMPILER_AST_TYPES_H_

#include "derplanner/compiler/types.h"

namespace plnnrc {

struct Task;
    struct Case;

// Task parameter.
struct Parameter
{
    // parameter formal name.
    Token name;
    // infered type.
    Token type;
};

// Argument of a task instance or of a precondition fact.
struct Argument
{
    // next argument in a pack.
    Argument*   next;
    // link to parameter used as an argument (null if literal).
    Parameter*  definition;
    // token representing this argument (used for literals).
    Token       literal;
};

// Domain task definition.
struct Task
{
    // task name.
    Token       name;
    // parameter linked list.
    Parameter*  first_parameter;
    // case linked list.
    Case*       first_case;
};

}

#endif
