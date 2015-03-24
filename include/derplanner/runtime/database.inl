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

#ifndef DERPLANNER_RUNTIME_DATABASE_INL_
#define DERPLANNER_RUNTIME_DATABASE_INL_

namespace plnnr {

static const uint64_t INVALID_GENERATION_ID = 0x0000000000ffffff;

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

/// `as_<Type>` accessors for `Fact_Table` tuple data (stored in column-major order).

// converts fact parameter with index param_index, referenced by handle or entry index to a given type.
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                     \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Table& table, uint32_t entry_index, uint32_t param_index)     \
    {                                                                                                       \
        plnnr_assert(entry_index < table.num_entries);                                                      \
        plnnr_assert(param_index < table.format.num_params);                                                \
        plnnr_assert(Type_##TYPE_TAG == table.format.types[param_index]);                                   \
        const TYPE_NAME* values = static_cast<const TYPE_NAME*>(table.columns[param_index]);                \
        return values[entry_index];                                                                         \
    }                                                                                                       \
                                                                                                            \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Table* table, uint32_t entry_index, uint32_t param_index)     \
    {                                                                                                       \
        return as_##TYPE_TAG(*table, entry_index, param_index);                                             \
    }                                                                                                       \
                                                                                                            \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Database& db, Fact_Handle handle, uint32_t param_index)       \
    {                                                                                                       \
        plnnr_assert(handle.table < db.num_tables);                                                         \
        const Fact_Table& table = db.tables[handle.table];                                                  \
        return as_##TYPE_TAG(table, handle.entry, param_index);                                             \
    }                                                                                                       \
                                                                                                            \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Database* db, Fact_Handle handle, uint32_t param_index)       \
    {                                                                                                       \
        return as_##TYPE_TAG(*db, handle, param_index);                                                     \
    }                                                                                                       \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// `as_<Type>` accessors for tuple data stored in row-major order (e.g. arguments & precondition outputs).

#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                                     \
    inline TYPE_NAME as_##TYPE_TAG(const void* data, Param_Layout layout, uint32_t param_index)                             \
    {                                                                                                                       \
        plnnr_assert(data != 0);                                                                                            \
        plnnr_assert(param_index < layout.num_params);                                                                      \
        plnnr_assert(Type_##TYPE_TAG == layout.types[param_index]);                                                         \
        const uint8_t* bytes = static_cast<const uint8_t*>(data);                                                           \
        size_t offset = layout.offsets[param_index];                                                                        \
        const TYPE_NAME* result = reinterpret_cast<const TYPE_NAME*>(bytes + offset);                                       \
        return result[0];                                                                                                   \
    }                                                                                                                       \
                                                                                                                            \
    inline TYPE_NAME as_##TYPE_TAG(const void* array, Param_Layout layout, uint32_t element_index, uint32_t param_index)    \
    {                                                                                                                       \
        plnnr_assert(array != 0);                                                                                           \
        const uint8_t* data = static_cast<const uint8_t*>(array) + element_index * layout.size;                             \
        return as_##TYPE_TAG(data, layout, param_index);                                                                    \
    }                                                                                                                       \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// overloaded `set_arg` functions for Fact_Table tuple data. (column-major order).

#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                             \
    inline void set_arg(Fact_Table& table, uint32_t entry_index, uint32_t param_index, const TYPE_NAME& value)      \
    {                                                                                                               \
        plnnr_assert(entry_index < table.num_entries);                                                              \
        plnnr_assert(param_index < table.format.num_params);                                                        \
        plnnr_assert(Type_##TYPE_TAG == table.format.types[param_index]);                                           \
        TYPE_NAME* column = static_cast<TYPE_NAME*>(table.columns[param_index]);                                    \
        column[entry_index] = value;                                                                                \
    }                                                                                                               \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// overloaded `set_arg` functions for row-major tuple data. (specified by Param_Layout).

#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                     \
    inline void set_arg(void* data, Param_Layout layout, uint32_t param_index, const TYPE_NAME& value)      \
    {                                                                                                       \
        plnnr_assert(data != 0);                                                                            \
        plnnr_assert(param_index < layout.num_params);                                                      \
        plnnr_assert(Type_##TYPE_TAG == layout.types[param_index]);                                         \
        uint8_t* bytes = static_cast<uint8_t*>(data);                                                       \
        size_t offset = layout.offsets[param_index];                                                        \
        TYPE_NAME* param = reinterpret_cast<TYPE_NAME*>(bytes + offset);                                    \
        *param = value;                                                                                     \
    }                                                                                                       \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// Fact_Table

template <typename T0>
inline void add_entry(Fact_Table& table, const T0& a0)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
}

template <typename T0, typename T1>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
    set_arg(table, entry, 1, a1);
}

template <typename T0, typename T1, typename T2>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1, const T2& a2)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
    set_arg(table, entry, 1, a1);
    set_arg(table, entry, 2, a2);
}

template <typename T0, typename T1, typename T2, typename T3>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1, const T2& a2, const T3& a3)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
    set_arg(table, entry, 1, a1);
    set_arg(table, entry, 2, a2);
    set_arg(table, entry, 3, a3);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
    set_arg(table, entry, 1, a1);
    set_arg(table, entry, 2, a2);
    set_arg(table, entry, 3, a3);
    set_arg(table, entry, 4, a4);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
    set_arg(table, entry, 1, a1);
    set_arg(table, entry, 2, a2);
    set_arg(table, entry, 3, a3);
    set_arg(table, entry, 4, a4);
    set_arg(table, entry, 5, a5);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
    set_arg(table, entry, 1, a1);
    set_arg(table, entry, 2, a2);
    set_arg(table, entry, 3, a3);
    set_arg(table, entry, 4, a4);
    set_arg(table, entry, 5, a5);
    set_arg(table, entry, 6, a6);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline void add_entry(Fact_Table& table, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
{
    uint32_t entry = table.num_entries++;
    plnnr_assert(entry < table.max_entries);
    set_arg(table, entry, 0, a0);
    set_arg(table, entry, 1, a1);
    set_arg(table, entry, 2, a2);
    set_arg(table, entry, 3, a3);
    set_arg(table, entry, 4, a4);
    set_arg(table, entry, 5, a5);
    set_arg(table, entry, 6, a6);
    set_arg(table, entry, 7, a7);
}

}

#endif
