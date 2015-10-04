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

#include "derplanner/compiler/base.h"

namespace plnnrc {

enum { default_alignment = 16 };

typedef void* Allocate(size_t bytes);
typedef void  Deallocate(void* ptr);

// allows global override of derplanner memory allocation.
void set_memory_functions(Allocate* allocate_function, Deallocate* deallocate_function);
// allocates a `bytes` sized memory block, using the globally set memory functions.
void* allocate(size_t bytes);
// deallocates memory block previously obtained with `allocate`, using the globally set memory functions.
void deallocate(void* ptr);

// Allocator interface.
class Memory
{
public:
    virtual ~Memory() {}
    virtual void* allocate(size_t size, size_t alignment = default_alignment) = 0;
    virtual void  deallocate(void* ptr) = 0;
};

// allocate an array of type `T` using specified alignment.
template <typename T>
T* allocate(Memory* mem, size_t count, size_t alignment);
// allocate an array of type `T` with compiler specified alignment for type `T`.
template <typename T>
T* allocate(Memory* mem, size_t count = 1);

// align size.
size_t align(size_t value, size_t alignment);
// align pointer.
void* align(void* ptr, size_t alignment);
// align pointer to compiler specified alignment for type `T` and cast.
template <typename T>
T* align(void* ptr);

// Default allocator, uses `plnnrc::allocate` and `plnnrc::deallocate`.
class Memory_Default : public Memory
{
public:
    virtual void* allocate(size_t size, size_t alignment = default_alignment);
    virtual void  deallocate(void* ptr);
};

// the static `Memory_Default` instance.
Memory* get_default_allocator();

struct Memory_Stack_Page;
struct Memory_Stack_Scope;

// Paged linear allocator.
class Memory_Stack : public Memory
{
public:
    static Memory_Stack*    create(size_t page_size);
    static void             destroy(Memory_Stack* mem);

    // `plnnrc::Memory` interface.
    virtual void* allocate(size_t size, size_t alignment=default_alignment);
    virtual void  deallocate(void*) { /* this allocator deallocates en masse */ }

    // release memory allocated after the `scope` was constructed.
    void pop(const Memory_Stack_Scope* scope);

    size_t get_total_allocated() const { return total_allocated; }
    size_t get_total_requested() const { return total_requested; }

private:
    friend struct Memory_Stack_Scope;

    plnnrc::Memory_Stack_Page*  allocate_page(size_t size);
    uint8_t*                    get_top();

    size_t              page_size;
    size_t              total_requested;
    size_t              total_allocated;
    Memory_Stack_Page*  head;
};

// Records the state of the `plnnrc::Memory_Stack` to call `plnnrc::Memory_Stack::pop` on destruction.
struct Memory_Stack_Scope
{
    Memory_Stack_Scope(Memory_Stack* stack);
    ~Memory_Stack_Scope();

    Memory_Stack*       stack;
    Memory_Stack_Page*  page;
    uint8_t*            top;
};

// RAII for `Memory_Stack`.
struct Memory_Stack_Context
{
    Memory_Stack_Context(size_t page_size)
    {
        mem = Memory_Stack::create(page_size);
    }

    ~Memory_Stack_Context()
    {
        Memory_Stack::destroy(mem);
    }

    Memory_Stack* mem;
};

}

/// Inline

template <typename T>
inline T* plnnrc::allocate(plnnrc::Memory* mem, size_t count, size_t alignment)
{
    return static_cast<T*>(mem->allocate(count*sizeof(T), alignment));
}

template <typename T>
inline T* plnnrc::allocate(plnnrc::Memory* mem, size_t count)
{
    return plnnrc::allocate<T>(mem, count, plnnrc_alignof(T));
}

inline size_t plnnrc::align(size_t value, size_t alignment)
{
    return (value + (alignment - 1)) & ~(alignment - 1);
}

inline void* plnnrc::align(void* ptr, size_t alignment)
{
    return reinterpret_cast<void*>((reinterpret_cast<uintptr_t>(ptr) + (alignment - 1)) & ~(alignment - 1));
}

template <typename T>
inline T* plnnrc::align(void* ptr)
{
    return static_cast<T*>(align(ptr, plnnrc_alignof(T)));
}

inline plnnrc::Memory_Stack_Scope::Memory_Stack_Scope(plnnrc::Memory_Stack* stack)
    : stack(stack)
    , page(stack->head)
    , top(stack->get_top())
{
}

inline plnnrc::Memory_Stack_Scope::~Memory_Stack_Scope()
{
    stack->pop(this);
}

#endif
