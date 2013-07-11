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
#include "derplanner/compiler/assert.h"
#include "id_table.h"

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

    inline bool is_pow2(uint32_t x)
    {
        return (x != 0) && !(x & (x-1));
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
}

uint32_t id_table_required_capacity(uint32_t max_count)
{
    return next_pow2(ceilf(max_count / load_factor));
}

id_table::id_table(id_table_entry* buffer, uint32_t capacity)
    : _buffer(buffer)
    , _capacity(capacity)
    , _mask(capacity-1)
{
    plnnrc_assert(is_pow2(_capacity));
}

void id_table::reset()
{
    memset(_buffer, 0, sizeof(_buffer[0])*_capacity);
}

void id_table::insert(const char* key, ast::node* value)
{
    uint32_t hash_code = hash(key);

    uint32_t slot = hash_code & _mask;

    for (uint32_t step = 0; step < _capacity; ++step)
    {
        slot = (slot + step) & _mask;

        id_table_entry& e = _buffer[slot];

        if (!e.key)
        {
            e.key = key;
            e.value = value;
            e.hash = hash_code;
            return;
        }

        uint32_t d = probe_distance(e.hash, slot, _capacity);

        if (d < step)
        {
            id_table_entry t = e;

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

ast::node* id_table::find(const char* key) const
{
    uint32_t hash_code = hash(key);

    uint32_t slot = hash_code & _mask;

    for (uint32_t step = 0; step < _capacity; ++step)
    {
        slot = (slot + step) & _mask;

        const id_table_entry& e = _buffer[slot];

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

}
