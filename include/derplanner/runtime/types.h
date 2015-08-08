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
#include <stddef.h>

namespace plnnr {

// Three component vector, could be stored in a fact database.
struct Vec3
{
    Vec3() :x(0.f), y(0.f), z(0.f) {}
    Vec3(float x, float y, float z) :x(x), y(y), z(z) {}

    float x, y, z;
};

// 32-bit size handle to point to entities in a fact database.
typedef uint32_t Id32;

// 32-bit size handle to point to entities in a fact database.
typedef uint64_t Id64;

// Allocator.
class Memory;

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
    uint8_t num_params;
    // type of each parameter.
    Type types[Max_Fact_Arity];
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
    // type (format) of the stored data tuples.
    Fact_Type       format;
    // current number of entries stored.
    uint32_t        num_entries;
    // total allocated number of entries.
    uint32_t        max_entries;
    // linear block of memory accommodating entries' data.
    void*           blob;
    // tuples in SOA layout.
    void*           columns[Max_Fact_Arity];
    // generation per each entry to support weak handles.
    uint32_t*       generations;
    // allocator.
    Memory*         memory;
};

// A set of fact tables.
struct Fact_Database
{
    Fact_Database();
    ~Fact_Database();

    // current number of tables.
    uint32_t        num_tables;
    // total allocated number of tables.
    uint32_t        max_tables;
    // seed value generated by compiler to make unique hashes.
    uint32_t        hash_seed;
    // fact name hash per each table.
    uint32_t*       hashes;
    // fact name per each table.
    const char**    names;
    // table data.
    Fact_Table*     tables;
    // linear block of memory accommodating database data (not including tables data).
    void*           blob;
    // allocator.
    Memory*         memory;
};

struct Planning_State;
struct Expansion_Frame;

// Generated expansion function prototype.
typedef bool Compound_Task_Expand(Planning_State*, Expansion_Frame*, Fact_Database*);

// Compound task expansion state.
struct Expansion_Frame
{
    enum Status
    {
        Status_None         = 0,
        // expand has generated it's last task on expansion stack.
        Status_Expanded     = 1,
        // in `each` case, one of the possible expansions was sucessfull.
        Status_Was_Expanded = 2,
    };

    // `Status` of expand function.
    uint32_t                status              : 2;
    // compound task type, this frame is holding data for, can be used for lookups in `Task_Info`.
    uint32_t                task_type           : 15;
    // index of the expanding case. [0 .. num_task_cases).
    uint32_t                case_index          : 15;
    // the number of tasks on the task stack before expansion.
    uint16_t                orig_task_count;
    // jump label to support coroutine-like expansion behaviour.
    uint16_t                expand_label;
    // jump label to support coroutine-like precondition iteration.
    uint16_t                precond_label;
    // number of fact database handles kept by the case precondition.
    uint16_t                num_handles;
    // offset in the Planning_State::expansion_blob before any data is written.
    uint32_t                orig_blob_size;
    // the expand function of the expanding case.
    Compound_Task_Expand*   expand;

    /// Data allocated from Planning_State::expansion_blob.

    // pointer to the compound task arguments.
    void*                   arguments;
    // fact database handles kept by the case precondition.
    Fact_Handle*            handles;
    // pointer to the variable bindings generated by the case precondition.
    void*                   bindings;
};

// Primitive (or compound) task in the plan.
struct Task_Frame
{
    // compound/primitive task type.
    uint32_t    task_type;
    // offset in Planning_State::task_blob before any data is written.
    uint32_t    orig_blob_size;
    // pointer to the task arguments.
    void*       arguments;
};

// Stack storage for POD data.
template <typename T>
struct Stack
{
    uint32_t    size;
    uint32_t    max_size;
    T*          frames;
};

// Helper: fixed size linear block of memory.
struct Linear_Blob
{
    uint32_t    max_size;
    uint8_t*    top;
    uint8_t*    base;
};

class Memory;

// Planning state to support non-recursive planning.
struct Planning_State
{
    Planning_State();
    ~Planning_State();

    // maximum expansion depth.
    uint32_t                max_depth;
    // maximum plan length.
    uint32_t                max_plan_length;
    // expansion stack to support back-tracking.
    Stack<Expansion_Frame>  expansion_stack;
    // the resulting plan is stored on this stack.
    Stack<Task_Frame>       task_stack;
    // compound task arguments and precondition state storage.
    Linear_Blob             expansion_blob;
    // task arguments storage.
    Linear_Blob             task_blob;
    // allocator used for allocating & growing planning blobs and stacks.
    Memory*                 memory;
};

// Helper to wrap an array of tasks `Task_Frame`, memory is owned by `Planning_State`. 
struct Plan
{
    // array of tasks.
    Task_Frame*             tasks;
    // number of tasks in the plan.
    uint32_t                length;
};

// Database construction parameters.
struct Database_Format
{
    // number of tables (fact types).
    uint32_t        num_tables;
    // seed value generated by compiler to make unque hashes.
    uint32_t        hash_seed;
    // size hint (max entries) per each table.
    uint32_t*       size_hints;
    // fact format.
    Fact_Type*      types;
    // fact name hash per each table.
    uint32_t*       hashes;
    // fact name per each table.
    const char**    names;
};

// Describes layout of task parameters in memory.
struct Param_Layout
{
    // number of parameters.
    uint8_t     num_params;
    // type of each parameter.
    Type*       types;
    // size in bytes needed for this layout.
    size_t      size;
    // offset in bytes for each parameter.
    size_t*     offsets;
};

// Runtime task information provided by the generated domain code.
struct Task_Info
{
    // number of tasks in domain (num_primitive + num_compound).
    uint32_t                    num_tasks;
    // number of primitive tasks in domain.
    uint32_t                    num_primitive;
    // number of compound tasks in domain.
    uint32_t                    num_compound;
    // number of cases for each compound task.
    uint32_t*                   num_cases;
    // index of the first case for each task.
    uint32_t*                   first_case;
    // seed value generated by compiler to make unque hashes.
    uint32_t                    hash_seed;
    // task name hashes (compound tasks are specified after primitive tasks).
    uint32_t*                   hashes;
    // task names (compound tasks are specified after primitive tasks).
    const char**                names;
    // format of task parameters.
    Param_Layout*               parameters;
    // format of the bindings created by compound task preconditions.
    Param_Layout*               bindings;
    // number of fact handles for each case.
    uint32_t*                   num_case_handles;
    // pointer to generated expand function for each compound task.
    Compound_Task_Expand**      expands;
};

// Domain info provided by the generated domain code.
struct Domain_Info
{
    // generated tasks info.
    Task_Info           task_info;
    // required fact database format.
    Database_Format     database_req;
};

/// Domain interface function pointer types.

// Must be called before calling `Get_Domain_Info`.
typedef void Init_Domain_Info();

// Retrieves `Domain_Info` object from generated code.
typedef const Domain_Info* Get_Domain_Info();

}

#endif
