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

#ifndef DERPLANNER_RUNTIME_PLANNING_H_
#define DERPLANNER_RUNTIME_PLANNING_H_

#include "derplanner/runtime/assert.h"
#include "derplanner/runtime/memory.h"
#include "derplanner/runtime/types.h"

namespace plnnr {

/// Planning_State

struct Planning_State_Config
{
    // maximum expansion depth.
    uint32_t    max_depth;
    // maximum plan length.
    uint32_t    max_plan_length;
    // maximum size of the expansions data block (arguments & preconditions).
    size_t      expansion_data_size;
    // maximum size of the plan data block (arguments & preconditions).
    size_t      plan_data_size;
};

void init(Planning_State& s, Memory* mem, const Planning_State_Config& config);
void destroy(Planning_State& s);

/// Iterative planning interface.

enum Find_Plan_Status
{
    // failed to find a plan for the current set of facts.
    Find_Plan_Failed = 0,
    // plan found.
    Find_Plan_Succeeded = 1,
    // `find_plan_step` exits with `Find_Plan_In_Progress`, when a primitive task is added to the task stack, or composite task added to the expansion stack.
    Find_Plan_In_Progress = 2,
    // `find_plan_step` exits with `Find_Plan_Max_Depth_Exceeded` when there's no space left in the expansion stack.
    Find_Plan_Max_Depth_Exceeded = 3,
    // `find_plan_step` exits with `Find_Plan_Max_Plan_Length_Exceeded` when there's no space left in the task stack.
    Find_Plan_Max_Plan_Length_Exceeded = 4,
};

// pushes the first composite task on the expansion stack.
void find_plan_init(const Domain_Info* domain, Planning_State* state);

// executes one step of the planning loop.
Find_Plan_Status find_plan_step(Fact_Database* db, Planning_State* state);

// runs the full planning loop (executes `find_plan_step` until status other than `Find_Plan_In_Progress` is returned).
Find_Plan_Status find_plan(const Domain_Info* domain, Fact_Database* db, Planning_State* state);

/// Domain_Info

const char* get_task_name(const Domain_Info* domain, uint32_t task_id);

/// Stack

template <typename T>
T* top(const Stack<T>& stack);

template <typename T>
T* pop(Stack<T>& stack);

template <typename T>
void push(Stack<T>& stack, const T& value);

template <typename T>
uint32_t size(const Stack<T>& stack);

template <typename T>
uint32_t max_size(const Stack<T>& stack);

}

#include "derplanner/runtime/planning.inl"

#endif
