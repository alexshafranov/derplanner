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
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/io.h"

using namespace plnnrc;

plnnrc::Formatter::Formatter()
{
    memset(this, 0, sizeof(Formatter));
}

plnnrc::Formatter::~Formatter()
{
    if (output)
    {
        plnnrc::flush(*this);
        output->flush();
    }
}

void plnnrc::init(Formatter& formatter, const char* tab, const char* newline, Writer* output)
{
    formatter.indent = 0;
    formatter.tab = tab;
    formatter.newline = newline;
    formatter.buffer_ptr = formatter.buffer;
    formatter.buffer_end = formatter.buffer + Formatter::Output_Buffer_Size;
    formatter.output = output;
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

namespace io
{
    static void put_char(Formatter& formatter, char c)
    {
        if (formatter.buffer_ptr + 1 > formatter.buffer_end)
        {
            plnnrc::flush(formatter);
        }

        *(formatter.buffer_ptr++) = c;
    }

    static void put_char(Array<char>& buffer, char c)
    {
        plnnrc::push_back(buffer, c);
    }

    static void put_str(Formatter& formatter, const char* str, uint32_t length)
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

    static void put_str(Array<char>& buffer, const char* str, uint32_t length)
    {
        plnnrc::push_back(buffer, str, length);
    }

    template <typename T>
    static void put_str(T& output, const char* str)
    {
        uint32_t length = (uint32_t)strlen(str);
        io::put_str(output, str, length);
    }

    template <typename T>
    static void put_uint(T& output, uint32_t n)
    {
        const char* digits = "0123456789";
        uint32_t t = n;

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
            uint32_t p = 1;

            for (int j = 1; j < num_digits-i; ++j)
            {
                p *= 10;
            }

            io::put_char(output, digits[(n / p) % 10]);
        }
    }

    template <typename T>
    static void put_int(T& output, int32_t n)
    {
        if (n < 0)
        {
            io::put_char(output, '-');
            n = -n;
        }

        put_uint(output, (uint32_t)(n));
    }

    template <typename T>
    static void put_token(T& output, const Token_Value& token)
    {
        io::put_str(output, token.str, token.length);
    }

    template <typename T>
    static void put_indent(T& output, const char* tab, uint32_t level)
    {
        for (uint32_t i = 0; i < level; ++i)
        {
            put_str(output, tab);
        }
    }

    template <typename T>
    static void write(T& output, const char* tab, uint32_t indent, const char* format, va_list arglist)
    {
        while (*format)
        {
            if (*format == '%')
            {
                switch (*++format)
                {
                // int32_t
                case 'd':
                    {
                        int32_t n = va_arg(arglist, int32_t);
                        io::put_int(output, n);
                    }
                    break;
                // uint32_t
                case 'u':
                    {
                        uint32_t n = va_arg(arglist, uint32_t);
                        io::put_uint(output, n);
                    }
                    break;
                // string
                case 's':
                    {
                        const char* s = va_arg(arglist, const char*);
                        io::put_str(output, s);
                    }
                    break;
                // Token_Value
                case 'n':
                    {
                        Token_Value token_value = va_arg(arglist, Token_Value);
                        io::put_token(output, token_value);
                    }
                    break;
                case 'i':
                    {
                        io::put_indent(output, tab, indent);
                    }
                    break;
                // %% -> %
                case '%':
                    {
                        io::put_char(output, *format);
                    }
                    break;
                }
            }
            else
            {
                io::put_char(output, *format);
            }

            ++format;
        }
    }
}

void plnnrc::indent(Formatter& formatter)
{
    io::put_indent(formatter, formatter.tab, formatter.indent);
}

void plnnrc::newline(Formatter& formatter)
{
    io::put_str(formatter, formatter.newline);
}

void plnnrc::write(Formatter& formatter, const char* format, va_list args)
{
    io::write(formatter, formatter.tab, formatter.indent, format, args);
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
    io::put_indent(formatter, formatter.tab, formatter.indent);
    va_list arglist;
    va_start(arglist, format);
    plnnrc::write(formatter, format, arglist);
    va_end(arglist);
    plnnrc::newline(formatter);
}

void plnnrc::write(Array<char>& buffer, const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);
    io::write(buffer, "", 0, format, arglist);
    va_end(arglist);
}

void plnnrc::put_char(Formatter& formatter, char c)
{
    io::put_char(formatter, c);
}
