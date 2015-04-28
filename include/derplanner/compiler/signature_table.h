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
// add parameter type to the currenly built signature.
void add_param(Signature_Table& table, Token_Type type);
// end building signature, compactify if the same signature was already built.
void end_signature(Signature_Table& table);

// gets offset in `types` array.
uint32_t get_signature_offset(const Signature_Table& table, uint32_t index);
// gets number of parameters in signature.
uint32_t get_signature_length(const Signature_Table& table, uint32_t index);
// get type of parameter.
Token_Type get_param_type(const Signature_Table& table, uint32_t index, uint32_t param_index);

}

inline uint32_t plnnrc::get_signature_offset(const plnnrc::Signature_Table& table, uint32_t index)
{
    const uint32_t compact_index = table.remap[index];
    return table.offsets[compact_index];
}

inline uint32_t plnnrc::get_signature_length(const plnnrc::Signature_Table& table, uint32_t index)
{
    const uint32_t compact_index = table.remap[index];
    return table.lengths[compact_index];
}

#endif
