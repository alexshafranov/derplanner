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

#include <math.h>
#include <string.h>
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/types.h"

namespace plnnrc {

/// Id_Table

// initialize `Id_Table`.
template <typename T>
void init(Id_Table<T>& table, Memory* mem, uint32_t max_size);

// destroys id table.
template <typename T>
void destroy(Id_Table<T>& table);

// set value for the key.
template <typename T>
void set(Id_Table<T>& table, const char* key, const T& value);

template <typename T>
void set(Id_Table<T>& table, const Token_Value& token_value, const T& value);

template <typename T>
void set(Id_Table<T>& table, const char* key, uint32_t length, const T& value);


// returns pointer to value for the given key or null if it doesn't exist.
template <typename T>
T* get(Id_Table<T>& table, const char* key);

template <typename T>
T* get(Id_Table<T>& table, const Token_Value& token_value);

template <typename T>
T* get(Id_Table<T>& table, const char* key, uint32_t length);

// specializations for `T*`
template <typename T>
T* get(Id_Table<T*>& table, const char* key, uint32_t length);

template <typename T>
T* get(Id_Table<T*>& table, const char* key);

template <typename T>
T* get(Id_Table<T*>& table, const Token_Value& token_value);


template <typename T>
uint32_t size(const Id_Table<T>& table);

template <typename T>
uint32_t max_size(const Id_Table<T>& table);

}

/// Implementation

namespace id_table
{
    inline uint32_t round_up_to_pow2(uint32_t value)
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

    const uint32_t load_factor_pct = 90;

    inline uint32_t required_size(uint32_t max_size)
    {
        max_size = (uint32_t)(::ceilf((100.f * max_size) / load_factor_pct));
        return round_up_to_pow2(max_size);
    }

    // FNV-1a
    inline uint32_t hash(const char* str, uint32_t length)
    {
        uint32_t result = 0x811c9dc5;

        for (uint32_t i = 0; i < length; ++i)
        {
            char c = str[i];
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
    void _set(plnnrc::Id_Table<T>& table, const char* key, uint32_t length, const T& value)
    {
        uint32_t max_size = table.max_size;
        uint32_t curr_hash = id_table::hash(key, length);
        const char* curr_key = key;
        uint32_t curr_length = length;
        T curr_value = value;
        uint32_t ideal_slot = curr_hash & (max_size - 1);

        // robin-hood hashing: linear probing + minimizing variance of the probing distance
        for (uint32_t probe = 0; probe < max_size; ++probe)
        {
            uint32_t slot = (ideal_slot + probe) & (max_size - 1);

            uint32_t slot_hash = table.hashes[slot];
            const char* slot_key = table.keys[slot];
            uint32_t slot_length = table.lengths[slot];
            T slot_value = table.values[slot];

            // empty slot -> set value and exit.
            if (!slot_key)
            {
                table.hashes[slot] = curr_hash;
                table.keys[slot] = curr_key;
                table.lengths[slot] = curr_length;
                table.values[slot] = curr_value;
                table.size++;
                return;
            }

            // same key -> update value and exit.
            if (slot_hash == curr_hash && slot_length == curr_length)
            {
                if (strncmp(slot_key, curr_key, curr_length) == 0)
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
                table.hashes[slot] = curr_hash;
                table.keys[slot] = curr_key;
                table.lengths[slot] = curr_length;
                table.values[slot] = curr_value;

                curr_hash = slot_hash;
                curr_key = slot_key;
                curr_length = slot_length;
                curr_value = slot_value;

                ideal_slot = slot_hash & (max_size - 1);
                probe = slot_dist;
            }
        }

        // only possible when table is full.
        plnnrc_assert(false);
    }

    template <typename T>
    inline void _grow(plnnrc::Id_Table<T>& table, uint32_t new_max_size)
    {
        plnnrc_assert(new_max_size > table.max_size);
        new_max_size = id_table::required_size(new_max_size);

        plnnrc::Memory* mem = table.memory;
        plnnrc_assert(mem != 0);

        uint32_t* new_hashes = plnnrc::allocate<uint32_t>(mem, new_max_size);
        const char** new_keys = plnnrc::allocate<const char*>(mem, new_max_size);
        uint32_t* new_lengths = plnnrc::allocate<uint32_t>(mem, new_max_size);
        T* new_values = plnnrc::allocate<T>(mem, new_max_size);
        // null key means empty cell.
        memset(new_keys, 0, sizeof(new_keys)*new_max_size);

        const uint32_t old_max = table.max_size;
        uint32_t* old_hashes = table.hashes;
        const char** old_keys = table.keys;
        uint32_t* old_lengths = table.lengths;
        T* old_values = table.values;

        table.size = 0;
        table.max_size = new_max_size;
        table.hashes = new_hashes;
        table.keys = new_keys;
        table.lengths = new_lengths;
        table.values = new_values;

        for (uint32_t i = 0; i < old_max; ++i)
        {
            const char* key = old_keys[i];
            uint32_t length = old_lengths[i];
            const T& value = old_values[i];

            if (key != 0)
            {
                _set(table, key, length, value);
            }
        }

        mem->deallocate(old_values);
        mem->deallocate(old_lengths);
        mem->deallocate(old_keys);
        mem->deallocate(old_hashes);
    }
}

template <typename T>
inline plnnrc::Id_Table<T>::Id_Table()
    : size(0), max_size(0), hashes(0), keys(0), lengths(0), values(0), memory(0) {}

template <typename T>
inline plnnrc::Id_Table<T>::~Id_Table()
{
    if (memory)
    {
        destroy(*this);
    }
}

template <typename T>
void plnnrc::init(plnnrc::Id_Table<T>& table, plnnrc::Memory* mem, uint32_t max_size)
{
    max_size = id_table::required_size(max_size);
    table.size = 0;
    table.max_size = max_size;
    table.hashes = plnnrc::allocate<uint32_t>(mem, max_size);
    table.keys = plnnrc::allocate<const char*>(mem, max_size);
    table.lengths = plnnrc::allocate<uint32_t>(mem, max_size);
    table.values = plnnrc::allocate<T>(mem, max_size);
    table.memory = mem;
    memset(table.keys, 0, sizeof(table.keys[0])*max_size);
}

template <typename T>
void plnnrc::destroy(plnnrc::Id_Table<T>& table)
{
    plnnrc::Memory* mem = table.memory;
    plnnrc_assert(mem != 0);
    mem->deallocate(table.values);
    mem->deallocate(table.lengths);
    mem->deallocate(table.keys);
    mem->deallocate(table.hashes);
    memset(&table, 0, sizeof(table));
}

template <typename T>
inline void plnnrc::set(plnnrc::Id_Table<T>& table, const char* key, const T& value)
{
    uint32_t length = (uint32_t)strlen(key);
    plnnrc::set<T>(table, key, length, value);
}

template <typename T>
inline void plnnrc::set(plnnrc::Id_Table<T>& table, const plnnrc::Token_Value& token_value, const T& value)
{
    plnnrc::set<T>(table, token_value.str, token_value.length, value);
}

template <typename T>
inline void plnnrc::set(plnnrc::Id_Table<T>& table, const char* key, uint32_t length, const T& value)
{
    const uint32_t max_size = table.max_size;
    const uint32_t grow_threshold = (max_size * id_table::load_factor_pct) / 100;

    id_table::_set(table, key, length, value);

    if (table.size > grow_threshold)
    {
        id_table::_grow(table, max_size * 2);
    }
}

template <typename T>
inline T* plnnrc::get(plnnrc::Id_Table<T>& table, const char* key)
{
    uint32_t length = (uint32_t)strlen(key);
    return plnnrc::get<T>(table, key, length);
}

template <typename T>
inline T* plnnrc::get(plnnrc::Id_Table<T>& table, const plnnrc::Token_Value& token_value)
{
    return plnnrc::get<T>(table, token_value.str, token_value.length);
}

template <typename T>
inline T* plnnrc::get(plnnrc::Id_Table<T>& table, const char* key, uint32_t length)
{
    const uint32_t max_size = table.max_size;

    uint32_t hash = id_table::hash(key, length);
    uint32_t ideal_slot = hash & (max_size - 1);

    for (uint32_t probe = 0; probe < max_size; ++probe)
    {
        uint32_t slot = (ideal_slot + probe) & (max_size - 1);

        uint32_t slot_hash = table.hashes[slot];
        const char* slot_key = table.keys[slot];
        uint32_t slot_length = table.lengths[slot];

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
        if (slot_hash == hash && slot_length == length)
        {
            if (strncmp(slot_key, key, length) == 0)
            {
                return &table.values[slot];
            }
        }
    }

    return 0;
}

template <typename T>
inline T* plnnrc::get(plnnrc::Id_Table<T*>& table, const char* key, uint32_t length)
{
    T** ptr = plnnrc::get<T*>(table, key, length);
    return ptr ? *ptr : 0;
}

template <typename T>
inline T* plnnrc::get(plnnrc::Id_Table<T*>& table, const char* key)
{
    uint32_t length = (uint32_t)strlen(key);
    return plnnrc::get<T>(table, key, length);
}

template <typename T>
inline T* plnnrc::get(plnnrc::Id_Table<T*>& table, const plnnrc::Token_Value& token_value)
{
    return plnnrc::get<T>(table, token_value.str, token_value.length);
}

template <typename T>
inline uint32_t plnnrc::size(const plnnrc::Id_Table<T>& table)
{
    return table.size;
}

template <typename T>
inline uint32_t plnnrc::max_size(const plnnrc::Id_Table<T>& table)
{
    return table.max_size;
}

#endif
