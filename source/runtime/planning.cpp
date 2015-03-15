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
#include "derplanner/runtime/domain_support.h"
#include "derplanner/runtime/planning.h"

using namespace plnnr;

Planning_State plnnr::create_planning_state(Memory* mem, Planning_State_Config config)
{
    Planning_State result;
    memset(&result, 0, sizeof(result));

    Expansion_Frame* expansion_frames = allocate<Expansion_Frame>(mem, config.max_depth);
    result.expansion_stack.max_size = config.max_depth;
    result.expansion_stack.frames = expansion_frames;

    Task_Frame* task_frames = allocate<Task_Frame>(mem, config.max_plan_length);
    result.task_stack.max_size = config.max_plan_length;
    result.task_stack.frames = task_frames;

    uint8_t* expansion_data = allocate<uint8_t>(mem, config.expansion_data_size, PLNNR_DEFAULT_ALIGNMENT);
    result.expansion_blob.max_size = config.expansion_data_size;
    result.expansion_blob.top = expansion_data;
    result.expansion_blob.base = expansion_data;

    uint8_t* plan_data = allocate<uint8_t>(mem, config.plan_data_size, PLNNR_DEFAULT_ALIGNMENT);
    result.task_blob.max_size = config.plan_data_size;
    result.task_blob.top = plan_data;
    result.task_blob.base = plan_data;

    return result;
}

void plnnr::destroy(Memory* mem, Planning_State& s)
{
    mem->deallocate(s.expansion_stack.frames);
    mem->deallocate(s.task_stack.frames);
    mem->deallocate(s.expansion_blob.base);
    mem->deallocate(s.task_blob.base);
    memset(&s, 0, sizeof(s));
}

static plnnr::Expansion_Frame* pop_expansion(plnnr::Planning_State* state, bool failed)
{
    Expansion_Frame* old_top = pop(state->expansion_stack);
    Expansion_Frame* new_top = top(state->expansion_stack);

    // revert all data allocated by old_top expansion.
    Linear_Blob* blob = &state->expansion_blob;
    blob->top = blob->base + old_top->blob_rewind;

    if (new_top && failed)
    {
        new_top->flags |= Expansion_Frame::Flags_Failed;

        // now revert tasks this expansion produced before failure.
    }

    return new_top;
}

bool plnnr::find_plan(const Domain_Info* domain, Fact_Database* db, Planning_State* state)
{
    // there's no composite tasks in this domain -> bail out with an empty plan.
    if (domain->task_info.num_composite == 0)
    {
        return true;
    }

    // the root task is the first composite task in domain.
    uint32_t root_id = domain->task_info.num_primitive;
    Composite_Task_Expand* root_expand = domain->task_info.expands[0];
    Param_Layout args_layout = domain->task_info.parameters[root_id];
    // this find_plan variant doesn't support root tasks with arguments.
    plnnr_assert(args_layout.num_params == 0);

    // put the root task on stack.
    begin_composite(state, root_id, root_expand, args_layout);

    for (;;)
    {
        Expansion_Frame* frame = top(state->expansion_stack);

        if (frame->expand(state, frame, db))
        {
            Expansion_Frame* new_top_frame = top(state->expansion_stack);

            // expanded to primitive tasks -> pop all expanded composites.
            if (frame == new_top_frame && (frame->flags & Expansion_Frame::Flags_Expanded) != 0)
            {
                while (frame && (frame->flags & Expansion_Frame::Flags_Expanded) != 0)
                {
                    frame = pop_expansion(state, false);
                }

                // all composites are now expanded -> plan found.
                if (!frame)
                {
                    return true;
                }
            }
        }
        else
        {
            // expansion failed -> pop expansion and revert tasks.
            frame = pop_expansion(state, true);

            if (!frame)
            {
                return false;
            }
        }
    }
}
