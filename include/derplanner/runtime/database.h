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

#ifndef DERPLANNER_RUNTIME_DATABASE_H_
#define DERPLANNER_RUNTIME_DATABASE_H_

#include "derplanner/runtime/assert.h"
#include "derplanner/runtime/memory.h"
#include "derplanner/runtime/types.h"

namespace plnnr {

/// Fact_Table

void init(Fact_Table& table, Memory* mem, const Fact_Type& format, uint32_t max_entries);
void destroy(Fact_Table& t);

/// Fact_Database

void init(Fact_Database& db, Memory* mem, const Database_Format& format);
void destroy(Fact_Database& db);

// returns a handle to the first entry in database table.
Fact_Handle first(const Fact_Database* db, uint32_t table_index);
// advances handle to the next entry.
Fact_Handle next(const Fact_Database* db, Fact_Handle handle);

/// Fact_Handle

bool is_valid(const Fact_Database& db, Fact_Handle handle);
bool is_valid(const Fact_Database* db, Fact_Handle handle);

/// Type

// returns size of the type `t`.
size_t get_type_size(Type t);
// returns alignment of the type `t`.
size_t get_type_alignment(Type t);

}

#include "derplanner/runtime/database.inl"

#endif
