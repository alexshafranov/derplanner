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

namespace plnnr
{

class stack
{
public:
    stack(size_t capacity);
    ~stack();

    void* push(size_t size, size_t alignment);
    void rewind(void* position);
    void reset();

    void* top() const;

private:
    stack(const stack&);
    const stack& operator=(const stack&);

    size_t _capacity;
    char* _buffer;
    char* _top;
};

template <typename T>
T* push(stack& s)
{
    return static_cast<T*>(s.push(sizeof(T), sizeof(T)));
}

struct planner_state;

typedef bool (*expand_func)(planner_state&, void*);

struct method_instance
{
    expand_func expand;
    void* args;
    void* precondition;
    void* mrewind;
    void* trewind;
    method_instance* parent;
    bool expanded;
    int stage;
};

struct task_instance
{
    int type;
    void* args;
    task_instance* link;
};

struct planner_state
{
    method_instance* top_method;
    task_instance* top_task;
    stack* mstack;
    stack* tstack;
};

method_instance* push_method(planner_state& pstate, expand_func expand);
task_instance* push_task(planner_state& pstate, int task_type);
method_instance* rewind_top_method(planner_state& pstate, bool rewind_tasks);
bool next_branch(planner_state& pstate, expand_func expand, void* worldstate);

bool find_plan(planner_state& pstate, expand_func root_method, void* worldstate);
task_instance* reverse_task_list(task_instance* head);

}

#endif
