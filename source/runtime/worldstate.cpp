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
#include "derplanner/runtime/worldstate.h"

namespace plnnr {
namespace tuple_list {

struct Handle;

struct Page
{
    Page* prev;
    char* memory;
    char* top;
    char  data[1];
};

struct Handle
{
    Page* head_page;
    void* head_tuple;
    tuple_traits tuple;
    size_t page_size;
};

namespace
{
    void set_ptr(void* tuple, size_t offset, void* ptr)
    {
        void** p = reinterpret_cast<void**>(static_cast<char*>(tuple) + offset);
        *p = ptr;
    }

    void* get_ptr(void* tuple, size_t offset)
    {
        void** p = reinterpret_cast<void**>(static_cast<char*>(tuple) + offset);
        return *p;
    }

    void* allocate(Handle* tuple_list)
    {
        size_t bytes = tuple_list->tuple.size;
        size_t alignment = tuple_list->tuple.alignment;
        Page* p = tuple_list->head_page;

        char* top = static_cast<char*>(memory::align(p->top, alignment));

        if (top + bytes > p->memory + tuple_list->page_size)
        {
            char* memory = static_cast<char*>(memory::allocate(tuple_list->page_size));

            if (!memory)
            {
                return 0;
            }

            p = memory::align<Page>(memory);
            p->prev = tuple_list->head_page;
            p->memory = memory;
            p->top = p->data;

            tuple_list->head_page = p;

            top = static_cast<char*>(memory::align(p->top, alignment));
        }

        p->top = top + bytes;

        return top;
    }
}

Handle* create(tuple_traits traits, size_t items_per_page)
{
    size_t handle_size = sizeof(Handle) + plnnr_alignof(Handle);
    size_t header_size = sizeof(Page) + plnnr_alignof(Page);
    size_t page_size = header_size + items_per_page * traits.size + traits.alignment;

    char* memory = static_cast<char*>(memory::allocate(handle_size + page_size));

    if (!memory)
    {
        return 0;
    }

    Handle* tuple_list = memory::align<Handle>(memory);
    Page* head_page = memory::align<Page>(tuple_list + 1);

    head_page->prev = 0;
    head_page->memory = memory;
    head_page->top = head_page->data;

    tuple_list->head_page = head_page;
    tuple_list->head_tuple = 0;
    tuple_list->tuple = traits;
    tuple_list->page_size = page_size;

    return tuple_list;
}

void clear(Handle* tuple_list)
{
    Page* p = tuple_list->head_page;

    while (p->prev)
    {
        Page* n = p->prev;
        memory::deallocate(p->memory);
        p = n;
    }

    p->top = p->data;
    tuple_list->head_page = p;
    tuple_list->head_tuple = 0;
}

void destroy(const Handle* tuple_list)
{
    for (Page* p = tuple_list->head_page; p != 0;)
    {
        Page* n = p->prev;
        memory::deallocate(p->memory);
        p = n;
    }
}

void* append(Handle* tuple_list)
{
    void* tuple = allocate(tuple_list);

    if (!tuple)
    {
        return 0;
    }

    void* head = tuple_list->head_tuple;

    size_t next_offset = tuple_list->tuple.next_offset;
    size_t prev_offset = tuple_list->tuple.prev_offset;

    set_ptr(tuple, next_offset, 0);
    set_ptr(tuple, prev_offset, 0);

    if (head)
    {
        void* tail = get_ptr(head, prev_offset);
        plnnr_assert(tail != 0);
        set_ptr(tail, next_offset, tuple);
        set_ptr(tuple, prev_offset, tail);
        set_ptr(head, prev_offset, tuple);
    }
    else
    {
        tuple_list->head_tuple = tuple;
        set_ptr(tuple, prev_offset, tuple);
    }

    return tuple;
}

void detach(Handle* tuple_list, void* tuple)
{
    size_t next_offset = tuple_list->tuple.next_offset;
    size_t prev_offset = tuple_list->tuple.prev_offset;

    void* head = tuple_list->head_tuple;
    void* next = get_ptr(tuple, next_offset);
    void* prev = get_ptr(tuple, prev_offset);

    if (next)
    {
        set_ptr(next, prev_offset, prev);
    }
    else
    {
        set_ptr(head, prev_offset, prev);
    }

    void* prev_next = get_ptr(prev, next_offset);

    if (prev_next)
    {
        set_ptr(prev, next_offset, next);
    }
    else
    {
        tuple_list->head_tuple = next;
    }
}

void undo(Handle* tuple_list, void* tuple)
{
    size_t next_offset = tuple_list->tuple.next_offset;
    size_t prev_offset = tuple_list->tuple.prev_offset;

    void* head = tuple_list->head_tuple;
    void* prev = get_ptr(tuple, prev_offset);
    void* next = get_ptr(tuple, next_offset);

    if ((head == tuple) ||
        (next != 0 && get_ptr(next, prev_offset) == tuple) ||
        (prev != 0 && get_ptr(prev, next_offset) == tuple))
    {
        detach(tuple_list, tuple);
    }
    else
    {
        if (prev)
        {
            set_ptr(prev, next_offset, tuple);
        }

        if (next)
        {
            set_ptr(next, prev_offset, tuple);
        }

        if (!head || head == next)
        {
            tuple_list->head_tuple = tuple;
            head = tuple;
            set_ptr(prev, next_offset, 0);
        }

        if (!next)
        {
            set_ptr(head, prev_offset, tuple);
        }
    }
}

void* head(Handle* tuple_list)
{
    plnnr_assert(tuple_list);
    return tuple_list->head_tuple;
}

}
}
