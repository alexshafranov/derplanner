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
#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/pool.h"

namespace plnnrc
{
    struct Page
    {
        size_t      size;
        Page*       prev;
        uint8_t*    blob;
        uint8_t*    top;
        uint8_t     data[1];
    };

    class Pool : public Memory
    {
    public:
        virtual void* allocate(size_t size, size_t alignment);
        virtual void  deallocate(void* ptr);

        size_t  page_size;
        size_t  total_requested;
        size_t  total_allocated;
        Page*   head;
    };
}

using namespace plnnrc;

static const size_t bookkeeping_size = sizeof(Pool) + plnnrc_alignof(Pool) + sizeof(Page) + plnnrc_alignof(Page);

plnnrc::Memory* plnnrc::create_paged_pool(size_t page_size)
{
    plnnrc_assert(page_size > bookkeeping_size);

    uint8_t* blob = static_cast<uint8_t*>(plnnrc::allocate(page_size));
    plnnrc_assert(blob != 0);

    // create Pool & Page on the blob itself.
    plnnrc::Pool* pool = new(plnnrc::align<plnnrc::Pool>(blob)) Pool;
    plnnrc::Page* head = plnnrc::align<plnnrc::Page>(pool + 1);

    head->size = page_size;
    head->prev = 0;
    head->blob = blob;
    head->top = head->data;

    pool->head = head;
    pool->page_size = page_size;
    pool->total_requested = 0;
    pool->total_allocated = page_size;

    return pool;
}

void plnnrc::destroy(Memory* handle)
{
    plnnrc::Pool* pool = static_cast<plnnrc::Pool*>(handle);

    for (Page* p = pool->head; p != 0;)
    {
        Page* n = p->prev;
        plnnrc::deallocate(p->blob);
        p = n;
    }
}

static plnnrc::Page* allocate_page(plnnrc::Pool* pool, size_t size)
{
    uint8_t* blob = static_cast<uint8_t*>(plnnrc::allocate(size));
    plnnrc_assert(blob != 0);
    pool->total_allocated += size;

    plnnrc::Page* page = plnnrc::align<plnnrc::Page>(blob);
    page->size = size;
    page->prev = pool->head;
    page->blob = blob;
    page->top = page->data;
    pool->head = page;

    return page;
}

void* plnnrc::Pool::allocate(size_t size, size_t alignment)
{
    Page* p = head;
    uint8_t* top = static_cast<uint8_t*>(plnnrc::align(p->top, alignment));

    if (top + size > p->blob + p->size)
    {
        size_t new_page_size = page_size;
        if (size + alignment + bookkeeping_size > new_page_size)
        {
            new_page_size = size + alignment + bookkeeping_size;
        }

        p = allocate_page(this, new_page_size);
        top = static_cast<uint8_t*>(plnnrc::align(p->top, alignment));
    }

    p->top = top + size;
    total_requested += size;

    return top;
}

void plnnrc::Pool::deallocate(void*) { /* this allocator deallocates en masse */ }

size_t plnnrc::get_total_allocated(const plnnrc::Memory* handle)
{
    const plnnrc::Pool* pool = static_cast<const plnnrc::Pool*>(handle);
    return pool->total_allocated;
}

size_t plnnrc::get_total_requested(const plnnrc::Memory* handle)
{
    const plnnrc::Pool* pool = static_cast<const plnnrc::Pool*>(handle);
    return pool->total_requested;
}
