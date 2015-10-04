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

#include <stdlib.h>
#include "derplanner/runtime/memory.h"

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

    plnnr::Allocate*   alloc_f   = default_alloc;
    plnnr::Deallocate* dealloc_f = default_dealloc;
}

void plnnr::set_memory_functions(Allocate* a, Deallocate* f)
{
    alloc_f = a;
    dealloc_f = f;
}

void* plnnr::allocate(size_t size)
{
    plnnr_assert(alloc_f != 0);
    return alloc_f(size);
}

void plnnr::deallocate(void* ptr)
{
    plnnr_assert(dealloc_f != 0);
    dealloc_f(ptr);
}

void* plnnr::Memory_Default::allocate(size_t size, size_t alignment)
{
    alignment = (alignment < sizeof(uint32_t)) ? sizeof(uint32_t) : alignment;
    void* p = plnnr::allocate(sizeof(uint32_t) + alignment + size);

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

void plnnr::Memory_Default::deallocate(void* ptr)
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

    plnnr::deallocate(p);
}
