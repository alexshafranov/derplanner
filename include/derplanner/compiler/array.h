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

#ifndef DERPLANNER_COMPILER_ARRAY_H_
#define DERPLANNER_COMPILER_ARRAY_H_

#include <string.h>

#include "derplanner/compiler/assert.h"
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/types.h"

namespace plnnrc {

template <typename T>
void init(Array<T>& array, Memory* mem, uint32_t max_size);

void destroy(Array_Base& array);

template <typename T>
void resize(Array<T>& array, uint32_t new_size);

template <typename T>
void grow(Array<T>& array, uint32_t new_max_size);

template <typename T>
void push_back(Array<T>& array, const T& value);

template <typename T>
void push_back(Array<T>& array, const T* values, uint32_t count);

template <typename T>
void clear(Array<T>& array);

template <typename T>
uint32_t size(const Array<T>& array);

template <typename T>
T& back(Array<T>& array);

template <typename T>
const T& back(const Array<T>& array);

template <typename T>
uint32_t index_of(const Array<T>& array, const T& value);

template <typename T>
bool empty(const Array<T>& array);

}

template <typename T>
void plnnrc::init(plnnrc::Array<T>& result, Memory* mem, uint32_t max_size)
{
    result.size = 0;
    result.max_size = max_size;
    result.data = plnnrc::allocate<T>(mem, max_size);
    result.memory = mem;
}

inline void plnnrc::destroy(plnnrc::Array_Base& array)
{
    plnnrc::Memory* mem = array.memory;
    plnnrc_assert(mem != 0);
    mem->deallocate(array.data);
    array.size = 0;
    array.max_size = 0;
    array.memory = 0;
}

template <typename T>
inline void plnnrc::resize(plnnrc::Array<T>& array, uint32_t new_size)
{
    if (new_size > array.max_size)
    {
        plnnrc::grow(array, new_size);
    }

    array.size = new_size;
}

template <typename T>
inline void plnnrc::grow(plnnrc::Array<T>& array, uint32_t new_max_size)
{
    plnnrc::Memory* mem = array.memory;
    plnnrc_assert(mem != 0);
    plnnrc_assert(array.max_size < new_max_size);
    T* new_data = plnnrc::allocate<T>(mem, new_max_size);
    memcpy(new_data, array.data, sizeof(T) * array.size);
    mem->deallocate(array.data);
    array.data = new_data;
    array.max_size = new_max_size;
}

template <typename T>
inline void plnnrc::push_back(plnnrc::Array<T>& array, const T& value)
{
    if (array.size + 1 > array.max_size)
    {
        grow(array, array.max_size * 2 + 8);
    }

    T* data = static_cast<T*>(array.data);
    data[array.size++] = value;
}

template <typename T>
inline void plnnrc::push_back(plnnrc::Array<T>& array, const T* values, uint32_t count)
{
    if (array.size + count > array.max_size)
    {
        grow(array, (array.size + count) * 2);
    }

    T* data = static_cast<T*>(array.data);
    memcpy(data + array.size, values, sizeof(T)*count);
    array.size += count;
}

template <typename T>
inline T& plnnrc::Array<T>::operator[](uint32_t index)
{
    plnnrc_assert(index < size);
    return static_cast<T*>(data)[index];
}

template <typename T>
inline const T& plnnrc::Array<T>::operator[](uint32_t index) const
{
    plnnrc_assert(index < size);
    return static_cast<const T*>(data)[index];
}

template <typename T>
inline void plnnrc::clear(plnnrc::Array<T>& array)
{
    array.size = 0;
}

template <typename T>
inline uint32_t plnnrc::size(const plnnrc::Array<T>& array)
{
    return array.size;
}

template <typename T>
inline const T& plnnrc::back(const plnnrc::Array<T>& array)
{
    return array[array.size - 1];
}

template <typename T>
inline T& plnnrc::back(plnnrc::Array<T>& array)
{
    return array[array.size - 1];
}

template <typename T>
inline uint32_t plnnrc::index_of(const plnnrc::Array<T>& array, const T& value)
{
    for (uint32_t i = 0; i < size(array); ++i)
    {
        if (value == array[i])
        {
            return i;
        }
    }

    return size(array);
}

template <typename T>
inline bool plnnrc::empty(const plnnrc::Array<T>& array)
{
    return size(array) == 0;
}

#endif
