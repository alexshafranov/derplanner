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

#include "derplanner/runtime/planning.h"

using namespace plnnr;

Planning_State plnnr::create_planning_state(Memory* mem, size_t expansion_stack_size, size_t task_stack_size)
{
    Planning_State result;
    memset(&result, 0, sizeof(result));

    result.expansion_stack.size = expansion_stack_size;
    result.expansion_stack.bottom = allocate<uint8_t>(mem, expansion_stack_size, plnnr_alignof(Expansion_Stack_Frame));

    result.task_stack.size = task_stack_size;
    result.task_stack.bottom = allocate<uint8_t>(mem, task_stack_size, plnnr_alignof(Task_Stack_Frame));

    return result;
}

void plnnr::destroy(Memory* mem, Planning_State& s)
{
    mem->deallocate(s.task_stack.bottom);
    mem->deallocate(s.expansion_stack.bottom);
    memset(&s, 0, sizeof(s));
}
