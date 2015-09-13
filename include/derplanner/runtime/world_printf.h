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

#ifndef DERPLANNER_RUNTIME_WORLD_PRINTF_H_
#define DERPLANNER_RUNTIME_WORLD_PRINTF_H_

#include <stdio.h>

namespace plnnr {

struct Atom_Printf;

struct World_Printf
{
    template <typename T>
    void atom_list(int /*atom_type*/, const char* name, T* head)
    {
        printf("(%s ", name);

        for (T* tuple = head; tuple != 0; tuple = tuple->next)
        {
            Atom_Printf atom_visitor;
            plnnr::reflect(*tuple, atom_visitor);

            if (tuple->next)
            {
                printf(" ");
            }
        }

        printf(")\n");
    }
};

template <typename E>
struct print_atom_element
{
    void operator()(const E& element)
    {
    }
};

template <>
struct print_atom_element<int>
{
    void operator()(const int& element)
    {
        printf("%d", element);
    }
};

struct Atom_Printf
{
    Atom_Printf()
        : current_element(0)
        , total_elements(0)
    {
    }

    void atom_begin(int /*atom_type*/, const char* /*name*/, int element_count)
    {
        total_elements = element_count;
        printf("(");
    }

    template <typename E>
    void atom_element(const E& element)
    {
        print_atom_element<E> printer;
        printer(element);

        if (++current_element < total_elements)
        {
            printf(", ");
        }
    }

    void atom_end(int /*atom_type*/, const char* /*name*/, int /*element_count*/)
    {
        printf(")");
    }

    int current_element;
    int total_elements;
};

struct task_printf
{
    void task(int /*task_type*/, const char* task_name)
    {
        printf("(%s)\n", task_name);
    }

    template <typename A>
    void task(int /*task_type*/, const char* task_name, const A* args)
    {
        printf("(%s ", task_name);
        Atom_Printf atom_visitor;
        plnnr::reflect(*args, atom_visitor);
        printf(")\n");
    }
};

}

#endif
