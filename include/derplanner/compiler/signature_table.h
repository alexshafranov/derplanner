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

#ifndef DERPLANNER_COMPILER_SIGNATURE_TABLE_H_
#define DERPLANNER_COMPILER_SIGNATURE_TABLE_H_

#include "derplanner/compiler/types.h"
#include "derplanner/compiler/array.h"

namespace plnnrc {

void init(Signature_Table& table, Memory* mem, uint32_t max_signatures);
void destroy(Signature_Table& table);

// begin building signature.
void begin_signature(Signature_Table& table);
// add parameter type to the currenly built signature, parameter index in tuple is returned.
uint32_t add_param(Signature_Table& table, Token_Type type);
// end building signature, compactify if the same signature was already built.
void end_signature(Signature_Table& table);

// total number of signatures stored in the `table`.
uint32_t size_sparse(const Signature_Table& table);
// the number of unique signatures stored in the `table`.
uint32_t size_dense(const Signature_Table& table);

// returns signature reference given the sparse index.
Signature get_sparse(const Signature_Table& table, uint32_t sparse_index);
// returns signature reference given the dense index.
Signature get_dense(const Signature_Table& table, uint32_t dense_index);

// maps sparse index to dense index.
uint32_t get_dense_index(const Signature_Table& table, uint32_t sparse_index);

}

inline uint32_t plnnrc::size_sparse(const plnnrc::Signature_Table& table)
{
    return plnnrc::size(table.remap);
}

inline uint32_t plnnrc::size_dense(const plnnrc::Signature_Table& table)
{
    return plnnrc::size(table.hashes);
}

inline plnnrc::Signature plnnrc::get_sparse(const plnnrc::Signature_Table& table, uint32_t sparse_index)
{
    uint32_t dense_index = table.remap[sparse_index];
    return plnnrc::get_dense(table, dense_index);
}

inline plnnrc::Signature plnnrc::get_dense(const plnnrc::Signature_Table& table, uint32_t dense_index)
{
    uint32_t offset = table.offsets[dense_index];
    uint32_t length = table.lengths[dense_index];
    plnnrc::Signature result = { length > 0 ? &table.types[offset] : 0, length, offset };
    return result;
}

inline uint32_t plnnrc::get_dense_index(const plnnrc::Signature_Table& table, uint32_t sparse_index)
{
    return table.remap[sparse_index];
}

#endif
