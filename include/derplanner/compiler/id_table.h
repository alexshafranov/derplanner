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

#ifndef DERPLANNER_COMPILER_ID_TABLE_H_
#define DERPLANNER_COMPILER_ID_TABLE_H_

#include <stdint.h>

namespace plnnrc {

namespace ast
{
    struct node;
}

struct id_table_entry;
class id_table;

class id_table_values
{
public:
    id_table_values();
    id_table_values(const id_table* table);
    id_table_values(const id_table_values& values);
    id_table_values& operator=(const id_table_values& values);

    bool empty() const;
    ast::node* pop();

private:
    uint32_t _slot;
    const id_table* _table;
};

class id_table
{
public:
    id_table();
    ~id_table();

    bool init(uint32_t max_count);

    void insert(const char* key, ast::node* value);
    ast::node* find(const char* key) const;

    unsigned count() const { return _count; }

    id_table_values values() const;

private:
    friend id_table_values;

    id_table(const id_table&);
    const id_table& operator=(const id_table&);

    id_table_entry* _buffer;
    uint32_t _capacity;
    uint32_t _mask;
    unsigned _count;
};

}

#endif
