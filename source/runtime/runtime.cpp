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

void stack::reset()
{
    rewind(_buffer);
}

method_instance* push_method(planner_state& pstate, expand_func expand)
{
    method_instance* new_method = push<method_instance>(pstate.mstack);
    new_method->expand = expand;
    new_method->args = 0;
    new_method->parent = pstate.top_method;
    new_method->precondition = 0;
    new_method->trewind = 0;
    new_method->mrewind = 0;
    new_method->expanded = false;
    new_method->stage = 0;

    pstate.top_method = new_method;

    return new_method;
}

task_instance* push_task(planner_state& pstate, int task_type)
{
    task_instance* new_task = push<task_instance>(pstate.tstack);
    new_task->type = task_type;
    new_task->args = 0;
    new_task->link = pstate.top_task;

    pstate.top_task = new_task;

    return new_task;
}

method_instance* rewind_top_method(planner_state& pstate, bool rewind_tasks_and_effects)
{
    method_instance* old_top = pstate.top_method;
    method_instance* new_top = old_top->parent;

    if (new_top)
    {
        pstate.mstack->rewind(new_top->mrewind);

        if (rewind_tasks_and_effects)
        {
            // rewind tasks
            if (new_top->trewind < pstate.tstack->top())
            {
                task_instance* task = memory::align<task_instance>(new_top->trewind);
                task_instance* top_task = task->link;

                pstate.tstack->rewind(new_top->trewind);

                pstate.top_task = top_task;
            }

            // rewind effects
            if (new_top->jrewind < pstate.journal->top())
            {
                operator_effect* bottom = static_cast<operator_effect*>(new_top->jrewind);
                operator_effect* top = reinterpret_cast<operator_effect*>(pstate.journal->top()) - 1;

                for (; top != bottom-1; --top)
                {
                    tuple_list::undo(top->list, top->tuple);
                }

                pstate.journal->rewind(new_top->jrewind);
            }
        }
    }

    pstate.top_method = new_top;

    return pstate.top_method;
}

bool next_branch(planner_state& pstate, expand_func expand, void* worldstate)
{
    method_instance* method = pstate.top_method;
    method->stage = 0;
    method->expand = expand;
    pstate.mstack->rewind(method->precondition);
    return method->expand(pstate, worldstate);
}

bool find_plan(planner_state& pstate, expand_func root_method, void* worldstate)
{
    find_plan_init(pstate, root_method);

    find_plan_status status = find_plan_step(pstate, worldstate);

    while (status == plan_in_progress)
    {
        status = find_plan_step(pstate, worldstate);
    }

    return status == plan_found;
}

task_instance* reverse_task_list(task_instance* head)
{
    task_instance* new_head = 0;

    while (head)
    {
        task_instance* link = head->link;
        head->link = new_head;
        new_head = head;
        head = link;
    }

    return new_head;
}

void find_plan_init(planner_state& pstate, expand_func root_method)
{
    push_method(pstate, root_method);
}

find_plan_status find_plan_step(planner_state& pstate, void* worldstate)
{
    if (pstate.top_method)
    {
        method_instance* method = pstate.top_method;

        // if found satisfying preconditions
        if (method->expand(pstate, worldstate))
        {
            // expanded to primitive tasks => go up popping expanded methods.
            if (method == pstate.top_method)
            {
                while (method && method->expanded)
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
        // backtrack otherwise
        else
        {
            rewind_top_method(pstate, true);
        }

        return plan_in_progress;
    }

    return plan_not_found;
}

}
