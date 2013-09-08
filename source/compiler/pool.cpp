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

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "pool.h"

namespace plnnrc {
namespace pool {

struct page
{
    page* prev;
    void* memory;
    char* top;
    char* end;
};

struct handle
{
    page* head;
    size_t page_size;
};

handle* init(size_t page_size)
{
    size_t handle_worstcase_size = sizeof(handle) + plnnrc_alignof(handle);
    size_t page_worstcase_size = sizeof(page) + plnnrc_alignof(page);
    size_t worstcase_size = handle_worstcase_size + page_worstcase_size;

    plnnrc_assert(page_size > worstcase_size);

    void* memory = memory::allocate(page_size);

    if (!memory)
    {
        return 0;
    }

    handle* pool = static_cast<handle*>(memory::align(memory, plnnrc_alignof(handle)));
    void* after_pool = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pool) + sizeof(handle));
    page* head = static_cast<page*>(memory::align(after_pool, plnnrc_alignof(page)));

    head->prev = 0;
    head->memory = memory;
    head->top = reinterpret_cast<char*>(head) + sizeof(page);
    head->end = static_cast<char*>(memory) + page_size;

    pool->head = head;
    pool->page_size = page_size;

    return pool;
}

void* allocate(handle* pool, size_t bytes, size_t alignment)
{
    page* p = pool->head;

    if (static_cast<char*>(memory::align(p->top, alignment)) + bytes > p->end)
    {
        void* memory = memory::allocate(pool->page_size);

        if (!memory)
        {
            return 0;
        }

        p = static_cast<page*>(memory::align(memory, plnnrc_alignof(page)));
        p->prev = pool->head;
        p->memory = memory;
        p->top = reinterpret_cast<char*>(p) + sizeof(page);
        p->end = reinterpret_cast<char*>(memory) + sizeof(pool->page_size);
        pool->head = p;
    }

    void* result = memory::align(p->top, alignment);
    p->top = static_cast<char*>(result) + bytes;

    return result;
}

void clear(handle* pool)
{
    for (page* p = pool->head; p != 0;)
    {
        page* n = p->prev;
        memory::deallocate(p->memory);
        p = n;
    }
}

}
}
