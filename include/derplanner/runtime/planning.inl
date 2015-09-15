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

#ifndef DERPLANNER_RUNTIME_PLANNING_INL_
#define DERPLANNER_RUNTIME_PLANNING_INL_

/// Planning_State

inline plnnr::Planning_State::Planning_State()
    : memory(0)
{
}

inline plnnr::Planning_State::~Planning_State()
{
    if (memory != 0)
    {
        destroy(this);
    }
}

inline plnnr::Plan plnnr::get_plan(const plnnr::Planning_State* self)
{
    plnnr::Plan plan;
    plan.tasks = self->task_stack.frames;
    plan.length = size(&self->task_stack);
    return plan;
}

/// Domain_Info

inline const char* plnnr::get_task_name(const plnnr::Domain_Info* domain, uint32_t task_id)
{
    plnnr_assert(task_id < domain->task_info.num_tasks);
    return domain->task_info.names ? domain->task_info.names[task_id] : 0;
}

inline plnnr::Param_Layout plnnr::get_task_param_layout(const plnnr::Domain_Info* domain, uint32_t task_id)
{
    return domain->task_info.parameters[task_id];
}

/// Stack

template <typename T>
inline T* plnnr::top(const plnnr::Stack<T>* self)
{
    return (self->size > 0) ? self->frames + (self->size - 1) : 0;
}

template <typename T>
inline T* plnnr::pop(plnnr::Stack<T>* self)
{
    plnnr_assert(self->size > 0);
    T* result = self->frames + (self->size - 1);
    --self->size;
    return result;
}

template <typename T>
inline void plnnr::push(plnnr::Stack<T>* self, const T& value)
{
    plnnr_assert(size(self) < max_size(self));
    self->frames[self->size++] = value;
}

template <typename T>
inline uint32_t plnnr::size(const plnnr::Stack<T>* self)
{
    return self->size;
}

template <typename T>
inline uint32_t plnnr::max_size(const plnnr::Stack<T>* self)
{
    return self->max_size;
}

#endif
