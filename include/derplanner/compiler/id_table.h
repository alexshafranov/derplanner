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

#ifndef DERPLANNER_COMPILER_ID_TABLE_H_
#define DERPLANNER_COMPILER_ID_TABLE_H_

#include <math.h>
#include <string.h>
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/types.h"
#include "derplanner/compiler/array.h"

namespace plnnrc {

/// Id_Table

// initialize `Id_Table`.
template <typename T>
void init(Id_Table<T>& table, Memory* mem, uint32_t max_size);

// destroys id table.
void destroy(Id_Table_Base& table);

// set value for the key, returns 'true' if the key is alredy present in the table.
template <typename T>
bool set(Id_Table<T>& table, const char* key, const T& value);

template <typename T>
bool set(Id_Table<T>& table, const Token_Value& token_value, const T& value);

template <typename T>
bool set(Id_Table<T>& table, const char* key, uint32_t length, const T& value);

// clears the table, memory is not deallocated.
template <typename T>
void clear(Id_Table<T>& table);

// returns the number of entries in the table.
template <typename T>
uint32_t size(const Id_Table<T>& table);

// returns pointer to value for the given key or null if it doesn't exist.
template <typename T>
T* get(const Id_Table<T>& table, const char* key);

template <typename T>
T* get(const Id_Table<T>& table, const Token_Value& token_value);

template <typename T>
T* get(const Id_Table<T>& table, const char* key, uint32_t length);

/// `get` specializations for `T*`

template <typename T>
T* get(const Id_Table<T*>& table, const char* key);

template <typename T>
T* get(const Id_Table<T*>& table, const Token_Value& token_value);

template <typename T>
T* get(const Id_Table<T*>& table, const char* key, uint32_t length);

// copy all values from the table to an array.
template <typename T>
void values(const Id_Table<T>& table, Array<T>& output_values);

}

#include "derplanner/compiler/id_table.inl"

#endif
