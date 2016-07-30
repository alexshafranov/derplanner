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

#include <string.h> // memset
#include "derplanner/runtime/domain.h"
#include "derplanner/runtime/database.h"
#include "derplanner/runtime/planning.h"

using namespace plnnr;

void plnnr::init(Planning_State* self, Memory* mem, const Planning_State_Config* config)
{
    memset(self, 0, sizeof(Planning_State));

    self->max_depth = config->max_depth;
    self->max_plan_length = config->max_plan_length;

    // allocate `max_depth + 1` frames, so that we can check for the maximum depth after the last frame is added.
    Expansion_Frame* expansion_frames = allocate<Expansion_Frame>(mem, config->max_depth + 1);
    self->expansion_stack.max_size = config->max_depth + 1;
    self->expansion_stack.frames = expansion_frames;

    // allocate `max_plan_length + 1` frames, so that we can check for the maximum length after the last frame is added.
    Task_Frame* task_frames = allocate<Task_Frame>(mem, config->max_plan_length + 1);
    self->task_stack.max_size = config->max_plan_length + 1;
    self->task_stack.frames = task_frames;

    uint8_t* expansion_data = allocate<uint8_t>(mem, config->expansion_data_size, plnnr::default_alignment);
    self->expansion_blob.max_size = (uint32_t)(config->expansion_data_size);
    self->expansion_blob.top = expansion_data;
    self->expansion_blob.base = expansion_data;

    uint8_t* plan_data = allocate<uint8_t>(mem, config->plan_data_size, plnnr::default_alignment);
    self->task_blob.max_size = (uint32_t)(config->plan_data_size);
    self->task_blob.top = plan_data;
    self->task_blob.base = plan_data;

    self->table_indices = allocate<uint32_t>(mem, config->max_bound_tables);

    self->memory = mem;
}

void plnnr::destroy(Planning_State* self)
{
    Memory* mem = self->memory;
    mem->deallocate(self->table_indices);
    mem->deallocate(self->expansion_stack.frames);
    mem->deallocate(self->task_stack.frames);
    mem->deallocate(self->expansion_blob.base);
    mem->deallocate(self->task_blob.base);
    memset(self, 0, sizeof(Planning_State));
}

bool plnnr::bind(Planning_State* self, const Domain_Info* domain, const Fact_Database* db)
{
    const Database_Format& req = domain->database_req;

    for (uint32_t i = 0; i < req.num_tables; ++i)
    {
        const Fact_Table* table = find_table(db, req.names[i]);
        if (!table)
            return false;

        uint32_t idx = (uint32_t)(table - db->tables);
        self->table_indices[i] = idx;
    }

    return true;
}

static plnnr::Expansion_Frame* pop_expansion(plnnr::Planning_State* state)
{
    Expansion_Frame* old_top = pop(&state->expansion_stack);
    Expansion_Frame* new_top = top(&state->expansion_stack);

    // revert all data allocated by old_top expansion.
    Blob* blob = &state->expansion_blob;
    blob->top = blob->base + old_top->orig_blob_size;

    return new_top;
}

static void undo_expansion(plnnr::Planning_State* state)
{
    plnnr_assert(state->expansion_stack.size > 0);

    Expansion_Frame* frame = top(&state->expansion_stack);
    frame->status = Expansion_Frame::Status_None;
    // rewind to the expand function start, so that we exit expansion loop and try the next satisifer.
    frame->expand_label = 0;

    // now revert any tasks parent expansion has produced.
    const uint32_t curr_task_count = state->task_stack.size;
    const uint32_t orig_task_count = frame->orig_task_count;

    Blob* blob = &state->task_blob;

    // any tasks added by expansion?
    if (curr_task_count > orig_task_count)
    {
        // the first task added by expansion.
        Task_Frame* task_frame = &state->task_stack.frames[orig_task_count];
        blob->top = blob->base + task_frame->orig_blob_size;
        state->task_stack.size = orig_task_count;
    }
}

void plnnr::find_plan_init(Planning_State* self, const Domain_Info* domain)
{
    plnnr_assert(domain->task_info.num_compound > 0);

    // the root task is the first compound task in domain.
    uint32_t root_id = domain->task_info.num_primitive;
    // this find_plan variant doesn't support root tasks with arguments.
    plnnr_assert(domain->task_info.parameters[root_id].num_params == 0);

    // reset previous planning loop state.
    self->expansion_stack.size = 0;
    self->task_stack.size = 0;
    self->expansion_blob.top = self->expansion_blob.base;
    self->task_blob.top = self->task_blob.base;

    // put the root task on stack.
    begin_compound(self, domain, root_id);
}

Find_Plan_Status plnnr::find_plan_step(Planning_State* self, Fact_Database* db)
{
    Expansion_Frame* frame = top(&self->expansion_stack);

    if (frame->expand(self, frame, db))
    {
        Expansion_Frame* new_top_frame = top(&self->expansion_stack);

        // expanded to primitive tasks -> pop all expanded compound tasks.
        if ((frame == new_top_frame) && (frame->status == Expansion_Frame::Status_Expanded))
        {
            while (frame && (frame->status == Expansion_Frame::Status_Expanded))
                frame = pop_expansion(self);

            // all compounds are now expanded -> plan found.
            if (!frame)
                return Find_Plan_Succeeded;
        }

        // check if the maximum expansion depth reached.
        if (size(&self->expansion_stack) > self->max_depth)
            return Find_Plan_Max_Depth_Exceeded;

        // check if the maximum plan length reached.
        if (size(&self->task_stack) > self->max_plan_length)
            return Find_Plan_Max_Plan_Length_Exceeded;

        return Find_Plan_In_Progress;
    }

    // expansion failed -> pop expansion and revert tasks.
    frame = pop_expansion(self);

    if (!frame)
        return Find_Plan_Failed;

    undo_expansion(self);

    return Find_Plan_In_Progress;
}

Find_Plan_Status plnnr::find_plan(Planning_State* self, Fact_Database* db, const Domain_Info* domain)
{
    find_plan_init(self, domain);

    Find_Plan_Status status = find_plan_step(self, db);
    while (status == Find_Plan_In_Progress)
        status = find_plan_step(self, db);

    return status;
}
