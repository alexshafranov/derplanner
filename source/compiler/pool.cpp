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

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "pool.h"

namespace plnnrc
{
    struct Page
    {
        Page*       prev;
        uint8_t*    memory;
        uint8_t*    top;
        uint8_t     data[1];
    };

    struct Paged_Pool
    {
        Page*   head;
        size_t  page_size;
    };
}

using namespace plnnrc;

plnnrc::Paged_Pool* plnnrc::create_paged_pool(size_t page_size)
{
    size_t worstcase_size = sizeof(Paged_Pool) + plnnrc_alignof(Paged_Pool) + sizeof(Page) + plnnrc_alignof(Page);
    plnnrc_assert(page_size > worstcase_size);
    (void)(worstcase_size);

    uint8_t* memory = static_cast<uint8_t*>(plnnrc::allocate(page_size));

    if (!memory)
    {
        return 0;
    }

    Paged_Pool* pool = plnnrc::align<Paged_Pool>(memory);
    Page* head = plnnrc::align<Page>(pool + 1);

    head->prev = 0;
    head->memory = memory;
    head->top = head->data;

    pool->head = head;
    pool->page_size = page_size;

    return pool;
}

void* plnnrc::allocate(Paged_Pool* pool, size_t bytes, size_t alignment)
{
    Page* p = pool->head;

    uint8_t* top = static_cast<uint8_t*>(plnnrc::align(p->top, alignment));

    if (top + bytes > p->memory + pool->page_size)
    {
        uint8_t* memory = static_cast<uint8_t*>(plnnrc::allocate(pool->page_size));

        if (!memory)
        {
            return 0;
        }

        p = plnnrc::align<Page>(memory);
        p->prev = pool->head;
        p->memory = memory;
        p->top = p->data;

        pool->head = p;

        top = static_cast<uint8_t*>(plnnrc::align(p->top, alignment));
    }

    p->top = top + bytes;

    return top;
}

void plnnrc::destroy(const Paged_Pool* pool)
{
    for (Page* p = pool->head; p != 0;)
    {
        Page* n = p->prev;
        plnnrc::deallocate(p->memory);
        p = n;
    }
}
