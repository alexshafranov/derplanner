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

#ifndef DERPLANNER_RUNTIME_DOMAIN_H_
#define DERPLANNER_RUNTIME_DOMAIN_H_

/// Inline functions & macros used in auto-generated domain code.

#include <algorithm>    // std::sort
#include <string.h>     // memset

#include "derplanner/runtime/assert.h"
#include "derplanner/runtime/memory.h"
#include "derplanner/runtime/intrinsics.h"
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"

#define plnnr_static_array_size(array) sizeof(array) / sizeof(array[0])

// switch-based fake coroutine macros, used in precondition binding iterator functions and case expansion functions.
#define plnnr_coroutine_begin(state, label) switch (state->label) { case 0:
#define plnnr_coroutine_yield(state, label, value) state->label = value; return true; case value:;
#define plnnr_coroutine_end() } return false

namespace plnnr {

// initializes `layout` offsets and a total size, run as part of domain initialization.
void compute_offsets_and_size(Param_Layout* layout);

// allocates memory from the `blob`, by bumping the `blob->top` pointer.
uint8_t* allocate_with_layout(Linear_Blob* blob, const Param_Layout* layout);

// reverts `blob->top` pointer to the `new_top`.
inline void revert(Linear_Blob* blob, void* new_top);


// allocates `Fact_Handle` from the expansion blob.
void allocate_precond_handles(Planning_State* state, Expansion_Frame* frame, uint32_t num_handles);

// allocates memory from the expansion blob, to store precondidition output (bindings).
void* allocate_precond_bindings(Planning_State* state, const Param_Layout* output_layout);

// allocates memory from the expansion blob, to store precondidition output (bindings).
void allocate_precond_bindings(Planning_State* state, Expansion_Frame* frame, const Param_Layout* output_layout);


// initializes and pushes an `Expansion_Frame` for the compound task with id `task_id`.
void begin_compound(Planning_State* state, const Domain_Info* domain, uint32_t task_id);

// initializes and pushes a `Task_Frame` for the primitive (or compound) task with id `task_id`.
void begin_task(Planning_State* state, const Domain_Info* domain, uint32_t task_id);

// rewinds a failed case expansion data and runs `expand` for the next case.
bool expand_next_case(Planning_State* state, const Domain_Info* domain, uint32_t task_id, Expansion_Frame* frame, Fact_Database* db, Compound_Task_Expand* expand);

// updates rewinding info after each iteration of `each` case.
void continue_iteration(const Planning_State* state, Expansion_Frame* frame);

// sets an argument for the topmost compound task.
template <typename T>
void set_compound_arg(Planning_State* state, const Param_Layout* layout, uint32_t param_index, const T& value);

// sets an argument for the topmost primitive (or compound) task.
template <typename T>
void set_task_arg(Planning_State* state, const Param_Layout* layout, uint32_t param_index, const T& value);

}

inline void plnnr::compute_offsets_and_size(plnnr::Param_Layout* layout)
{
    const uint8_t num_params = layout->num_params;

    if (!num_params)
    {
        layout->size = 0;
        return;
    }

    const plnnr::Type first_type = layout->types[0];
    layout->offsets[0] = 0;

    size_t offset = get_type_size(first_type);
    for (uint8_t i = 1; i < num_params; ++i)
    {
        const plnnr::Type type = layout->types[i];
        const size_t alignment = get_type_alignment(type);
        const size_t size = get_type_size(type);

        offset = align(offset, alignment);
        layout->offsets[i] = offset;
        offset += size;
    }

    layout->size = offset;
}

inline uint8_t* plnnr::allocate_with_layout(plnnr::Linear_Blob* blob, const plnnr::Param_Layout* layout)
{
    if (!layout->size)
        return 0;

    const Type first_type = layout->types[0];
    const size_t alignment = get_type_alignment(first_type);
    const size_t size = layout->size;

    uint8_t* bytes = (uint8_t*)(align(blob->top, alignment));
    blob->top = bytes + size;
    plnnr_assert((uint32_t)(blob->top - blob->base) <= blob->max_size);

    return bytes;
}

inline void plnnr::revert(plnnr::Linear_Blob* blob, void* new_top)
{
    blob->top = (uint8_t*)(new_top);
}

inline void plnnr::allocate_precond_handles(plnnr::Planning_State* state, plnnr::Expansion_Frame* frame, uint32_t num_handles)
{
    if (!num_handles)
        return;

    plnnr::Linear_Blob* blob = &state->expansion_blob;
    uint8_t* bytes = blob->top;
    blob->top = (uint8_t*)align(blob->top, plnnr_alignof(Fact_Handle)) + sizeof(Fact_Handle) * num_handles;
    plnnr_assert((uint32_t)(blob->top - blob->base) <= blob->max_size);
    frame->handles = reinterpret_cast<Fact_Handle*>(bytes);
    frame->num_handles = (uint16_t)(num_handles);
}

inline void* plnnr::allocate_precond_bindings(plnnr::Planning_State* state, const plnnr::Param_Layout* output_type)
{
    plnnr::Linear_Blob* blob = &state->expansion_blob;
    uint8_t* bytes = allocate_with_layout(blob, output_type);
    return bytes;
}

inline void plnnr::allocate_precond_bindings(plnnr::Planning_State* state, plnnr::Expansion_Frame* frame, const plnnr::Param_Layout* output_type)
{
    frame->bindings = allocate_precond_bindings(state, output_type);
}

inline void plnnr::begin_compound(plnnr::Planning_State* state, const plnnr::Domain_Info* domain, uint32_t task_id)
{
    const uint32_t num_primitive = domain->task_info.num_primitive;
    plnnr_assert(task_id >= num_primitive && task_id < domain->task_info.num_tasks);
    plnnr::Compound_Task_Expand* expand = domain->task_info.expands[task_id - num_primitive];
    const plnnr::Param_Layout* param_layout = &domain->task_info.parameters[task_id];

    plnnr::Linear_Blob* blob = &state->expansion_blob;
    uint32_t blob_size = (uint32_t)(blob->top - blob->base);

    plnnr::Expansion_Frame frame;
    memset(&frame, 0, sizeof(plnnr::Expansion_Frame));
    frame.task_type = task_id;
    frame.expand = expand;
    frame.orig_task_count = (uint16_t)(state->task_stack.size);
    frame.orig_blob_size = blob_size;
    frame.arguments = allocate_with_layout(blob, param_layout);

    const uint32_t global_case_index = domain->task_info.first_case[task_id - num_primitive];
    const uint32_t num_handles = domain->task_info.num_case_handles[global_case_index];
    const Param_Layout* precond_output_layout = &domain->task_info.bindings[global_case_index];

    allocate_precond_handles(state, &frame, num_handles);
    allocate_precond_bindings(state, &frame, precond_output_layout);

    push(state->expansion_stack, frame);
}

inline bool plnnr::expand_next_case(plnnr::Planning_State* state, const plnnr::Domain_Info* domain, uint32_t task_id,
                                    plnnr::Expansion_Frame* frame, plnnr::Fact_Database* db, plnnr::Compound_Task_Expand* expand)
{
    const uint32_t num_primitive = domain->task_info.num_primitive;
    const Param_Layout* param_layout = &domain->task_info.parameters[task_id];

    const uint32_t next_case_index = frame->case_index + 1;
    void* arguments = frame->arguments;

    // rewind all data after this task arguments.
    plnnr::Linear_Blob* blob = &state->expansion_blob;
    // first rewind to the state before this task was added.
    blob->top = blob->base + frame->orig_blob_size;
    // next, skip over arguments (actual arguments are not cleared by this call).
    allocate_with_layout(blob, param_layout);

    memset(frame, 0, sizeof(plnnr::Expansion_Frame));
    frame->case_index = next_case_index;
    frame->expand = expand;
    frame->arguments = arguments;

    const uint32_t global_case_index = domain->task_info.first_case[task_id - num_primitive] + next_case_index;
    const uint32_t num_handles = domain->task_info.num_case_handles[global_case_index];
    const plnnr::Param_Layout* precond_output_layout = &domain->task_info.bindings[global_case_index];

    allocate_precond_handles(state, frame, num_handles);
    allocate_precond_bindings(state, frame, precond_output_layout);

    // then try the new expansion.
    return frame->expand(state, frame, db);
}

inline void plnnr::begin_task(plnnr::Planning_State* state, const plnnr::Domain_Info* domain, uint32_t task_id)
{
    plnnr_assert(task_id < domain->task_info.num_tasks);
    const plnnr::Param_Layout* param_layout = &domain->task_info.parameters[task_id];

    plnnr::Linear_Blob* blob = &state->task_blob;
    uint32_t blob_size = (uint32_t)(blob->top - blob->base);

    plnnr::Task_Frame frame;
    memset(&frame, 0, sizeof(Task_Frame));
    frame.task_type = task_id;
    frame.orig_blob_size = blob_size;
    frame.arguments = allocate_with_layout(blob, param_layout);

    push(state->task_stack, frame);
}

inline void plnnr::continue_iteration(const plnnr::Planning_State* state, plnnr::Expansion_Frame* frame)
{
    frame->orig_task_count = (uint16_t)(state->task_stack.size);
    frame->orig_blob_size = (uint32_t)(state->expansion_blob.top - state->expansion_blob.base);
    frame->status = plnnr::Expansion_Frame::Status_Was_Expanded;
}

template <typename T>
inline void plnnr::set_compound_arg(plnnr::Planning_State* state, const plnnr::Param_Layout* layout, uint32_t param_index, const T& value)
{
    plnnr::Expansion_Frame* frame = top(state->expansion_stack);
    set_arg(frame->arguments, layout, param_index, value);
}

template <typename T>
inline void plnnr::set_task_arg(plnnr::Planning_State* state, const plnnr::Param_Layout* layout, uint32_t param_index, const T& value)
{
    plnnr::Task_Frame* frame = top(state->task_stack);
    set_arg(frame->arguments, layout, param_index, value);
}

#endif
