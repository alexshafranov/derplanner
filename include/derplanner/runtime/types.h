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

#ifndef DERPLANNER_RUNTIME_TYPES_H_
#define DERPLANNER_RUNTIME_TYPES_H_

#include <stdint.h>

namespace plnnr {

// Type IDs supported by fact database.
enum Type
{
    Type_None = 0,
    #define PLNNR_TYPE(TYPE_TAG, TYPE_NAME) Type_##TYPE_TAG,
    #include "derplanner/runtime/type_tags.inl"
    #undef PLNNR_TYPE
    Type_Count,
};

enum { Max_Fact_Arity = 16 };

// Format of the fact tuple.
struct Fact_Type
{
    // number of parameters.
    uint8_t arity;
    // type of each parameter.
    Type param_type[Max_Fact_Arity];
};

// Weak pointer to a fact tuple in Fact_Database.
struct Fact_Handle
{
    // index of table in Fact_Database.
    uint64_t table      : 20;
    // index of entry in Fact_Table.
    uint64_t entry      : 20;
    // entry generation when this handle was obtained.
    uint64_t generation : 24;
};

// Collection of tuples of a single fact type.
struct Fact_Table
{
    // current number of entries stored.
    uint32_t        num_entries;
    // total allocated number of entries.
    uint32_t        max_entries;
    // linear block of memory accommodating entries' data.
    void*           blob;
    // tuples laid out in SOA order.
    void*           columns[Max_Fact_Arity];
    // generation per each entry to support weak handles.
    uint32_t*       generations;
};

// A set of fact tables.
struct Fact_Database
{
    // current number of tables.
    uint32_t        num_tables;
    // total allocated number of tables.
    uint32_t        max_tables;
    // fact name hash per each table.
    uint32_t*       hashes;
    // fact name per each table.
    const char**    names;
    // type (format) per each table.
    Fact_Type*      types;
    // table data.
    Fact_Table*     tables;
    // linear block of memory accommodating database data.
    void*           blob;
};

// Header of a single frame on expansion stack. The frame layout is: Header | Arguments | Precondition_State.
struct Expansion_Frame
{
    // in progress/expanded/failed flags.
    uint32_t                flags       : 2;
    // composite task type this frame holds data for, can be used for lookups in `Task_Info`.
    uint32_t                task_type   : 15;
    // index of the expanding case.
    uint32_t                case_index  : 15;
    // size of the parent (previous) frame.
    uint32_t                parent_size;
    // pointer to the expand function of that task/case.
    Composite_Task_Expand   task_expand;
};

// Header of a single frame on task (plan) stack. The frame layout is: Header | Arguments.
struct Task_Frame
{
    // composite/primitive task type.
    uint32_t task_type;
};

//
struct Planner_State
{
    Expansion_Frame*    top_expansion;
    Task_Frame*         top_task;
};

//
typedef bool (*Composite_Task_Expand)(Planner_State*, Expansion_Frame*, Fact_Database*);

// Database construction parameters.
struct Database_Format
{
    // number of tables (fact types).
    uint32_t        num_tables;
    // size hint (max entries) per each table.
    uint32_t*       size_hints;
    // fact format.
    Fact_Type*      types;
    // fact name hash per each table.
    uint32_t*       hashes;
    // fact name per each table
    const char**    names;
};

// Runtime task information specified by generated domain code.
struct Task_Info
{
    // number of tasks in domain.
    uint32_t                num_tasks;
    // number of primitive tasks in domain.
    uint32_t                num_primitive;
    // task name hashes (composite tasks are specified after primitive).
    uint32_t*               hashes;
    // task names (composite tasks are specified after primitive).
    const char**            names;
    // format of task parameters.
    Fact_Type*              parameters;
    // pointer to generated expand function for each composite task.
    Composite_Task_Expand*  expands;
};

// Interface to the generated domain code.
class Domain
{
public:
    virtual ~Domain() {}

    // initialize domain information.
    virtual void init() = 0;

    // format of the fact database required by this domain.
    virtual Database_Format get_database_requirements() const = 0;

    // task type information for tasks specified in this domain.
    virtual Task_Info get_task_info() const = 0;
};

}

#endif
