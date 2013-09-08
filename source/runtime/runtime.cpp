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
    _top += size;
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
    return 0;
}

task_instance* push_task(planner_state& pstate, int task_type)
{
    return 0;
}

method_instance* rewind_top_method(planner_state& pstate, bool rewind_tasks)
{
    return 0;
}

bool next_branch(planner_state& pstate, expand_func expand, void* worldstate)
{
    return false;
}

bool find_plan(planner_state& pstate, expand_func root_method, void* worldstate)
{
    return false;
}

task_instance* reverse_task_list(task_instance* head)
{
    return 0;
}

}
