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

#include "derplanner/compiler/s_expression.h"
#include <stdlib.h>

namespace derplanner {
namespace s_expression {

static const int chunk_node_count = 2048;

struct linear_memory_chunk
{
    linear_memory_chunk* next;
    int top;
    node data[chunk_node_count];
};

static void free_allocated_chunks(linear_memory_chunk* root)
{
    for (linear_memory_chunk* chunk = root; chunk != 0;)
    {
        linear_memory_chunk* next = chunk->next;
        free(chunk);
        chunk = next;
    }
}

static node* allocate_node(void*& memory)
{
    linear_memory_chunk* chunk = reinterpret_cast<linear_memory_chunk*>(memory);

    if (chunk->top >= chunk_node_count)
    {
        linear_memory_chunk* new_chunk = reinterpret_cast<linear_memory_chunk*>(malloc(sizeof(linear_memory_chunk)));

        new_chunk->next = chunk;
        new_chunk->top = 0;

        chunk = new_chunk;
        memory = new_chunk;
    }

    chunk->top++;

    return chunk->data + chunk->top;
}

tree::tree()
    : root(0)
    , _memory(0)
{
}

tree::~tree()
{
    if (_memory)
    {
        free_allocated_chunks(reinterpret_cast<linear_memory_chunk*>(_memory));
    }
}

void tree::parse(const char* text)
{
    if (_memory)
    {
        free_allocated_chunks(reinterpret_cast<linear_memory_chunk*>(_memory));
        _memory = 0;
        root = 0;
    }

    root = allocate_node(_memory);
}

}
}
