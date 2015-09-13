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
#include <stdarg.h> // va_list
#include "derplanner/compiler/io.h"

namespace plnnrc {

bool is_valid_id(const char* symbol);

class Formatter
{
public:
    Formatter(Writer& output, const char* tab="\t", const char* newline="\n");
    ~Formatter();

    bool init(size_t buffer_size);

    // high-level interface
    void writeln(const char* format, ...);
    void newline();

    // low-level interface
    void put_char(char c);
    void put_str(const char* s);
    void put_int(int n);
    void put_id(const char* s);
    void indent();
    void put_indent();
    void dedent();

    void flush();

private:
    Formatter(const Formatter&);
    const Formatter& operator=(const Formatter&);

    void _write(const char* format, va_list args);

    Writer& _output;
    char* _buffer;
    char* _buffer_top;
    char* _buffer_end;

    int _indent_level;
    const char* _tab;
    const char* _newline;
};

struct Scope
{
    Scope(Formatter& output, bool end_with_empty_line=true);
    ~Scope();

    Formatter& output;
    bool end_with_empty_line;

    Scope& operator=(const Scope&);
};

struct Class_Scope
{
    Class_Scope(Formatter& output);
    ~Class_Scope();

    Formatter& output;

    Class_Scope& operator=(const Class_Scope&);
};

struct Indented
{
    Indented(Formatter& output);
    ~Indented();

    Formatter& output;

    Indented& operator=(const Indented&);
};

class Paste_Func
{
public:
    virtual ~Paste_Func() {}
    virtual void operator()(Formatter& output) = 0;
};

}

#endif
