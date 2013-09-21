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
    char* memory;
    char* top;
    char  data[1];
};

struct handle
{
    page* head;
    size_t page_size;
};

handle* create(size_t page_size)
{
    size_t worstcase_size = sizeof(handle) + plnnrc_alignof(handle) + sizeof(page) + plnnrc_alignof(page);
    plnnrc_assert(page_size > worstcase_size);

    char* memory = static_cast<char*>(memory::allocate(page_size));

    if (!memory)
    {
        return 0;
    }

    handle* pool = memory::align<handle>(memory);
    page* head = memory::align<page>(pool + 1);

    head->prev = 0;
    head->memory = memory;
    head->top = head->data;

    pool->head = head;
    pool->page_size = page_size;

    return pool;
}

void* allocate(handle* pool, size_t bytes, size_t alignment)
{
    page* p = pool->head;

    char* top = static_cast<char*>(memory::align(p->top, alignment));

    if (top + bytes > p->memory + pool->page_size)
    {
        char* memory = static_cast<char*>(memory::allocate(pool->page_size));

        if (!memory)
        {
            return 0;
        }

        p = memory::align<page>(memory);
        p->prev = pool->head;
        p->memory = memory;
        p->top = p->data;

        pool->head = p;
    }

    p->top = top + bytes;

    return top;
}

void destroy(const handle* pool)
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
