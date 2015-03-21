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

#ifndef DERPLANNER_RUNTIME_DOMAIN_SUPPORT_H_
#define DERPLANNER_RUNTIME_DOMAIN_SUPPORT_H_

#include <string.h> // memset

#include "derplanner/runtime/assert.h"
#include "derplanner/runtime/memory.h"
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"

namespace plnnr {

inline void compute_offsets_and_size(Param_Layout* layout)
{
    uint8_t num_params = layout->num_params;
    if (num_params == 0)
    {
        layout->size = 0;
        return;
    }

    Type first_type = layout->types[0];
    layout->offsets[0] = 0;
    size_t offset = get_type_size(first_type);

    for (uint8_t i = 1; i < num_params; ++i)
    {
        Type type = layout->types[i];
        size_t alignment = get_type_alignment(type);
        size_t size = get_type_size(type);

        offset = align(offset, alignment);
        layout->offsets[i] = offset;
        offset += size;
    }

    layout->size = offset;
}

inline uint8_t* allocate_with_layout(Linear_Blob* blob, Param_Layout layout)
{
    if (layout.size == 0) { return 0; }

    Type first_type = layout.types[0];
    size_t alignment = get_type_alignment(first_type);
    size_t size = layout.size;

    uint8_t* bytes = static_cast<uint8_t*>(align(blob->top, alignment));
    blob->top = bytes + size;

    return bytes;
}

/// Functions used in generated code.

inline Fact_Handle* allocate_precond_handles(Planning_State* state, Expansion_Frame* frame, uint32_t num_handles)
{
    Linear_Blob* blob = &state->expansion_blob;
    uint8_t* bytes = blob->top;
    blob->top = (uint8_t*)align(blob->top, plnnr_alignof(Fact_Handle)) + sizeof(Fact_Handle) * num_handles;
    frame->handles = reinterpret_cast<Fact_Handle*>(bytes);
    frame->num_handles = static_cast<uint16_t>(num_handles);
    return frame->handles;
}

inline void allocate_precond_result(Planning_State* state, Expansion_Frame* frame, Param_Layout output_type)
{
    Linear_Blob* blob = &state->expansion_blob;
    uint8_t* bytes = allocate_with_layout(blob, output_type);
    frame->precond_result = bytes;
}

inline void begin_composite(Planning_State* state, uint32_t id, Composite_Task_Expand* expand, Param_Layout args_layout)
{
    Linear_Blob* blob = &state->expansion_blob;
    uint32_t blob_size = static_cast<uint32_t>(blob->top - blob->base);

    Expansion_Frame frame;
    memset(&frame, 0, sizeof(Expansion_Frame));
    frame.task_type = id;
    frame.expand = expand;
    frame.orig_task_count = static_cast<uint16_t>(state->task_stack.size);
    frame.orig_blob_size = blob_size;
    frame.arguments = allocate_with_layout(blob, args_layout);

    push(state->expansion_stack, frame);
}

inline void begin_task(Planning_State* state, uint32_t id, Param_Layout args_layout)
{
    Linear_Blob* blob = &state->task_blob;
    uint32_t blob_size = static_cast<uint32_t>(blob->top - blob->base);

    Task_Frame frame;
    memset(&frame, 0, sizeof(Task_Frame));
    frame.task_type = id;
    frame.orig_blob_size = blob_size;
    frame.arguments = allocate_with_layout(blob, args_layout);

    push(state->task_stack, frame);
}

inline bool expand_next_case(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, Composite_Task_Expand* expand, Param_Layout args_layout)
{
    frame->case_index++;
    frame->expand_label = 0;
    frame->precond_label = 0;
    frame->expand = expand;

    // rewind data past arguments.
    Linear_Blob* blob = &state->expansion_blob;
    blob->top = blob->base + frame->orig_blob_size;
    allocate_with_layout(blob, args_layout);

    return frame->expand(state, frame, db);
}

template <typename T>
inline void set_composite_arg(Planning_State* state, Param_Layout layout, uint32_t param_index, const T& value)
{
    Expansion_Frame* frame = top(state->expansion_stack);
    set_arg(frame->arguments, layout, param_index, value);
}

template <typename T>
inline void set_task_arg(Planning_State* state, Param_Layout layout, uint32_t param_index, const T& value)
{
    Task_Frame* frame = top(state->task_stack);
    set_arg(frame->arguments, layout, param_index, value);
}

template <typename T>
inline void set_precond_result(Expansion_Frame* frame, Param_Layout layout, uint32_t param_index, const T& value)
{
    set_arg(frame->precond_result, layout, param_index, value);
}

}

#endif
