//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
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
#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "formatter.h"

namespace plnnrc {

namespace
{
    bool id_needs_conversion(const char* symbol)
    {
        for (const char* c = symbol; *c != 0; ++c)
        {
            if (*c == '-' || *c == '?' || *c == '!')
            {
                return true;
            }
        }

        return false;
    }

    int id_converted_length(const char* symbol)
    {
        const char* c;

        for (c = symbol; *c != 0; ++c)
        {
            if (*c != '?' && *c != '!')
            {
                break;
            }
        }

        if (*c == 0)
        {
            return 0;
        }

        int length = 0;

        if (isdigit(*c))
        {
            ++length;
        }

        for (; *c != 0; ++c)
        {
            if (isalnum(*c) || *c == '_' || *c == '-')
            {
                ++length;
                continue;
            }

            if (*c == '!' || *c == '?')
            {
                continue;
            }

            return -1;
        }

        return length;
    }
}

bool is_valid_id(const char* symbol)
{
    return id_converted_length(symbol) > 0;
}

Formatter::Formatter(Writer& output, const char* tab, const char* newline)
    : _output(output)
    , _buffer(0)
    , _buffer_top(0)
    , _buffer_end(0)
    , _indent_level(0)
    , _tab(tab)
    , _newline(newline)
{
}

Formatter::~Formatter()
{
    flush();
    memory::deallocate(_buffer);
}

bool Formatter::init(size_t buffer_size)
{
    void* buffer = memory::allocate(buffer_size);

    if (!buffer)
    {
        return false;
    }

    memory::deallocate(_buffer);

    _buffer = static_cast<char*>(buffer);
    _buffer_top = _buffer;
    _buffer_end = _buffer + buffer_size;

    return true;
}

void Formatter::_write(const char* format, va_list arglist)
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
                    put_int(n);
                }
                break;
            // string
            case 's':
                {
                    const char* s = va_arg(arglist, const char*);
                    put_str(s);
                }
                break;
            // identifier
            case 'i':
                {
                    const char* s = va_arg(arglist, const char*);

                    if (id_needs_conversion(s))
                    {
                        put_id(s);
                    }
                    else
                    {
                        put_str(s);
                    }
                }
                break;
            // paste functor
            case 'p':
                {
                    Paste_Func* paste = va_arg(arglist, Paste_Func*);
                    (*paste)(*this);
                }
                break;
            // %% -> %
            case '%':
                {
                    put_char(*format);
                }
                break;
            }
        }
        else
        {
            put_char(*format);
        }

        ++format;
    }
}

void Formatter::writeln(const char* format, ...)
{
    put_indent();
    va_list arglist;
    va_start(arglist, format);
    _write(format, arglist);
    va_end(arglist);
    newline();
}

void Formatter::newline()
{
    put_str(_newline);
}

void Formatter::put_char(char c)
{
    if (_buffer_top + 1 > _buffer_end)
    {
        flush();
    }

    *(_buffer_top++) = c;
}

void Formatter::put_str(const char* s)
{
    size_t length = strlen(s);

    if (_buffer_top + length > _buffer_end)
    {
        flush();

        if (_buffer_top + length > _buffer_end)
        {
            _output.write(s, length);
            return;
        }
    }

    memcpy(_buffer_top, s, length);
    _buffer_top += length;
}

void Formatter::put_int(int n)
{
    const char* digits = "0123456789";
    int t = n;

    if (n < 0)
    {
        put_char('-');
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

        put_char(digits[(n / p) % 10]);
    }
}

void Formatter::put_id(const char* symbol)
{
    plnnrc_assert(is_valid_id(symbol));

    const char* c = symbol;

    for (; *c != 0; ++c)
    {
        if (*c != '?' && *c != '!')
        {
            break;
        }
    }

    if (isdigit(*c))
    {
        put_char('_');
    }

    for (; *c != 0; ++c)
    {
        if (*c == '?' || *c == '!')
        {
            continue;
        }

        if (isalnum(*c) || *c == '_')
        {
            put_char(*c);
            continue;
        }

        if (*c == '-')
        {
            put_char('_');
            continue;
        }
    }
}

void Formatter::indent()
{
    _indent_level++;
}

void Formatter::put_indent()
{
    for (int i = 0; i < _indent_level; ++i)
    {
        put_str(_tab);
    }
}

void Formatter::dedent()
{
    _indent_level--;
}

void Formatter::flush()
{
    size_t count = _buffer_top - _buffer;

    if (count > 0)
    {
        _output.write(_buffer, count);
        _buffer_top = _buffer;
    }
}

Scope::Scope(Formatter& output, bool end_with_empty_line)
    : output(output)
    , end_with_empty_line(end_with_empty_line)
{
    output.put_indent();
    output.indent();
    output.put_char('{');
    output.newline();
}

Scope::~Scope()
{
    output.dedent();
    output.put_indent();
    output.put_char('}');
    output.newline();

    if (end_with_empty_line)
    {
        output.newline();
    }
}

Class_Scope::Class_Scope(Formatter& output)
    : output(output)
{
    output.put_indent();
    output.indent();
    output.put_char('{');
    output.newline();
}

Class_Scope::~Class_Scope()
{
    output.dedent();
    output.put_indent();
    output.put_str("};");
    output.newline();
    output.newline();
}

Indented::Indented(Formatter& output)
    : output(output)
{
    output.indent();
}

Indented::~Indented()
{
    output.dedent();
}

}
