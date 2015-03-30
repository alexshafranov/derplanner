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
void init(Array<T>& array, uint32_t max_size);

template <typename T>
void destroy(Array<T>& array);

template <typename T>
void resize(Array<T>& array, uint32_t new_size);

template <typename T>
void grow(Array<T>& array, uint32_t new_max_size);

template <typename T>
void push_back(Array<T>& array, const T& value);

template <typename T>
uint32_t size(const Array<T>& array);

}

template <typename T>
void plnnrc::init(plnnrc::Array<T>& result, uint32_t max_size)
{
    result.size = 0;
    result.max_size = max_size;
    result.data = static_cast<T*>(plnnrc::allocate(sizeof(T) * max_size));
}

template <typename T>
inline void plnnrc::destroy(Array<T>& array)
{
    plnnrc::deallocate(array.data);
    array.size = 0;
    array.max_size = 0;
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
    plnnrc_assert(array.max_size < new_max_size);
    T* new_data = static_cast<T*>(plnnrc::allocate(sizeof(T) * new_max_size));
    memcpy(new_data, array.data, sizeof(T) * array.size);
    plnnrc::deallocate(array.data);
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

    array.data[array.size++] = value;
}

template <typename T>
inline T& plnnrc::Array<T>::operator[](uint32_t index)
{
    plnnrc_assert(index < size);
    return data[index];
}

template <typename T>
inline const T& plnnrc::Array<T>::operator[](uint32_t index) const
{
    plnnrc_assert(index < size);
    return data[index];
}

template <typename T>
inline uint32_t plnnrc::size(const plnnrc::Array<T>& array)
{
    return array.size;
}

#endif
