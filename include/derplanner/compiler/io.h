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

#ifndef DERPLANNER_COMPILER_IO_H_
#define DERPLANNER_COMPILER_IO_H_

#include <stddef.h> // size_t
#include <stdarg.h> // va_list
#include <stdio.h>

#include "derplanner/compiler/types.h"

namespace plnnrc {

// Simple output interface.
class Writer
{
public:
    virtual ~Writer() {}

    // write `size` bytes of `data` and return actual amount of bytes written.
    virtual size_t write(const void* data, size_t size) = 0;
};

// `FILE*` version of writer.
class Writer_Crt : public Writer
{
public:
    Writer_Crt(FILE* fd) :fd(fd) {}

    virtual size_t write(const void* data, size_t size)
    {
        return fwrite(data, 1, size, fd);
    }

    FILE* fd;
};

// creates `Writer_Crt` configured to write to `stdout` file.
Writer_Crt make_stdout_writer();

/// Formatter

// initialize `Formatter`.
void init(Formatter& formatter, const char* tab, const char* newline, uint32_t buffer_size, Writer* output);
// destroy `Formatter`.
void destroy(Formatter& formatter);

// write indentation symbols.
void indent(Formatter& formatter);
// write formatted string to the buffered output.
// `format` has the following specifiers: %s = C string, %d = decimal, %i = Token_Value.
void write(Formatter& formatter, const char* format, ...);
// start a new line.
void newline(Formatter& formatter);

// `write` variant with automatic indentation and newline.
void writeln(Formatter& formatter, const char* format, ...);

// `write` variant with `va_list`.
void write(Formatter& formatter, const char* format, va_list args);

// make sure the buffer is written to output.
void flush(Formatter& formatter);

/// Low-level interface.

void put_char(Formatter& formatter, char c);
void put_str(Formatter& formatter, const char* str);
void put_str(Formatter& formatter, const char* str, size_t length);
void put_int(Formatter& formatter, int n);
void put_token(Formatter& formatter, const Token_Value& token_value);
void put_indent(Formatter& formatter, uint32_t level);

// increase the current indentation level.
void enter_scope(Formatter& formatter);
// decrease the current indentation level.
void exit_scope(Formatter& formatter);

}

inline plnnrc::Writer_Crt plnnrc::make_stdout_writer()
{
    return plnnrc::Writer_Crt(stdout);
}

inline void plnnrc::enter_scope(plnnrc::Formatter& formatter)
{
    ++formatter.indent;
}

inline void plnnrc::exit_scope(plnnrc::Formatter& formatter)
{
    --formatter.indent;
}

#endif
