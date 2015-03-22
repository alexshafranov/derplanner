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

#ifndef DERPLANNER_COMPILER_POOL_H_
#define DERPLANNER_COMPILER_POOL_H_

#include <stddef.h> // size_t

namespace plnnrc {

///
/// Paged Pool
///
/// Chain of uniformly sized blocks of memory, where each block is used for a linear `bump-pointer` allocations.
/// There's no `deallocate` function as the pool can only grow. All pages are freed at once in `destroy`.
///

struct Pool_Handle;

Pool_Handle* create_paged_pool(size_t page_size);

void destroy(const Pool_Handle* handle);

void* allocate(Pool_Handle* handle, size_t bytes, size_t alignment);

}

#endif
