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

#include <stdarg.h>
#include <string.h>
#include "derplanner/compiler/memory.h"
#include "formatter.h"

namespace plnnrc {

formatter::formatter(writer& output)
    : _output(output)
    , _buffer(0)
    , _buffer_top(0)
    , _buffer_end(0)
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

void formatter::write(const char* format, ...)
{
    va_list arglist;
    va_start(arglist, format);

    while (*format)
    {
        if (*format == '%')
        {
            switch (*++format)
            {
            case 'd':
                {
                    int n = va_arg(arglist, int);
                    _puti(n);
                }
                break;
            case 's':
                {
                    const char* s = va_arg(arglist, const char*);
                    _puts(s);
                }
                break;
            }
        }
        else
        {
            _putc(*format);
        }

        ++format;
    }

    va_end(arglist);
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
}

void formatter::_puti(int n)
{
    _puts("239");
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

}
