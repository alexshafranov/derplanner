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

#include <new>
#include <stdlib.h>
#include "derplanner/compiler/memory.h"

namespace
{
    void* default_alloc(size_t size)
    {
        return ::malloc(size);
    }

    void default_dealloc(void* ptr)
    {
        ::free(ptr);
    }

    plnnrc::Allocate*   alloc_f   = default_alloc;
    plnnrc::Deallocate* dealloc_f = default_dealloc;

    plnnrc::Memory_Default default_allocator;
}

namespace plnnrc
{
    struct Memory_Stack_Page
    {
        size_t                  size;
        Memory_Stack_Page*      prev;
        uint8_t*                blob;
        uint8_t*                top;
        uint8_t                 data[1];
    };

    enum { Bookkeeping_Size = sizeof(Memory_Stack) + plnnrc_alignof(Memory_Stack) + sizeof(Memory_Stack_Page) + plnnrc_alignof(Memory_Stack_Page) };
}

void plnnrc::set_memory_functions(Allocate* a, Deallocate* f)
{
    alloc_f = a;
    dealloc_f = f;
}

void* plnnrc::allocate(size_t size)
{
    plnnrc_assert(alloc_f != 0);
    return alloc_f(size);
}

void plnnrc::deallocate(void* ptr)
{
    plnnrc_assert(dealloc_f != 0);
    dealloc_f(ptr);
}

plnnrc::Memory* plnnrc::get_default_allocator()
{
    return &default_allocator;
}

void* plnnrc::Memory_Default::allocate(size_t size, size_t alignment)
{
    alignment = (alignment < sizeof(uint32_t)) ? sizeof(uint32_t) : alignment;
    void* p = plnnrc::allocate(sizeof(uint32_t) + alignment + size);

    uint32_t* pad = static_cast<uint32_t*>(p);
    pad[0] = 0;
    ++pad;

    uint32_t* data = static_cast<uint32_t*>(align(pad, alignment));

    for (; pad < data; ++pad)
    {
        pad[0] = 0xffffffff;
    }

    return data;
}

void plnnrc::Memory_Default::deallocate(void* ptr)
{
    if (!ptr)
    {
        return;
    }

    uint32_t* p = reinterpret_cast<uint32_t*>(ptr);

    do
    {
        --p;
    }
    while (p[0] == 0xffffffff);

    plnnrc::deallocate(p);
}

plnnrc::Memory_Stack* plnnrc::Memory_Stack::create(size_t page_size)
{
    plnnrc_assert(page_size > Bookkeeping_Size);

    uint8_t* blob = static_cast<uint8_t*>(plnnrc::allocate(page_size));
    plnnrc_assert(blob != 0);

    Memory_Stack* stack = new(plnnrc::align<Memory_Stack>(blob)) Memory_Stack;
    Memory_Stack_Page* head = plnnrc::align<Memory_Stack_Page>(stack + 1);

    head->size = page_size;
    head->prev = 0;
    head->blob = blob;
    head->top = head->data;

    stack->head = head;
    stack->page_size = page_size;
    stack->total_requested = 0;
    stack->total_allocated = page_size;

    return stack;
}

void plnnrc::Memory_Stack::destroy(plnnrc::Memory_Stack* mem)
{
    for (Memory_Stack_Page* page = mem->head; page != 0; )
    {
        Memory_Stack_Page* prev_page = page->prev;
        plnnrc::deallocate(page->blob);
        page = prev_page;
    }
}

plnnrc::Memory_Stack_Page* plnnrc::Memory_Stack::allocate_page(size_t size)
{
    uint8_t* blob = static_cast<uint8_t*>(plnnrc::allocate(size));
    plnnrc_assert(blob != 0);
    this->total_allocated += size;

    Memory_Stack_Page* page = plnnrc::align<Memory_Stack_Page>(blob);
    page->size = size;
    page->prev = this->head;
    page->blob = blob;
    page->top = page->data;
    this->head = page;

    return page;
}

void* plnnrc::Memory_Stack::allocate(size_t size, size_t alignment)
{
    Memory_Stack_Page* page = head;
    uint8_t* top = static_cast<uint8_t*>(plnnrc::align(page->top, alignment));

    if (top + size > page->blob + page->size)
    {
        size_t new_page_size = page_size;
        if (size + alignment + Bookkeeping_Size > new_page_size)
        {
            new_page_size = size + alignment + Bookkeeping_Size;
        }

        page = allocate_page(new_page_size);
        top = static_cast<uint8_t*>(plnnrc::align(page->top, alignment));
    }

    page->top = top + size;
    total_requested += size;

    return top;
}

uint8_t* plnnrc::Memory_Stack::get_top()
{
    return head->top;
}

void plnnrc::Memory_Stack::pop(const Memory_Stack_Scope* scope)
{
    Memory_Stack_Page* page = this->head;
    for (; page != scope->page; )
    {
        plnnrc_assert(page != 0);
        Memory_Stack_Page* prev_page = page->prev;
        plnnrc::deallocate(page->blob);
        page = prev_page;
    }

    plnnrc_assert(scope->top > page->blob);
    plnnrc_assert(scope->top < page->blob + page->size);
    page->top = scope->top;
    this->head = page;
}
