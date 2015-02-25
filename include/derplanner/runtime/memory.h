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

#ifndef DERPLANNER_RUNTIME_MEMORY_H_
#define DERPLANNER_RUNTIME_MEMORY_H_

#include <stddef.h> // for size_t
#include <stdint.h> // for uintptr_t

namespace plnnr {
namespace memory {

typedef void* (*Alloc_Func) (size_t size);
typedef void (*Dealloc_Func)(void* ptr);

void set_custom(Alloc_Func a, Dealloc_Func f);

void* allocate(size_t);
void deallocate(void*);

}
}

#ifndef plnnr_alignof
    #define plnnr_alignof(T) __alignof(T)
#endif

#ifndef PLNNR_DEFAULT_ALIGNMENT
    #define PLNNR_DEFAULT_ALIGNMENT (16)
#endif

class Memory
{
public:
    virtual ~Memory() {}
    virtual void* allocate(size_t size, size_t alignment=PLNNR_DEFAULT_ALIGNMENT)=0;
    virtual void  deallocate(void* ptr)=0;
};

template <typename T>
inline T* allocate(Memory* mem, size_t count, size_t alignment)
{
    return static_cast<T*>(mem->allocate(count*sizeof(T), alignment));
}

template <typename T>
inline T* allocate(Memory* mem, size_t count)
{
    return allocate<T>(count*sizeof(T), plnnr_alignof(T));
}

inline void* align(void* ptr, size_t alignment)
{
    return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(ptr) + (alignment - 1)) & ~(alignment - 1));
}

template <typename T>
inline T* align(void* ptr)
{
    return static_cast<T*>(align(ptr, plnnr_alignof(T)));
}

}

#endif
