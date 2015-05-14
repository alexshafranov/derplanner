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

#ifndef DERPLANNER_COMPILER_ERRORS_H_
#define DERPLANNER_COMPILER_ERRORS_H_

#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/types.h"

namespace plnnrc {

// returns a format string for the given error type.
const char* get_format_string(Error_Type error_type);

void init(Error& builder, Error_Type type, Location loc);

Error& operator<<(Error& builder, const Token_Value& token_value);
Error& operator<<(Error& builder, const Token_Type&  token_type);
Error& operator<<(Error& builder, const Token_Group& token_group);

}

/// Inline

inline void plnnrc::init(plnnrc::Error& builder, plnnrc::Error_Type type, plnnrc::Location loc)
{
    memset(&builder, 0, sizeof(builder));
    builder.type = type;
    builder.loc = loc;
    builder.format = plnnrc::get_format_string(type);
}

inline plnnrc::Error& plnnrc::operator<<(plnnrc::Error& builder, const plnnrc::Token_Value& token_value)
{
    plnnrc_assert(builder.num_args < plnnrc::Error::Max_Args);
    builder.args[builder.num_args].token_value = token_value;
    builder.arg_types[builder.num_args] = plnnrc::Error::Arg_Type_Token_Value;
    ++builder.num_args;
    return builder;
}

inline plnnrc::Error& plnnrc::operator<<(plnnrc::Error& builder, const plnnrc::Token_Type&  token_type)
{
    plnnrc_assert(builder.num_args < plnnrc::Error::Max_Args);
    builder.args[builder.num_args].token_type = token_type;
    builder.arg_types[builder.num_args] = plnnrc::Error::Arg_Type_Token_Type;
    ++builder.num_args;
    return builder;
}

inline plnnrc::Error& plnnrc::operator<<(plnnrc::Error& builder, const plnnrc::Token_Group& token_group)
{
    plnnrc_assert(builder.num_args < plnnrc::Error::Max_Args);
    builder.args[builder.num_args].token_group = token_group;
    builder.arg_types[builder.num_args] = plnnrc::Error::Arg_Type_Token_Group;
    ++builder.num_args;
    return builder;
}

#endif
