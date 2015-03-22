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
#include "derplanner/compiler/assert.h"
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
