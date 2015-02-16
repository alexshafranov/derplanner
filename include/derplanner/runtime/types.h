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

#ifndef DERPLANNER_RUNTIME_TYPES_H_
#define DERPLANNER_RUNTIME_TYPES_H_

#include <stdint.h>

namespace plnnr {

enum Type
{
    Type_None = 0,
    #define PLNNR_TYPE(TYPE_TAG, TYPE_NAME) Type_##TYPE_TAG,
    #include "derplanner/runtime/type_tags.inl"
    #undef PLNNR_TYPE
    Type_Count,
};

enum { Max_Fact_Arity = 16 };

struct Fact_Type
{
    uint8_t arity;
    Type param_type[Max_Fact_Arity];
};

// Points to a a single fact instance in Fact_Table
struct Fact_Handle
{
    uint32_t index;
    uint32_t generation;
};

//
// Fact_Name(P00, P01, ..., P0M)
// Fact_Name(P10, P11, ..., P1M)
// ...
// Fact_Name(PN0, PN1, ..., PNM)
//
// columns[i]  { P0i, P1i, ..., PNi }
//
struct Fact_Table
{
    uint32_t    fact_id;
    uint32_t    num_entries;
    uint32_t    max_entries;
    Fact_Type   format;
    void*       buffer;
    void*       columns[Max_Fact_Arity];
    uint32_t*   generations;
};

struct Fact_Database
{
    uint32_t    num_tables;
    Fact_Table* tables;
};

}

#endif
