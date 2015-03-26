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

#ifndef DERPLANNER_COMPILER_ID_TABLE_H_
#define DERPLANNER_COMPILER_ID_TABLE_H_

#include <string.h>
#include <math.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/types.h"

namespace plnnrc {

/// Id_Table

// creates id table.
template <typename T>
void init(Id_Table<T>& table, uint32_t max_size);

// destroys id table.
template <typename T>
void destroy(Id_Table<T>& table);

// set value for the key.
template <typename T>
void set(Id_Table<T>& table, const char* key, const T& value);

// returns pointer to value for the given key or null if it doesn't exist.
template <typename T>
const T* get(const Id_Table<T>& table, const char* key);

}

/// Implementation

namespace id_table
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

    const float load_factor = 0.9f;

    inline uint32_t required_size(uint32_t max_size)
    {
        return next_pow2(static_cast<uint32_t>(ceilf(max_size / load_factor)));
    }

    // FNV-1a
    inline uint32_t hash(const char* str)
    {
        uint32_t result = 0x811c9dc5;

        for (uint8_t c = *str; c != 0; c = *(++str))
        {
            result ^= c;
            result *= 0x01000193;
        }

        return result;
    }

    // distance (number of steps) between ideal slot and actual slot
    inline uint32_t probing_distance(uint32_t hash_code, uint32_t slot, uint32_t max_size)
    {
        uint32_t ideal_slot = hash_code & (max_size - 1);
        uint32_t distance = (slot + max_size - ideal_slot) & (max_size - 1);
        return distance;        
    }

    template <typename T>
    inline void grow(plnnrc::Id_Table<T>& table, uint32_t new_max_size)
    {
        plnnrc_assert(new_max_size > table.max_size);

        const uint32_t size = table.size;
        new_max_size = id_table::required_size(new_max_size);

        const char** new_keys = static_cast<const char**>(plnnrc::allocate(new_max_size * sizeof(void*)));
        uint32_t* new_hashes = static_cast<uint32_t*>(plnnrc::allocate(new_max_size * sizeof(uint32_t)));
        T* new_values = static_cast<T*>(plnnrc::allocate(new_max_size * sizeof(T)));

        memset(new_keys,    0, sizeof(new_keys)*new_max_size);
        memset(new_hashes,  0, sizeof(new_hashes)*new_max_size);
        memset(new_values,  0, sizeof(new_values)*new_max_size);

        memcpy(new_keys,    table.keys,     sizeof(new_keys)*size);
        memcpy(new_hashes,  table.hashes,   sizeof(new_hashes)*size);
        memcpy(new_values,  table.values,   sizeof(new_values)*size);

        plnnrc::destroy(table);

        table.size = size;
        table.max_size = new_max_size;
        table.keys = new_keys;
        table.hashes = new_hashes;
        table.values = new_values;
    }
}

template <typename T>
void plnnrc::init(plnnrc::Id_Table<T>& table, uint32_t max_size)
{
    max_size = id_table::required_size(max_size);
    table.size = 0;
    table.max_size = max_size;
    table.keys = static_cast<const char**>(plnnrc::allocate(max_size * sizeof(void*)));
    table.hashes = static_cast<uint32_t*>(plnnrc::allocate(max_size * sizeof(uint32_t)));
    table.values = static_cast<T*>(plnnrc::allocate(max_size * sizeof(T)));
    memset(table.keys, 0, sizeof(table.keys[0])*max_size);
    memset(table.hashes, 0, sizeof(table.hashes[0])*max_size);
    memset(table.values, 0, sizeof(table.values[0])*max_size);
}

template <typename T>
void plnnrc::destroy(plnnrc::Id_Table<T>& table)
{
    plnnrc::deallocate(table.keys);
    plnnrc::deallocate(table.hashes);
    plnnrc::deallocate(table.values);
}

template <typename T>
void plnnrc::set(plnnrc::Id_Table<T>& table, const char* key, const T& value)
{
    const uint32_t max_size = table.max_size;

    if( table.size / float(table.max_size) > id_table::load_factor )
    {
        id_table::grow(table, max_size);
    }

    uint32_t curr_hash = id_table::hash(key);
    const char* curr_key = key;
    T curr_value = value;
    uint32_t ideal_slot = curr_hash & (max_size - 1);

    // robin-hood hashing: linear probing + minimizing variance of the probing distance
    for (uint32_t probe = 0; probe < max_size; ++probe)
    {
        uint32_t slot = (ideal_slot + probe) & (max_size - 1);

        const char* slot_key = table.keys[slot];
        uint32_t slot_hash = table.hashes[slot];
        T slot_value = table.values[slot];

        // empty slot -> set value and exit.
        if (!slot_key)
        {
            table.keys[slot] = curr_key;
            table.values[slot] = curr_value;
            table.hashes[slot] = curr_hash;
            table.size++;
            return;
        }

        // same key -> update value and exit.
        if (slot_hash == curr_hash)
        {
            if (strcmp(slot_key, curr_key) == 0)
            {
                table.values[slot] = curr_value;
                return;
            }
        }

        // robin hood probing:
        // if the number of linear probing steps needed to arrive to this slot
        // is less than the number of steps for the current key,
        // re-insert the slot key.

        uint32_t slot_dist = id_table::probing_distance(slot_hash, slot, max_size);
        if (slot_dist < probe)
        {
            table.keys[slot] = curr_key;
            table.values[slot] = curr_value;
            table.hashes[slot] = curr_hash;

            curr_key = slot_key;
            curr_value = slot_value;
            curr_hash = slot_hash;

            ideal_slot = slot_hash & (max_size - 1);
            probe = slot_dist;
        }
    }

    // only possible when table is full.
    plnnrc_assert(false);
}

template <typename T>
const T* plnnrc::get(const plnnrc::Id_Table<T>& table, const char* key)
{
    const max_size = table.max_size;

    uint32_t hash = id_table::hash(key);
    uint32_t ideal_slot = hash & (max_size - 1);

    for (uint32_t probe = 0; probe < max_size; ++probe)
    {
        uint32_t slot = (ideal_slot + probe) & (max_size - 1);

        const char* slot_key = table.keys[slot];
        uint32_t slot_hash = table.hashes[slot];
        T slot_value = table.values[slot];

        // arrived to un-occupied slot -> key is not in the table.
        if (slot_key == 0)
        {
            break;
        }

        uint32_t slot_dist = id_table::probing_distance(slot_hash, slot, max_size);
        // breaking probing distance invariant -> key is not in the table.
        if (probe > slot_dist)
        {
            break;
        }

        // check if the current slot is the key we're looking for.
        if (slot_hash == curr_hash)
        {
            if (strcmp(slot_key, key) == 0)
            {
                return table.values + slot;
            }
        }
    }

    return 0;
}

#endif
