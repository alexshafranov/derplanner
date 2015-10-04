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

#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/signature_table.h"

using namespace plnnrc;

void plnnrc::init(Signature_Table& table, Memory* mem, uint32_t max_signatures)
{
    init(table.types, mem, max_signatures * 4); // allocate for average 4 params per signature.
    init(table.hashes, mem, max_signatures);
    init(table.offsets, mem, max_signatures);
    init(table.lengths, mem, max_signatures);
    init(table.remap, mem, max_signatures);
}

void plnnrc::destroy(Signature_Table& table)
{
    destroy(table.types);
    destroy(table.hashes);
    destroy(table.offsets);
    destroy(table.lengths);
    destroy(table.remap);
}

void plnnrc::begin_signature(Signature_Table& table)
{
    const uint32_t new_index = size(table.offsets);
    const uint32_t new_offset = size(table.types);
    push_back(table.remap, new_index);
    push_back(table.offsets, new_offset);
}

uint32_t plnnrc::add_param(Signature_Table& table, Token_Type type)
{
    plnnrc_assert(is_Type(type));
    uint32_t offset = back(table.offsets);
    push_back(table.types, type);
    return size(table.types) - offset - 1;
}

static uint32_t hash(Signature_Table& table, uint32_t signature_index)
{
    const uint32_t offset = table.offsets[signature_index];
    const uint32_t length = table.lengths[signature_index];

    // FNV-1a
    uint32_t result = 0x811c9dc5;

    for (uint32_t i = 0; i < length; ++i)
    {
        uint8_t c = (uint8_t)(table.types[i + offset] - Token_Group_Type_First);
        result ^= c;
        result *= 0x01000193;
    }

    return result;
}

static bool equal(Signature_Table& table, uint32_t index_a, uint32_t index_b)
{
    const uint32_t offset_a = table.offsets[index_a];
    const uint32_t offset_b = table.offsets[index_b];
    const uint32_t length_a = table.lengths[index_a];
    const uint32_t length_b = table.lengths[index_b];

    if (length_a != length_b)
    {
        return false;
    }

    for (uint32_t i = 0; i < length_a; ++i)
    {
        const Token_Type type_a = table.types[i + offset_a];
        const Token_Type type_b = table.types[i + offset_b];

        if (type_a != type_b)
        {
            return false;
        }
    }

    return true;
}

void plnnrc::end_signature(Signature_Table& table)
{
    const uint32_t new_index = back(table.remap);
    const uint32_t new_offset = back(table.offsets);
    const uint32_t new_length = size(table.types) - new_offset;
    push_back(table.lengths, new_length);

    const uint32_t new_hash = ::hash(table, new_index);
    for (uint32_t i = 0; i < new_index; ++i)
    {
        const uint32_t hash = table.hashes[i];
        if (hash == new_hash)
        {
            if (::equal(table, i, new_index))
            {
                back(table.remap) = i;
                resize(table.offsets, new_index);
                resize(table.lengths, new_index);
                resize(table.types, new_offset);
                return;
            }
        }
    }

    push_back(table.hashes, new_hash);
}
