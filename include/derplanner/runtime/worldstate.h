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

#ifndef DERPLANNER_RUNTIME_WORLDSTATE_H_
#define DERPLANNER_RUNTIME_WORLDSTATE_H_

#include <stddef.h> // size_t, offsetof
#include <derplanner/runtime/memory.h> // plnnr_alignof

namespace plnnr {
namespace tuple_list {

struct tuple_traits
{
    size_t size;
    size_t alignment;
    size_t parent_offset;
    size_t next_offset;
    size_t prev_offset;
};

struct handle;

handle* create(void** head, tuple_traits traits, size_t tuples_per_page);

void destroy(const handle* tuple_list);

void* effect_add(handle* tuple_list);

void effect_delete(handle* tuple_list, void* tuple);

void effect_rollback(handle* tuple_list, void* tuple);

template <typename T>
inline handle* create(T** head, size_t tuples_per_page)
{
    tuple_traits traits;
    traits.size = sizeof(T);
    traits.alignment = plnnr_alignof(T);
    traits.parent_offset = offsetof(T, parent);
    traits.next_offset = offsetof(T, next);
    traits.prev_offset = offsetof(T, prev);
    return create(reinterpret_cast<void**>(head), traits, tuples_per_page);
}

template <typename T>
inline handle* head_to_handle(T* head)
{
    return static_cast<handle*>(head->parent);
}

template <typename T>
inline T* effect_add(handle* tuple_list)
{
    return static_cast<T*>(effect_add(tuple_list));
}

}
}

#endif
