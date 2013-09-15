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

#ifndef DERPLANNER_COMPILER_FORMATTER_H_
#define DERPLANNER_COMPILER_FORMATTER_H_

#include <stddef.h> // size_t
#include "derplanner/compiler/io.h"

namespace plnnrc {

bool is_valid_id(const char* symbol);

class formatter;

struct scope
{
    scope(formatter& output);
    ~scope();

    formatter& output;
};

class formatter
{
public:
    formatter(writer& output);
    ~formatter();

    bool init(size_t buffer_size);

    void write(const char* format, ...);
    void flush();

private:
    friend scope;

    formatter(const formatter&);
    const formatter& operator=(const formatter&);

    void _putc(char c);
    void _puts(const char* s);
    void _puti(int n);
    void _putid(const char* s);

    void _put_indent();

    writer& _output;
    char* _buffer;
    char* _buffer_top;
    char* _buffer_end;

    int _indent_level;
};

}

#endif
