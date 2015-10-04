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

#include "derplanner/compiler/array.h"
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

static const char* get_nice_description(Token_Type type)
{
    switch (type)
    {
    #define PLNNRC_KEYWORD_TOKEN(TAG, STR)      \
        case Token_##TAG:                       \
            return "keyword '" STR "'";         \

    #define PLNNRC_TYPE_KEYWORD_TOKEN(TAG, STR) \
        case Token_##TAG:                       \
            return "type '" STR "'";            \

    #define PLNNRC_PUNCTUATOR_TOKEN(TAG, STR)   \
        case Token_##TAG:                       \
            return "'" STR "'";                 \

    #define PLNNRC_OPERATOR_TOKEN(TAG, STR)     \
        case Token_##TAG:                       \
            return "operator '" STR "'";        \

    #include "derplanner/compiler/token_tags.inl"
        case Token_Eos:
            return "end-of-stream";

        default:
            return 0;
    };

    #undef PLNNRC_OPERATOR_TOKEN
    #undef PLNNRC_PUNCTUATOR_TOKEN
    #undef PLNNRC_TYPE_KEYWORD_TOKEN
    #undef PLNNRC_KEYWORD_TOKEN
}

static const char* get_description(Token_Type type)
{
    const char* desc = get_nice_description(type);
    if (desc)
    {
        return desc;
    }

    return get_type_name(type);
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
            case Error::Arg_Type_Token:
                {
                    Token tok = error.args[slot].token;
                    const char* desc = get_nice_description(tok.type);

                    if (desc)
                    {
                        write(fmtr, "%s", desc);
                    }
                    else
                    {
                        write(fmtr, "'%n'", tok.value);
                    }

                    break;
                }
            case Error::Arg_Type_Token_Value:
                {
                    Token_Value value = error.args[slot].token_value;
                    write(fmtr, "'%n'", value);
                    break;
                }
            case Error::Arg_Type_Token_Type:
                {
                    write(fmtr, "%s", get_description(error.args[slot].token_type));
                    break;
                }
            case Error::Arg_Type_Token_Group:
                {
                    write(fmtr, "%s", get_group_name(error.args[slot].token_group));
                    break;
                }
            case Error::Arg_Type_Func_Call_Signature:
                {
                    const ast::Func* func = error.args[slot].func_call;
                    write(fmtr, "%n(", func->name);
                    for (uint32_t i = 0; i < size(func->arg_types); ++i)
                    {
                        const Token_Type type = func->arg_types[i];
                        write(fmtr, "%s", get_description(type));
                        if (i < size(func->arg_types) - 1)
                            write(fmtr, ", ");
                    }
                    write(fmtr, ")");
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
