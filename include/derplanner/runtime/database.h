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

void init(Fact_Table* self, Memory* mem, const Fact_Type* format, uint32_t max_entries);
void destroy(Fact_Table* self);

// resets the table size, memory is not deallocated.
void clear(Fact_Table* self);

// grows or compacts table stogare.
void set_max_entries(Fact_Table* self, uint32_t max_entries);

// retuns true if there're no tuples in the table.
bool empty(const Fact_Table* self);

// set `value` for a fact parameter.
template <typename T>
void set_arg(Fact_Table* self, uint32_t entry_index, uint32_t param_index, const T& value);

// add fact (a0,) to the table `self`.
template <typename T0>
void add_entry(Fact_Table* self, const T0& a0);

// add fact (a0, a1) to the table `self`.
template <typename T0, typename T1>
void add_entry(Fact_Table* self, const T0& a0, const T1& a1);

// add fact (a0, a1, a2) to the table `self`.
template <typename T0, typename T1, typename T2>
void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2);

// add fact (a0, a1, a2, a3) to the table `self`.
template <typename T0, typename T1, typename T2, typename T3>
void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3);

// add fact (a0, a1, a2, a3, a4) to the table `self`.
template <typename T0, typename T1, typename T2, typename T3, typename T4>
void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4);

// add fact (a0, a1, a2, a3, a4, a5) to the table `self`.
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5);

// add fact (a0, a1, a2, a3, a4, a5, a6) to the table `self`.
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6);

// add fact (a0, a1, a2, a3, a4, a6, a7) to the table `self`.
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7);

// fact argument accessors.
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                         \
    TYPE_NAME as_##TYPE_TAG(const Fact_Table* self, uint32_t entry_index, uint32_t arg_index);  \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// Param_Layout

// set `value` for a tuple `data`, which has the memory layout specified by `Param_Layout`.
template <typename T>
void set_arg(void* data, const Param_Layout* layout, uint32_t param_index, const T& value);

/// Fact_Database

void init(Fact_Database* self, Memory* mem, const Database_Format* format);
void destroy(Fact_Database* self);

// returns a handle to the first entry in database table.
Fact_Handle first(const Fact_Database* self, uint32_t table_index);
// advances handle to the next entry.
Fact_Handle next(const Fact_Database* self, Fact_Handle handle);

// search for table by fact name hash.
const Fact_Table*   find_table(const Fact_Database* self, const char* fact_name);
Fact_Table*         find_table(Fact_Database* self, const char* fact_name);

/// Fact_Handle

bool is_valid(const Fact_Database* self, Fact_Handle handle);

/// Type

// returns size of the type `t`.
uint32_t get_type_size(Type t);
// returns alignment of the type `t`.
uint32_t get_type_alignment(Type t);

// Murmur2 hash function used by compiler and runtime to generate hashes for the names used in domain (e.g. facts).
uint32_t murmur2_32(const void* key, uint32_t len, uint32_t seed);

}

#include "derplanner/runtime/database.inl"

#endif
