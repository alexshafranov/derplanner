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

#include <ctype.h>

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/io.h"

using namespace plnnrc;

static const char* s_format_str[] =
{
    "Unknown",
    #define PLNNRC_ERROR(TAG, FORMAT_STR) FORMAT_STR,
    #include "derplanner/compiler/error_tags.inl"
    #undef PLNNRC_ERROR
    "Count",
};

const char* plnnrc::get_format_string(plnnrc::Error_Type error_type)
{
    return s_format_str[error_type];
}

void plnnrc::format_error(const plnnrc::Error& error, plnnrc::Formatter& fmtr)
{
    write(fmtr, "error (%d, %d): ", error.loc.line, error.loc.column);

    const char* format = get_format_string(error.type);
    while (*format)
    {
        if (*format == '$')
        {
            ++format;
            char digit = *format;
            plnnrc_assert(isdigit(digit));
            uint32_t slot = digit - '0';
            plnnrc_assert(slot < error.num_args);
            Error::Arg_Type arg_type = error.arg_types[slot];
            plnnrc_assert(arg_type != Error::Arg_Type_None);

            switch (arg_type)
            {
            case Error::Arg_Type_Token_Value:
                {
                    write(fmtr, "%n", error.args[slot].token_value);
                    break;
                }
            case Error::Arg_Type_Token_Type:
                {
                    write(fmtr, "%s", get_type_name(error.args[slot].token_type));
                    break;
                }
            case Error::Arg_Type_Token_Group:
                {
                    write(fmtr, "%s", get_group_name(error.args[slot].token_group));
                    break;
                }
            default:
                plnnrc_assert(false);
                break;
            }
        }
        else
        {
            put_char(fmtr, *format);
        }

        ++format;
    }

    newline(fmtr);
}
