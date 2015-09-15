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

// MurmurHash2 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.
inline uint32_t murmur2_32(const void* key, uint32_t len, uint32_t seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const uint32_t m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value
    uint32_t h = seed ^ len;

    // Mix 4 bytes at a time into the hash
    const uint8_t* data = (const uint8_t*)key;

    while(len >= 4)
    {
        uint32_t k = *(uint32_t*)data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    // Handle the last few bytes of the input array
    switch(len)
    {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
        h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
} 

enum { Invalid_Generation_Id = 0x0000000000ffffffull };

inline Fact_Database::Fact_Database()
    : memory(0)
{
}

inline Fact_Database::~Fact_Database()
{
    if (this->memory != 0)
    {
        destroy(this);
    }
}

/// Fact_Handle

inline bool is_valid(const Fact_Database* self, Fact_Handle handle)
{
    const Fact_Table* table = self->tables + handle.table;
    return handle.generation == table->generations[handle.entry];
}

/// Fact_Database

inline Fact_Handle first(const Fact_Database* self, uint32_t table_index)
{
    plnnr_assert(table_index < self->num_tables);
    const Fact_Table& table = self->tables[table_index];

    Fact_Handle handle;
    handle.table = table_index;
    handle.entry = 0;
    handle.generation = (table.num_entries > 0) ? table.generations[0] : (uint64_t) Invalid_Generation_Id;

    return handle;
}

inline Fact_Handle next(const Fact_Database* self, Fact_Handle handle)
{
    plnnr_assert(is_valid(self, handle));
    const Fact_Table& table = self->tables[handle.table];

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
        result.generation = Invalid_Generation_Id;
    }

    return result;
}

// returns size of type given type enum value
inline uint32_t get_type_size(Type t)
{
    switch (t)
    {
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME) \
    case Type_##TYPE_TAG: return (uint32_t)sizeof(TYPE_NAME);
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
inline uint32_t get_type_alignment(Type t)
{
    switch (t)
    {
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME) \
    case Type_##TYPE_TAG: return (uint32_t)plnnr_alignof(TYPE_NAME);
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

// converts fact parameter with index arg_index, referenced by handle or entry index to a given type.
#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                     \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Table* self, uint32_t entry_index, uint32_t arg_index)        \
    {                                                                                                       \
        plnnr_assert(entry_index < self->num_entries);                                                      \
        plnnr_assert(arg_index < self->format.num_params);                                                  \
        plnnr_assert(Type_##TYPE_TAG == self->format.types[arg_index]);                                     \
        const TYPE_NAME* values = static_cast<const TYPE_NAME*>(self->columns[arg_index]);                  \
        return values[entry_index];                                                                         \
    }                                                                                                       \
                                                                                                            \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Database& db, Fact_Handle handle, uint32_t arg_index)         \
    {                                                                                                       \
        plnnr_assert(handle.table < db.num_tables);                                                         \
        const Fact_Table* table = db.tables + handle.table;                                                 \
        return as_##TYPE_TAG(table, handle.entry, arg_index);                                               \
    }                                                                                                       \
                                                                                                            \
    inline TYPE_NAME as_##TYPE_TAG(const Fact_Database* db, Fact_Handle handle, uint32_t arg_index)         \
    {                                                                                                       \
        return as_##TYPE_TAG(*db, handle, arg_index);                                                       \
    }                                                                                                       \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// `as_<Type>` accessors for tuple data stored in row-major order (e.g. arguments & precondition outputs).

#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                                     \
    inline TYPE_NAME as_##TYPE_TAG(const void* data, Param_Layout layout, uint32_t arg_index)                               \
    {                                                                                                                       \
        plnnr_assert(data != 0);                                                                                            \
        plnnr_assert(arg_index < layout.num_params);                                                                        \
        plnnr_assert(Type_##TYPE_TAG == layout.types[arg_index]);                                                           \
        const uint8_t* bytes = static_cast<const uint8_t*>(data);                                                           \
        size_t offset = layout.offsets[arg_index];                                                                          \
        const TYPE_NAME* result = reinterpret_cast<const TYPE_NAME*>(bytes + offset);                                       \
        return result[0];                                                                                                   \
    }                                                                                                                       \
                                                                                                                            \
    inline TYPE_NAME as_##TYPE_TAG(const void* array, Param_Layout layout, uint32_t element_index, uint32_t arg_index)      \
    {                                                                                                                       \
        plnnr_assert(array != 0);                                                                                           \
        const uint8_t* data = static_cast<const uint8_t*>(array) + element_index * layout.size;                             \
        return as_##TYPE_TAG(data, layout, arg_index);                                                                      \
    }                                                                                                                       \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// `set_arg` functions for Fact_Table tuple data. (column-major order).

    template <typename T>
    inline void set_arg(Fact_Table*, uint32_t, uint32_t, const T&)
    {
        plnnr_assert(false);
    }

#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                                     \
    template <>                                                                                                             \
    inline void set_arg<TYPE_NAME>(Fact_Table* self, uint32_t entry_index, uint32_t param_index, const TYPE_NAME& value)    \
    {                                                                                                                       \
        plnnr_assert(entry_index < self->num_entries);                                                                      \
        plnnr_assert(param_index < self->format.num_params);                                                                \
        plnnr_assert(Type_##TYPE_TAG == self->format.types[param_index]);                                                   \
        TYPE_NAME* column = static_cast<TYPE_NAME*>(self->columns[param_index]);                                            \
        column[entry_index] = value;                                                                                        \
    }                                                                                                                       \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// `set_arg` functions for row-major tuple data. (specified by Param_Layout).

    template <typename T>
    inline void set_arg(void*, const Param_Layout*, uint32_t, const T&)
    {
        plnnr_assert(false);
    }

#define PLNNR_TYPE(TYPE_TAG, TYPE_NAME)                                                                                     \
    template <>                                                                                                             \
    inline void set_arg<TYPE_NAME>(void* data, const Param_Layout* layout, uint32_t param_index, const TYPE_NAME& value)    \
    {                                                                                                                       \
        plnnr_assert(data != 0);                                                                                            \
        plnnr_assert(param_index < layout->num_params);                                                                     \
        plnnr_assert(Type_##TYPE_TAG == layout->types[param_index]);                                                        \
        uint8_t* bytes = static_cast<uint8_t*>(data);                                                                       \
        size_t offset = layout->offsets[param_index];                                                                       \
        TYPE_NAME* param = reinterpret_cast<TYPE_NAME*>(bytes + offset);                                                    \
        *param = value;                                                                                                     \
    }                                                                                                                       \

    #include "derplanner/runtime/type_tags.inl"
#undef PLNNR_TYPE

/// Fact_Table

inline bool empty(const Fact_Table* table)
{
    return table->num_entries == 0;
}

template <typename T0>
inline void add_entry(Fact_Table* self, const T0& a0)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
}

template <typename T0, typename T1>
inline void add_entry(Fact_Table* self, const T0& a0, const T1& a1)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
    set_arg(self, entry, 1, a1);
}

template <typename T0, typename T1, typename T2>
inline void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
    set_arg(self, entry, 1, a1);
    set_arg(self, entry, 2, a2);
}

template <typename T0, typename T1, typename T2, typename T3>
inline void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
    set_arg(self, entry, 1, a1);
    set_arg(self, entry, 2, a2);
    set_arg(self, entry, 3, a3);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
    set_arg(self, entry, 1, a1);
    set_arg(self, entry, 2, a2);
    set_arg(self, entry, 3, a3);
    set_arg(self, entry, 4, a4);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
    set_arg(self, entry, 1, a1);
    set_arg(self, entry, 2, a2);
    set_arg(self, entry, 3, a3);
    set_arg(self, entry, 4, a4);
    set_arg(self, entry, 5, a5);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
    set_arg(self, entry, 1, a1);
    set_arg(self, entry, 2, a2);
    set_arg(self, entry, 3, a3);
    set_arg(self, entry, 4, a4);
    set_arg(self, entry, 5, a5);
    set_arg(self, entry, 6, a6);
}

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7>
inline void add_entry(Fact_Table* self, const T0& a0, const T1& a1, const T2& a2, const T3& a3, const T4& a4, const T5& a5, const T6& a6, const T7& a7)
{
    uint32_t entry = self->num_entries++;
    plnnr_assert(entry < self->max_entries);
    set_arg(self, entry, 0, a0);
    set_arg(self, entry, 1, a1);
    set_arg(self, entry, 2, a2);
    set_arg(self, entry, 3, a3);
    set_arg(self, entry, 4, a4);
    set_arg(self, entry, 5, a5);
    set_arg(self, entry, 6, a6);
    set_arg(self, entry, 7, a7);
}

}

#endif
