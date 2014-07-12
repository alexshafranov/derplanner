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

namespace ast { struct Node; }

class Id_Table_Values;
struct Id_Table_Entry;

class Id_Table
{
public:
    Id_Table();
    ~Id_Table();

    bool init(uint32_t max_count);

    bool insert(const char* key, ast::Node* value);
    ast::Node* find(const char* key) const;

    uint32_t count() const { return _count; }

    Id_Table_Values values() const;

private:
    friend class Id_Table_Values;

    Id_Table(const Id_Table&);
    const Id_Table& operator=(const Id_Table&);

    bool _allocate(uint32_t new_capacity);
    bool _grow();
    void _insert(uint32_t hash_code, const char* key, ast::Node* value);

    Id_Table_Entry* _buffer;
    uint32_t _capacity;
    uint32_t _mask;
    uint32_t _count;
};

class Id_Table_Values
{
public:
    Id_Table_Values();
    Id_Table_Values(const Id_Table* table);
    Id_Table_Values(const Id_Table_Values& values);
    Id_Table_Values& operator=(const Id_Table_Values& values);

    bool empty() const { return _slot == 0xffffffff; }
    ast::Node* value() const;
    void pop();

private:
    uint32_t _slot;
    const Id_Table* _table;
};

}

#endif
