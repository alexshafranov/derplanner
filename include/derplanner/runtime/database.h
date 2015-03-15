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

static const uint64_t INVALID_GENERATION_ID = 0xffffffffffffffff;

/// Fact_Database

Fact_Database create_fact_database(Memory* mem, const Database_Format& format);
void destroy(Memory* mem, Fact_Database& db);

/// Fact_Table

Fact_Table create_fact_table(Memory* mem, const Fact_Type& format, uint32_t max_entries);
void destroy(Memory* mem, Fact_Table& t);

template <typename T0>
inline void add_entry(Fact_Table& table, const T0& a0)
{
    uint32_t entry = table.num_entries;
    plnnr_assert(entry < table.max_entries);
    T0* c0 = static_cast<T0*>(table.columns[0]);
    c0[entry] = a0;
    ++table.num_entries;
}

template <typename T0, typename T1>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1)
{
    uint32_t entry = table.num_entries;
    plnnr_assert(entry < table.max_entries);
    T0* c0 = static_cast<T0*>(table.columns[0]);
    T1* c1 = static_cast<T0*>(table.columns[1]);
    c0[entry] = a0;
    c1[entry] = a1;
    ++table.num_entries;
}

/// Fact_Handle

inline bool is_valid(const Fact_Database& db, Fact_Handle handle)
{
    const Fact_Table& table = db.tables[handle.table];
    return handle.generation == table.generations[handle.entry];
}

inline bool is_valid(const Fact_Database* db, Fact_Handle handle)
{
    return is_valid(*db, handle);
}

/// Fact_Database

inline Fact_Handle first(const Fact_Database* db, uint32_t table_index)
{
    plnnr_assert(table_index < db->num_tables);
    const Fact_Table& table = db->tables[table_index];

    Fact_Handle handle;
    handle.table = table_index;
    handle.entry = 0;
    handle.generation = (table.num_entries > 0) ? table.generations[0] : INVALID_GENERATION_ID;

    return handle;
}

inline Fact_Handle next(const Fact_Database* db, Fact_Handle handle)
{
    plnnr_assert(is_valid(db, handle));
    const Fact_Table& table = db->tables[handle.table];

    Fact_Handle result;
    result.table = handle.table;

    bool hasNext = (handle.entry < table.num_entries - 1);
    if (hasNext)
    {
        result.entry = handle.entry + 1;
        result.generation = table.generations[result.entry];
    }
    else
    {
        result.entry = 0;
        result.generation = INVALID_GENERATION_ID;
    }

    return result;
}

/// as_<Type> inline accessors for Fact_Table data

// converts fact parameter with index param_index, referenced by handle to a given type.
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                             \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Table& t, Fact_Handle handle, int param_index)        \
    {                                                                                               \
        plnnr_assert(handle.entry < t.num_entries);                                                 \
        const TYPE_NAME* values = static_cast<const TYPE_NAME*>(t.columns[param_index]);            \
        return values[handle.entry];                                                                \
    }                                                                                               \
                                                                                                    \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Table* t, Fact_Handle handle, int param_index)        \
    {                                                                                               \
        return as_##TYPE_TAG(*t, handle, param_index);                                              \
    }                                                                                               \
                                                                                                    \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Database& db, Fact_Handle handle, int param_index)    \
    {                                                                                               \
        plnnr_assert(handle.table < db.num_tables);                                                 \
        const Fact_Table& table = db.tables[handle.table];                                          \
        return as_##TYPE_TAG(table, handle, param_index);                                           \
    }                                                                                               \
                                                                                                    \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Database* db, Fact_Handle handle, int param_index)    \
    {                                                                                               \
        return as_##TYPE_TAG(*db, handle, param_index);                                             \
    }                                                                                               \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

// returns size of type given type enum value
inline size_t get_type_size(Type t)
{
    switch (t)
    {
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME) \
    case Type_##TYPE_TAG: return sizeof(TYPE_NAME);
    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE
    case Type_None:
    case Type_Count:
    default:
        plnnr_assert(false);
        return 0;
    }
}

// returns alignment of type given type enum value
inline size_t get_type_alignment(Type t)
{
    switch (t)
    {
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME) \
    case Type_##TYPE_TAG: return plnnr_alignof(TYPE_NAME);
    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE
    case Type_None:
    case Type_Count:
    default:
        plnnr_assert(false);
        return 0;
    }
}

}

#endif
