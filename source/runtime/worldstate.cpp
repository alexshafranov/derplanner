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
    size_t tuple_size;
    size_t tuple_alignment;
    size_t tuples_per_page;
};

handle* create(size_t tuple_size, size_t tuple_alignment, size_t tuples_per_page)
{
    size_t page_size =
        sizeof(handle) + plnnr_alignof(handle) +
        sizeof(page) + plnnr_alignof(page) +
        tuple_alignment + tuple_size * tuples_per_page;

    char* memory = static_cast<char*>(memory::allocate(page_size));

    if (!memory)
    {
        return 0;
    }

    handle* tuple_list = memory::align<handle>(memory);
    page* head_page = memory::align<page>(tuple_list + 1);

    head_page->prev = 0;
    head_page->memory = memory;
    head_page->top = head_page->data;

    tuple_list->head_page = head_page;
    tuple_list->head_tuple = 0;
    tuple_list->tuple_size = tuple_size;
    tuple_list->tuple_alignment = tuple_alignment;
    tuple_list->tuples_per_page = tuples_per_page;

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

}
}
