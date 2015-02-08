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
#include "derplanner/runtime/worldstate_old.h"
#include "derplanner/runtime/runtime.h"

namespace plnnr
{

Stack::Stack(size_t capacity)
    : _capacity(capacity)
    , _buffer(0)
    , _top(0)
{
    _buffer = static_cast<char*>(memory::allocate(capacity));
    _top = _buffer;
}

Stack::~Stack()
{
    memory::deallocate(_buffer);
}

void* Stack::push(size_t size, size_t alignment)
{
    char* top = static_cast<char*>(memory::align(_top, alignment));
    _top = top + size;
    return top;
}

void Stack::rewind(void* position)
{
    _top = static_cast<char*>(position);
}

void Stack::rewind(size_t offset)
{
    _top = _buffer + offset;
}

void Stack::reset()
{
    rewind(_buffer);
}

void reset(Planner_State& pstate)
{
    pstate.top_method = 0;
    pstate.top_task = 0;

    pstate.methods->reset();
    pstate.tasks->reset();
    pstate.journal->reset();

    if (pstate.trace)
    {
        pstate.trace->reset();
    }
}

Method_Instance* copy_method(Method_Instance* method, Stack* destination)
{
    void* dest = destination->push(method->size, plnnr_alignof(Method_Instance));
    ::memcpy(dest, method, method->size);
    return static_cast<Method_Instance*>(dest);
}

Method_Instance* push_method(Planner_State& pstate, int task_type, Expand_Func expand)
{
    Method_Instance* new_method = push<Method_Instance>(pstate.methods);

    new_method->flags = method_flags_none;
    new_method->expanding_branch = 0;
    new_method->arguments = 0;
    new_method->precondition = 0;
    new_method->size = sizeof(Method_Instance);
    new_method->task_rewind = (uint32_t)pstate.tasks->top_offset();
    new_method->journal_rewind = (uint32_t)pstate.journal->top_offset();
    new_method->trace_rewind = 0;
    new_method->stage = 0;
    new_method->type = task_type;
    new_method->expand = expand;
    new_method->prev = pstate.top_method;

    if (pstate.trace)
    {
        Method_Trace trace;
        trace.type = new_method->type;
        trace.branch_index = new_method->expanding_branch;
        push(pstate.trace, trace);
        new_method->trace_rewind = (uint32_t)pstate.trace->top_offset();
    }

    pstate.top_method = new_method;

    return new_method;
}

Task_Instance* push_task(Planner_State& pstate, int task_type, Expand_Func expand)
{
    Task_Instance* new_task = push<Task_Instance>(pstate.tasks);

    new_task->args_align = 0;
    new_task->args_size = 0;
    new_task->type = task_type;
    new_task->expand = expand;
    new_task->prev = pstate.top_task;
    new_task->next = 0;

    if (pstate.top_task)
    {
        pstate.top_task->next = new_task;
    }

    pstate.top_task = new_task;

    return new_task;
}

Task_Instance* push_task(Planner_State& pstate, Task_Instance* task)
{
    Task_Instance* new_task = push_task(pstate, task->type, task->expand);

    if (arguments(task))
    {
        void* args_dst = pstate.tasks->push(task->args_size, task->args_align);
        ::memcpy(args_dst, arguments(task), task->args_size);
        new_task->args_align = task->args_align;
        new_task->args_size = task->args_size;
    }

    return new_task;
}

Method_Instance* rewind_top_method(Planner_State& pstate, bool rewind_tasks_and_effects)
{
    Method_Instance* old_top = pstate.top_method;
    Method_Instance* new_top = old_top->prev;

    if (new_top)
    {
        // rewind everything after parent method precondition
        pstate.methods->rewind(end(new_top));

        if (rewind_tasks_and_effects)
        {
            new_top->flags |= method_flags_failed;

            // rewind tasks
            if (new_top->task_rewind < pstate.tasks->top_offset())
            {
                Task_Instance* task = memory::align<Task_Instance>(pstate.tasks->ptr(new_top->task_rewind));
                Task_Instance* top_task = task->prev;

                pstate.tasks->rewind(new_top->task_rewind);

                pstate.top_task = top_task;
                pstate.top_task->next = 0;
            }

            // rewind effects
            if (new_top->journal_rewind < pstate.journal->top_offset())
            {
                Operator_Effect* bottom = static_cast<Operator_Effect*>(pstate.journal->ptr(new_top->journal_rewind));
                Operator_Effect* top = static_cast<Operator_Effect*>(pstate.journal->top()) - 1;

                for (; top != bottom-1; --top)
                {
                    tuple_list::undo(top->list, top->tuple);
                }

                pstate.journal->rewind(new_top->journal_rewind);
            }

            // rewind trace
            if (pstate.trace)
            {
                pstate.trace->rewind(new_top->trace_rewind);
            }
        }
    }

    pstate.top_method = new_top;

    return pstate.top_method;
}

void undo_effects(Stack* journal)
{
    if (!journal->empty())
    {
        Operator_Effect* top = static_cast<Operator_Effect*>(journal->top()) - 1;
        Operator_Effect* bottom = memory::align<Operator_Effect>(journal->buffer());

        for (; top != bottom-1 ; --top)
        {
            tuple_list::undo(top->list, top->tuple);
        }
    }
}

bool expand_next_branch(Planner_State& pstate, Expand_Func expand, void* worldstate)
{
    Method_Instance* method = pstate.top_method;

    method->stage = 0;
    method->expanding_branch++;
    method->expand = expand;

    method->size = method->precondition;
    pstate.methods->rewind(precondition(method));
    method->precondition = 0;

    if (pstate.trace)
    {
        Method_Trace* trace = memory::align<Method_Trace>(pstate.trace->ptr(method->trace_rewind)) - 1;
        trace->branch_index = method->expanding_branch;
    }

    return method->expand(method, pstate, worldstate);
}

bool find_plan(Planner_State& pstate, int root_method_type, Expand_Func root_method, void* worldstate)
{
    find_plan_init(pstate, root_method_type, root_method);

    Find_Plan_Status status = find_plan_step(pstate, worldstate);

    while (status == plan_in_progress)
    {
        status = find_plan_step(pstate, worldstate);
    }

    return status == plan_found;
}

void find_plan_init(Planner_State& pstate, int root_method_type, Expand_Func root_method)
{
    push_method(pstate, root_method_type, root_method);
}

void find_plan_init(Planner_State& pstate, Task_Instance* composite_task)
{
    Method_Instance* method = push_method(pstate, composite_task->type, composite_task->expand);

    if (arguments(composite_task))
    {
        void* args_dst = pstate.methods->push(composite_task->args_size, composite_task->args_align);
        ::memcpy(args_dst, arguments(composite_task), composite_task->args_size);
        size_t method_offset = pstate.methods->offset(method);
        size_t arguments_offset = pstate.methods->offset(args_dst);
        method->arguments = uint32_t(arguments_offset - method_offset);
        method->size = uint32_t(pstate.methods->top_offset() - method_offset);
    }
}

Find_Plan_Status find_plan_step(Planner_State& pstate, void* worldstate)
{
    plnnr_assert(pstate.top_method);

    Method_Instance* method = pstate.top_method;

    // if found satisfying preconditions
    if (method->expand(method, pstate, worldstate))
    {
        // expanded to primitive tasks => go up popping expanded methods.
        if (method == pstate.top_method && method->flags & method_flags_expanded)
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
    // backtrack otherwise
    else
    {
        method = rewind_top_method(pstate, true);

        if (!method)
        {
            return plan_not_found;
        }
    }

    return plan_in_progress;
}

}
