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

#include <stdint.h>
#include <stddef.h> // size_t

#include "derplanner/runtime/worldstate.h"
#include "derplanner/runtime/memory.h" // alignof

namespace plnnr {

class Stack
{
public:
    Stack(size_t capacity);
    ~Stack();

    void* push(size_t size, size_t alignment);

    void rewind(void* position);
    void rewind(size_t offset);

    void* ptr(size_t offset) { return _buffer + offset; }
    size_t offset(void* p) { return static_cast<char*>(p) - _buffer; }

    void reset();

    void* top() const { return _top; }
    size_t top_offset() const { return _top - _buffer; }
    void* buffer() const { return _buffer; }
    bool empty() const { return _top == _buffer; }

private:
    Stack(const Stack&);
    const Stack& operator=(const Stack&);

    size_t _capacity;
    char* _buffer;
    char* _top;
};

template <typename T>
T* push(Stack* s)
{
    return static_cast<T*>(s->push(sizeof(T), plnnr_alignof(T)));
}

template <typename T>
void push(Stack* s, const T& value)
{
    T* d = push<T>(s);
    *d = value;
}

template <typename T>
T* bottom(Stack* s)
{
    return memory::align<T>(s->buffer());
}

template <typename T>
T* top(Stack* s)
{
    return memory::align<T>(s->top()) - 1;
}

struct Planner_State;
struct Method_Instance;

typedef bool (*Expand_Func)(Method_Instance*, Planner_State&, void*);

enum Method_Flags
{
    method_flags_none       = 0x0,
    method_flags_expanded   = 0x1,
    method_flags_failed     = 0x2,
};

struct Method_Instance
{
    uint8_t             flags;
    uint16_t            expanding_branch;
    uint32_t            arguments;
    uint32_t            precondition;
    uint32_t            size;
    uint32_t            task_rewind;
    uint32_t            journal_rewind;
    uint32_t            trace_rewind;
    uint32_t            stage;
    int32_t             type;
    Expand_Func         expand;
    Method_Instance*    prev;
};

inline void* arguments(Method_Instance* method)
{
    return memory::offset(method, method->arguments);
}

inline void* precondition(Method_Instance* method)
{
    return memory::offset(method, method->precondition);
}

inline void* end(Method_Instance* method)
{
    return memory::offset(method, method->size);
}

template <typename T>
T* arguments(Method_Instance* method)
{
    return static_cast<T*>(arguments(method));
}

template <typename T>
T* precondition(Method_Instance* method)
{
    return static_cast<T*>(precondition(method));
}

struct Task_Instance
{
    uint16_t        args_align;
    uint32_t        args_size;
    int32_t         type;
    Expand_Func     expand;
    Task_Instance*  prev;
    Task_Instance*  next;
};

inline void* arguments(Task_Instance* task)
{
    return task->args_size > 0 ? memory::align(task + 1, task->args_align) : 0;
}

struct Operator_Effect
{
    tuple_list::Handle* list;
    void* tuple;
};

struct Method_Trace
{
    int32_t type;
    uint16_t branch_index;
};

struct Planner_State
{
    Method_Instance* top_method;
    Task_Instance* top_task;
    Stack* methods;
    Stack* tasks;
    Stack* journal;
    Stack* trace;
};

void reset(Planner_State& pstate);

enum Find_Plan_Status
{
    plan_not_found = 0,
    plan_in_progress,
    plan_found,
};

template <typename T>
T* push_arguments(Planner_State& pstate, Method_Instance* method)
{
    T* arguments = push<T>(pstate.methods);
    size_t method_offset = pstate.methods->offset(method);
    size_t arguments_offset = pstate.methods->offset(arguments);
    method->arguments = uint32_t(arguments_offset - method_offset);
    method->size = uint32_t(pstate.methods->top_offset() - method_offset);
    return arguments;
}

template <typename T>
T* push_precondition(Planner_State& pstate, Method_Instance* method)
{
    T* precondition = push<T>(pstate.methods);
    precondition->stage = 0;
    size_t method_offset = pstate.methods->offset(method);
    size_t precondition_offset = pstate.methods->offset(precondition);
    method->precondition = uint32_t(precondition_offset - method_offset);
    method->size = uint32_t(pstate.methods->top_offset() - method_offset);
    return precondition;
}

template <typename T>
T* push_arguments(Planner_State& pstate, Task_Instance* task)
{
    T* arguments = push<T>(pstate.tasks);
    task->args_align = plnnr_alignof(T);
    task->args_size = sizeof(T);
    return arguments;
}

Method_Instance* push_method(Planner_State& pstate, int task_type, Expand_Func expand);

Task_Instance* push_task(Planner_State& pstate, int task_type, Expand_Func expand);
Task_Instance* push_task(Planner_State& pstate, Task_Instance* task);

Method_Instance* rewind_top_method(Planner_State& pstate, bool rewind_tasks);
bool expand_next_branch(Planner_State& pstate, Expand_Func expand, void* Worldstate);

void undo_effects(Stack* journal);
Method_Instance* copy_method(Method_Instance* method, Stack* destination);

bool find_plan(Planner_State& pstate, int root_method_type, Expand_Func root_method, void* Worldstate);

void find_plan_init(Planner_State& pstate, int root_method_type, Expand_Func root_method);
void find_plan_init(Planner_State& pstate, Task_Instance* composite_task);

Find_Plan_Status find_plan_step(Planner_State& pstate, void* Worldstate);

}

#endif
