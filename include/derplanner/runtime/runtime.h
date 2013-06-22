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

namespace plnnr
{

struct stack
{
};

struct method_expansion;

typedef bool (*expand_func)(method_expansion*, stack&, stack&, void*);

struct method_instance
{
    void*               args;     // struct with arguments.
    expand_func         tail;     // expand primitive tasks which follow after this method.
    expand_func         expand;   // method expansion.
    method_instance*    next;     // next method in task list.
};

struct method_expansion
{
    void*               rewind;       // stack top before pushing this branch.
    void*               args;         // argument struct.
    void*               precondition; // branch precondition state.
    expand_func         expand;       // function which expands the branch task list.
    method_instance*    method;       // next method in the branch task list to expand.
    void*               mrewind;      // stack top before expanding this branch.
    void*               prewind;      // primitive task stack top before expanding this branch.
    method_expansion*   previous;     // parent branch on a stack.
};

struct task_instance
{
    unsigned          type;    // type of the task.
    void*             args;    // pointer to the arguments struct.
    task_instance*    link;    // 'previous' task during find_plan, reversed when plan is found.
};

bool find_plan(void* world, stack& mstack, stack& pstack);

}

#endif
