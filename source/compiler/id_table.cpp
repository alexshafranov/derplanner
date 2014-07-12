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

#include <string.h>
#include <math.h>
#include <stddef.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/id_table.h"

namespace plnnrc {

namespace
{
    inline uint32_t next_pow2(uint32_t value)
    {
        value--;
        value = (value >> 1)  | value;
        value = (value >> 2)  | value;
        value = (value >> 4)  | value;
        value = (value >> 8)  | value;
        value = (value >> 16) | value;
        value++;
        return value;
    }

    // bernstein hash
    inline uint32_t hash(const char* str)
    {
        uint32_t result = 0;

        for (const char* c = str; *c != 0; ++c)
        {
            result = 33 * result + *c;
        }

        return result;
    }

    inline uint32_t probe_distance(uint32_t hash_code, uint32_t slot, uint32_t capacity)
    {
        return (slot + capacity - (hash_code & (capacity-1))) & (capacity-1);
    }

    const float load_factor = 0.9f;

    inline uint32_t required_capacity(uint32_t max_count)
    {
        return next_pow2((uint32_t)ceilf(max_count / load_factor));
    }
}

struct Id_Table_Entry
{
    const char* key;
    uint32_t    hash;
    ast::Node*  value;
};

Id_Table::Id_Table()
    : _buffer(0)
    , _capacity(0)
    , _mask(0)
    , _count(0)
{
}

bool Id_Table::init(uint32_t max_count)
{
    memory::deallocate(_buffer);
    return _allocate(required_capacity(max_count));
}

Id_Table::~Id_Table()
{
    memory::deallocate(_buffer);
}

bool Id_Table::_allocate(uint32_t new_capacity)
{
    Id_Table_Entry* buffer = static_cast<Id_Table_Entry*>(memory::allocate(sizeof(buffer[0])*new_capacity));

    if (!buffer)
    {
        return false;
    }

    _count = 0;
    _capacity = new_capacity;
    _mask = _capacity - 1;
    _buffer = buffer;
    memset(_buffer, 0, sizeof(_buffer[0])*_capacity);

    return true;
}

void Id_Table::_insert(uint32_t hash_code, const char* key, ast::Node* value)
{
    plnnrc_assert(_buffer != 0);

    uint32_t slot = hash_code & _mask;

    for (uint32_t step = 0; step < _capacity; ++step)
    {
        slot = (slot + step) & _mask;

        Id_Table_Entry& e = _buffer[slot];

        if (!e.key)
        {
            e.key = key;
            e.value = value;
            e.hash = hash_code;
            _count++;
            return;
        }

        if (hash_code == e.hash)
        {
            if (strcmp(key, e.key) == 0)
            {
                e.value = value;
                return;
            }
        }

        uint32_t d = probe_distance(e.hash, slot, _capacity);

        if (d < step)
        {
            Id_Table_Entry t = e;

            e.key = key;
            e.value = value;
            e.hash = hash_code;

            key = t.key;
            value = t.value;
            hash_code = t.hash;

            step = d;
        }
    }

    plnnrc_assert(false);
}

bool Id_Table::_grow()
{
    plnnrc_assert(_buffer != 0);

    Id_Table_Entry* old_buffer = _buffer;
    uint32_t old_capacity = _capacity;

    if (!_allocate(old_capacity*2))
    {
        return false;
    }

    for (uint32_t i = 0; i < old_capacity; ++i)
    {
        Id_Table_Entry& old_entry = old_buffer[i];

        if (old_entry.key)
        {
            _insert(old_entry.hash, old_entry.key, old_entry.value);
        }
    }

    memory::deallocate(old_buffer);

    return true;
}

bool Id_Table::insert(const char* key, ast::Node* value)
{
    plnnrc_assert(_buffer != 0);

    if (_count + 1 >= _capacity * load_factor)
    {
        if (!_grow())
        {
            return false;
        }
    }

    uint32_t hash_code = hash(key);

    _insert(hash_code, key, value);

    return true;
}

ast::Node* Id_Table::find(const char* key) const
{
    plnnrc_assert(key != 0);

    if (!_buffer)
    {
        return 0;
    }

    uint32_t hash_code = hash(key);

    uint32_t slot = hash_code & _mask;

    for (uint32_t step = 0; step < _capacity; ++step)
    {
        slot = (slot + step) & _mask;

        const Id_Table_Entry& e = _buffer[slot];

        if (!e.key)
        {
            break;
        }

        uint32_t d = probe_distance(e.hash, slot, _capacity);

        if (step > d)
        {
            break;
        }

        if (e.hash == hash_code)
        {
            if (strcmp(key, e.key) == 0)
            {
                return e.value;
            }
        }
    }

    return 0;
}

Id_Table_Values::Id_Table_Values()
    : _slot(0xffffffff)
    , _table(0)
{
}

Id_Table_Values::Id_Table_Values(const Id_Table* table)
    : _slot(0xffffffff)
    , _table(table)
{
    for (uint32_t s = 0; s < _table->_capacity; ++s)
    {
        if (_table->_buffer[s].key)
        {
            _slot = s;
            break;
        }
    }
}

Id_Table_Values::Id_Table_Values(const Id_Table_Values& values)
    : _slot(values._slot)
    , _table(values._table)
{
}

Id_Table_Values& Id_Table_Values::operator=(const Id_Table_Values& values)
{
    _slot = values._slot;
    _table = values._table;
    return *this;
}

void Id_Table_Values::pop()
{
    if (!empty())
    {
        uint32_t s = _slot + 1;
        _slot = 0xffffffff;

        for (; s < _table->_capacity; ++s)
        {
            if (_table->_buffer[s].key)
            {
                _slot = s;
                break;
            }
        }
    }
}

ast::Node* Id_Table_Values::value() const
{
    plnnrc_assert(!empty() && _table);
    return _table->_buffer[_slot].value;
}

Id_Table_Values Id_Table::values() const
{
    return Id_Table_Values(this);
}

}
