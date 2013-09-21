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

struct page
{
    page* prev;
    char* memory;
    char* top;
    char  data[1];
};

struct handle
{
    page* head_page;
    void* head_tuple;
    tuple_traits tuple;
    size_t page_size;
};

handle* create(tuple_traits traits, size_t tuples_per_page)
{
    size_t head_page_size =
        sizeof(handle) + plnnr_alignof(handle) +
        sizeof(page) + plnnr_alignof(page) +
        traits.alignment + traits.size * tuples_per_page;

    char* memory = static_cast<char*>(memory::allocate(head_page_size));

    if (!memory)
    {
        return 0;
    }

    size_t page_size =
        sizeof(page) + plnnr_alignof(page) + traits.alignment + traits.size * tuples_per_page;

    handle* tuple_list = memory::align<handle>(memory);
    page* head_page = memory::align<page>(tuple_list + 1);

    head_page->prev = 0;
    head_page->memory = memory;
    head_page->top = head_page->data;

    tuple_list->head_page = head_page;
    tuple_list->head_tuple = 0;
    tuple_list->tuple = traits;
    tuple_list->page_size = page_size;

    return tuple_list;
}

void destroy(const handle* tuple_list)
{
    for (page* p = tuple_list->head_page; p != 0;)
    {
        page* n = p->prev;
        memory::deallocate(p->memory);
        p = n;
    }
}

handle* head_to_handle(void* head)
{
    return reinterpret_cast<handle*>(static_cast<char*>(head) - offsetof(handle, head_tuple));
}

}
}
