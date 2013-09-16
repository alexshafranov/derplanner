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
#include <stdarg.h>
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

formatter::formatter(writer& output, const char* tab, const char* newline)
    : _output(output)
    , _buffer(0)
    , _buffer_top(0)
    , _buffer_end(0)
    , _indent_level(0)
    , _tab(tab)
    , _newline(newline)
{
}

formatter::~formatter()
{
    flush();
    memory::deallocate(_buffer);
}

bool formatter::init(size_t buffer_size)
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

void formatter::writeln(const char* format, ...)
{
    _put_indent();

    va_list arglist;
    va_start(arglist, format);

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
                    _puti(n);
                }
                break;
            // string
            case 's':
                {
                    const char* s = va_arg(arglist, const char*);
                    _puts(s);
                }
                break;
            // identifier
            case 'i':
                {
                    const char* s = va_arg(arglist, const char*);

                    if (id_needs_conversion(s))
                    {
                        _putid(s);
                    }
                    else
                    {
                        _puts(s);
                    }
                }
            }
        }
        else
        {
            _putc(*format);
        }

        ++format;
    }

    va_end(arglist);

    newline();
}

void formatter::newline()
{
    _puts(_newline);
}

void formatter::_putc(char c)
{
    if (_buffer_top >= _buffer_end)
    {
        flush();
    }

    *(_buffer_top++) = c;
}

void formatter::_puts(const char* s)
{
    size_t length = strlen(s);

    if (_buffer_top + length >= _buffer_end)
    {
        flush();

        if (_buffer_top + length >= _buffer_end)
        {
            _output.write(s, length);
            return;
        }
    }

    memcpy(_buffer_top, s, length);
    _buffer_top += length;
}

void formatter::_puti(int n)
{
    const char* digits = "0123456789";
    int t = n;

    if (n < 0)
    {
        _putc('-');
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

        _putc(digits[(n / p) % 10]);
    }
}

void formatter::_putid(const char* symbol)
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
        _putc('_');
    }

    for (; *c != 0; ++c)
    {
        if (*c == '?' || *c == '!')
        {
            continue;
        }

        if (isalnum(*c) || *c == '_')
        {
            _putc(*c);
            continue;
        }

        if (*c == '-')
        {
            _putc('_');
            continue;
        }
    }
}

void formatter::flush()
{
    size_t count = _buffer_top - _buffer;

    if (count > 0)
    {
        _output.write(_buffer, count);
        _buffer_top = _buffer;
    }
}

void formatter::_put_indent()
{
    for (int i = 0; i < _indent_level; ++i)
    {
        _puts("\t");
    }
}

scope::scope(formatter& output, bool end_with_empty_line)
    : output(output)
    , end_with_empty_line(end_with_empty_line)
{
    output._put_indent();
    output._indent_level++;
    output._putc('{');
    output._puts(output._newline);
}

scope::~scope()
{
    output._indent_level--;
    output._put_indent();
    output._putc('}');
    output._puts(output._newline);

    if (end_with_empty_line)
    {
        output._puts(output._newline);
    }
}

class_scope::class_scope(formatter& output)
    : output(output)
{
    output._put_indent();
    output._indent_level++;
    output._putc('{');
    output._puts(output._newline);
}

class_scope::~class_scope()
{
    output._indent_level--;
    output._put_indent();
    output._puts("};");
    output._puts(output._newline);
    output._puts(output._newline);
}

}
