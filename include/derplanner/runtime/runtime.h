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

#ifndef DERPLANNER_RUNTIME_RUNTIME_H_
#define DERPLANNER_RUNTIME_RUNTIME_H_

#include <stddef.h> // size_t

#include "derplanner/runtime/worldstate.h"
#include "derplanner/runtime/memory.h" // alignof
#include "derplanner/runtime/coroutine_macro.h"

namespace plnnr {

class stack
{
public:
    stack(size_t capacity);
    ~stack();

    void* push(size_t size, size_t alignment);

    void rewind(void* position);
    void reset();

    void* top() const { return _top; }
    bool empty() const { return _top > _buffer; }

private:
    stack(const stack&);
    const stack& operator=(const stack&);

    size_t _capacity;
    char* _buffer;
    char* _top;
};

template <typename T>
T* push(stack* s)
{
    return static_cast<T*>(s->push(sizeof(T), plnnr_alignof(T)));
}

struct planner_state;

typedef bool (*expand_func)(planner_state&, void*);

struct method_instance
{
    int type;
    expand_func expand;
    void* args;
    void* precondition;
    void* mrewind;
    void* trewind;
    void* jrewind;
    method_instance* parent;
    bool expanded;
    int stage;
};

struct task_instance
{
    int type;
    void* args;
    size_t args_size;
    task_instance* parent;
};

struct operator_effect
{
    tuple_list::handle* list;
    void* tuple;
};

struct planner_state
{
    method_instance* top_method;
    task_instance* top_task;
    stack* mstack;
    stack* tstack;
    stack* journal;
};

enum find_plan_status
{
    plan_found = 0,
    plan_not_found,
    plan_in_progress,
};

method_instance* push_method(planner_state& pstate, int task_type, expand_func expand);
task_instance* push_task(planner_state& pstate, int task_type);
method_instance* rewind_top_method(planner_state& pstate, bool rewind_tasks);
void undo_effects(stack* journal);
bool next_branch(planner_state& pstate, expand_func expand, void* worldstate);

task_instance* reverse_task_list(task_instance* head);

bool find_plan(planner_state& pstate, int root_method_type, expand_func root_method, void* worldstate);

void find_plan_init(planner_state& pstate, int root_method_type, expand_func root_method);
find_plan_status find_plan_step(planner_state& pstate, void* worldstate);

}

#endif
