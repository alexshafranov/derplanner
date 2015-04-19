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

#ifndef DERPLANNER_COMPILER_MEMORY_H_
#define DERPLANNER_COMPILER_MEMORY_H_

#include <stddef.h> // for size_t
#include <stdint.h> // for uintptr_t

#ifndef plnnrc_alignof
    #define plnnrc_alignof(T) __alignof(T)
#endif

namespace plnnrc {

enum { default_alignment = 16 };

typedef void* Allocate(size_t bytes);
typedef void  Deallocate(void* ptr);

// allows global override of derplanner memory allocation.
void set_memory_functions(Allocate* allocate_function, Deallocate* deallocate_function);

// allocates a `bytes` sized memory block.
void* allocate(size_t bytes);

// deallocates memory block pointed previously obtained by `allocate`.
void deallocate(void* ptr);

// Allocator interface.
class Memory
{
public:
    virtual ~Memory() {}
    virtual void* allocate(size_t size, size_t alignment=default_alignment) = 0;
    virtual void  deallocate(void* ptr) = 0;
};

// Default allocator, uses `plnnrc::allocate` and `plnnrc::deallocate`.
class Memory_Default : public Memory
{
public:
    virtual void* allocate(size_t size, size_t alignment=default_alignment);
    virtual void  deallocate(void* ptr);
};

// returns static `Memory_Default` instance.
Memory* get_default_allocator();

// allocate an array of type `T` using specified alignment.
template <typename T>
inline T* allocate(Memory* mem, size_t count, size_t alignment)
{
    return static_cast<T*>(mem->allocate(count*sizeof(T), alignment));
}

// allocate an array of type `T` with compiler specified alignment for type `T`.
template <typename T>
inline T* allocate(Memory* mem, size_t count)
{
    return allocate<T>(mem, count, plnnrc_alignof(T));
}

// align size.
inline size_t align(size_t value, size_t alignment)
{
    return (value + (alignment - 1)) & ~(alignment - 1);
}

// align pointer.
inline void* align(void* ptr, size_t alignment)
{
    return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(ptr) + (alignment - 1)) & ~(alignment - 1));
}

// align pointer to compiler specified alignment for type `T` and cast.
template <typename T>
inline T* align(void* ptr)
{
    return static_cast<T*>(align(ptr, plnnrc_alignof(T)));
}

}

#endif
