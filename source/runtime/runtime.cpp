//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
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

#include <string.h>
#include "derplanner/runtime/assert.h"
#include "derplanner/runtime/memory.h"
#include "derplanner/runtime/worldstate.h"
#include "derplanner/runtime/runtime.h"

namespace plnnr
{

stack::stack(size_t capacity)
    : _capacity(capacity)
    , _buffer(0)
    , _top(0)
{
    _buffer = static_cast<char*>(memory::allocate(capacity));
    _top = _buffer;
}

stack::~stack()
{
    memory::deallocate(_buffer);
}

void* stack::push(size_t size, size_t alignment)
{
    char* top = static_cast<char*>(memory::align(_top, alignment));
    _top = top + size;
    return top;
}

void stack::rewind(void* position)
{
    _top = static_cast<char*>(position);
}

void stack::rewind(size_t offset)
{
    _top = _buffer + offset;
}

void stack::reset()
{
    rewind(_buffer);
}

void reset(planner_state& pstate)
{
    pstate.top_method = 0;
    pstate.top_task = 0;
    pstate.mstack->reset();
    pstate.tstack->reset();
    pstate.journal->reset();
}

method_instance* copy_method(method_instance* method, stack* destination)
{
    void* dest = destination->push(method->size, plnnr_alignof(method_instance));
    ::memcpy(dest, method, method->size);
    return static_cast<method_instance*>(dest);
}

method_instance* push_method(planner_state& pstate, int task_type, expand_func expand)
{
    method_instance* new_method = push<method_instance>(pstate.mstack);

    new_method->flags = method_flags_none;
    new_method->arguments = 0;
    new_method->precondition = 0;
    new_method->size = sizeof(method_instance);
    new_method->task_rewind = 0;
    new_method->journal_rewind = 0;
    new_method->stage = 0;
    new_method->type = task_type;
    new_method->expand = expand;
    new_method->prev = pstate.top_method;

    pstate.top_method = new_method;

    return new_method;
}

task_instance* push_task(planner_state& pstate, int task_type)
{
    task_instance* new_task = push<task_instance>(pstate.tstack);

    new_task->args_align = 0;
    new_task->args_size = 0;
    new_task->type = task_type;
    new_task->prev = pstate.top_task;
    new_task->next = 0;

    if (pstate.top_task)
    {
        pstate.top_task->next = new_task;
    }

    pstate.top_task = new_task;

    return new_task;
}

void pop_task(planner_state& pstate)
{
    task_instance* top_task = pstate.top_task;
    task_instance* prev_task = top_task->prev;
    pstate.top_task = prev_task;

    if (prev_task)
    {
        prev_task->next = 0;
    }

    pstate.tstack->rewind(top_task);
}

method_instance* rewind_top_method(planner_state& pstate, bool rewind_tasks_and_effects)
{
    method_instance* old_top = pstate.top_method;
    method_instance* new_top = old_top->prev;

    if (new_top)
    {
        // rewind everything after parent method precondition
        pstate.mstack->rewind(end(new_top));

        if (rewind_tasks_and_effects)
        {
            new_top->flags |= method_flags_failed;

            // rewind tasks
            if (new_top->task_rewind < pstate.tstack->top_offset())
            {
                task_instance* task = memory::align<task_instance>(pstate.tstack->ptr(new_top->task_rewind));
                task_instance* top_task = task->prev;

                pstate.tstack->rewind(new_top->task_rewind);

                pstate.top_task = top_task;
                pstate.top_task->next = 0;
            }

            // rewind effects
            if (new_top->journal_rewind < pstate.journal->top_offset())
            {
                operator_effect* bottom = static_cast<operator_effect*>(pstate.journal->ptr(new_top->journal_rewind));
                operator_effect* top = static_cast<operator_effect*>(pstate.journal->top()) - 1;

                for (; top != bottom-1; --top)
                {
                    tuple_list::undo(top->list, top->tuple);
                }

                pstate.journal->rewind(new_top->journal_rewind);
            }
        }
    }

    pstate.top_method = new_top;

    return pstate.top_method;
}

void undo_effects(stack* journal)
{
    if (!journal->empty())
    {
        operator_effect* top = static_cast<operator_effect*>(journal->top()) - 1;
        operator_effect* bottom = memory::align<operator_effect>(journal->buffer());

        for (; top != bottom-1 ; --top)
        {
            tuple_list::undo(top->list, top->tuple);
        }
    }
}

bool next_branch(planner_state& pstate, expand_func expand, void* worldstate)
{
    method_instance* method = pstate.top_method;
    method->stage = 0;
    method->expand = expand;

    method->size = method->precondition;
    pstate.mstack->rewind(precondition(method));
    method->precondition = 0;

    return method->expand(method, pstate, worldstate);
}

bool find_plan(planner_state& pstate, int root_method_type, expand_func root_method, void* worldstate)
{
    find_plan_init(pstate, root_method_type, root_method);

    find_plan_status status = find_plan_step(pstate, worldstate);

    while (status == plan_in_progress)
    {
        status = find_plan_step(pstate, worldstate);
    }

    return status == plan_found;
}

void find_plan_init(planner_state& pstate, int root_method_type, expand_func root_method)
{
    push_method(pstate, root_method_type, root_method);
}

find_plan_status find_plan_step(planner_state& pstate, void* worldstate)
{
    if (pstate.top_method)
    {
        method_instance* method = pstate.top_method;

        // if found satisfying preconditions
        if (method->expand(method, pstate, worldstate))
        {
            if (method == pstate.top_method)
            {
                // partial plan found
                if (pstate.top_task && pstate.top_task->type == internal_task_yield)
                {
                    pop_task(pstate);
                    return plan_found_partial;
                }

                // expanded to primitive tasks => go up popping expanded methods.
                if (method->flags & method_flags_expanded)
                {
                    while (method && (method->flags & method_flags_expanded))
                    {
                        method = rewind_top_method(pstate, false);
                    }

                    // all methods were expanded => plan found.
                    if (!method)
                    {
                        return plan_found;
                    }
                }
            }
        }
        // backtrack otherwise
        else
        {
            method = rewind_top_method(pstate, true);

            while (method && (method->flags & method_flags_one_shot))
            {
                method = rewind_top_method(pstate, true);
            }
        }

        return plan_in_progress;
    }

    return plan_not_found;
}

}
