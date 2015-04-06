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

#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/io.h"

using namespace plnnrc;

plnnrc::Formatter::Formatter()
{
    memset(this, 0, sizeof(Formatter));
}

plnnrc::Formatter::~Formatter()
{
    if (buffer)
    {
        plnnrc::flush(*this);
        plnnrc::destroy(*this);
    }
}

void plnnrc::init(Formatter& formatter, const char* tab, const char* newline, uint32_t buffer_size, Writer* output)
{
    formatter.indent = 0;
    formatter.tab = tab;
    formatter.newline = newline;
    formatter.buffer = static_cast<uint8_t*>(plnnrc::allocate(buffer_size));
    formatter.buffer_ptr = formatter.buffer;
    formatter.buffer_end = formatter.buffer + buffer_size;
    formatter.output = output;
}

void plnnrc::destroy(Formatter& formatter)
{
    plnnrc::deallocate(formatter.buffer);
    memset(&formatter, 0, sizeof(Formatter));
}

void plnnrc::flush(Formatter& formatter)
{
    size_t count = formatter.buffer_ptr - formatter.buffer;
    if (count > 0)
    {
        formatter.output->write(formatter.buffer, count);
        formatter.buffer_ptr = formatter.buffer;
    }
}

void plnnrc::indent(Formatter& formatter)
{
    plnnrc::put_indent(formatter, formatter.indent);
}

void plnnrc::newline(Formatter& formatter)
{
    plnnrc::put_str(formatter, formatter.newline);
}

void plnnrc::write(Formatter& formatter, const char* format, va_list arglist)
{
    while (*format)
    {
        if (*format == '%')
        {
            switch (*++format)
            {
            // decimal number
            case 'd':
                {
                    int n = va_arg(arglist, int);
                    plnnrc::put_int(formatter, n);
                }
                break;
            // string
            case 's':
                {
                    const char* s = va_arg(arglist, const char*);
                    plnnrc::put_str(formatter, s);
                }
                break;
            // Token_Value
            case 'i':
                {
                    Token_Value token_value = va_arg(arglist, Token_Value);
                    plnnrc::put_token(formatter, token_value);
                }
                break;
            // %% -> %
            case '%':
                {
                    plnnrc::put_char(formatter, *format);
                }
                break;
            }
        }
        else
        {
            plnnrc::put_char(formatter, *format);
        }

        ++format;
    }
}

void plnnrc::write(Formatter& formatter, const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    plnnrc::write(formatter, format, arglist);
    va_end(arglist);
}

void plnnrc::writeln(Formatter& formatter, const char* format, ...)
{
    plnnrc::put_indent(formatter, formatter.indent);
    va_list arglist;
    va_start(arglist, format);
    plnnrc::write(formatter, format, arglist);
    va_end(arglist);
    plnnrc::newline(formatter);
}

void plnnrc::put_char(Formatter& formatter, char c)
{
    if (formatter.buffer_ptr + 1 > formatter.buffer_end)
    {
        plnnrc::flush(formatter);
    }

    *(formatter.buffer_ptr++) = c;
}

void plnnrc::put_str(Formatter& formatter, const char* str)
{
    size_t length = strlen(str);
    plnnrc::put_str(formatter, str, length);
}

void plnnrc::put_str(Formatter& formatter, const char* str, size_t length)
{
    if (formatter.buffer_ptr + length > formatter.buffer_end)
    {
        plnnrc::flush(formatter);

        if (formatter.buffer_ptr + length > formatter.buffer_end)
        {
            formatter.output->write(str, length);
            return;
        }
    }

    memcpy(formatter.buffer_ptr, str, length);
    formatter.buffer_ptr += length;
}

void plnnrc::put_token(Formatter& formatter, const Token_Value& token_value)
{
    plnnrc::put_str(formatter, token_value.str, token_value.length);
}

void plnnrc::put_int(Formatter& formatter, int n)
{
    const char* digits = "0123456789";
    int t = n;

    if (n < 0)
    {
        plnnrc::put_char(formatter, '-');
        t = -n;
    }

    int num_digits = 0;

    if (t == 0)
    {
        num_digits = 1;
    }

    while (t)
    {
        num_digits++;
        t /= 10;
    }

    for (int i = 0; i < num_digits; ++i)
    {
        int p = 1;

        for (int j = 1; j < num_digits-i; ++j)
        {
            p *= 10;
        }

        plnnrc::put_char(formatter, digits[(n / p) % 10]);
    }
}

void plnnrc::put_indent(Formatter& formatter, uint32_t level)
{
    for (uint32_t i = 0; i < level; ++i)
    {
        plnnrc::put_str(formatter, formatter.tab);
    }
}
