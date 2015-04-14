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

    struct Pool
    {
        Page*   head;
        size_t  page_size;
    };
}

using namespace plnnrc;

static Page* allocate_page(Pool* pool)
{
    uint8_t* memory = static_cast<uint8_t*>(plnnrc::allocate(pool->page_size));
    plnnrc_assert(memory != 0);

    Page* page = plnnrc::align<Page>(memory);
    page->prev = pool->head;
    page->memory = memory;
    page->top = page->data;
    pool->head = page;

    return page;
}

plnnrc::Pool* plnnrc::create_paged_pool(size_t page_size)
{
    const size_t bookkeeping_size = sizeof(Pool) + plnnrc_alignof(Pool) + sizeof(Page) + plnnrc_alignof(Page);
    plnnrc_assert(page_size > bookkeeping_size);
    (void)(bookkeeping_size);

    uint8_t* memory = static_cast<uint8_t*>(plnnrc::allocate(page_size));
    plnnrc_assert(memory != 0);

    Pool* pool = plnnrc::align<Pool>(memory);
    Page* head = plnnrc::align<Page>(pool + 1);

    head->prev = 0;
    head->memory = memory;
    head->top = head->data;

    pool->head = head;
    pool->page_size = page_size;

    return pool;
}

void* plnnrc::allocate(Pool* pool, size_t bytes, size_t alignment)
{
    Page* p = pool->head;
    plnnrc_assert(sizeof(Page) + plnnrc_alignof(Page) + bytes + alignment < pool->page_size);

    uint8_t* top = static_cast<uint8_t*>(plnnrc::align(p->top, alignment));

    if (top + bytes > p->memory + pool->page_size)
    {
        Page* p = allocate_page(pool);
        top = static_cast<uint8_t*>(plnnrc::align(p->top, alignment));
    }

    p->top = top + bytes;

    return top;
}

void plnnrc::destroy(const Pool* pool)
{
    for (Page* p = pool->head; p != 0;)
    {
        Page* n = p->prev;
        plnnrc::deallocate(p->memory);
        p = n;
    }
}
