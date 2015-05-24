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

#ifndef DERPLANNER_COMPILER_STRING_BUFFER_H_
#define DERPLANNER_COMPILER_STRING_BUFFER_H_

#include <string.h>
#include "derplanner/compiler/types.h"
#include "derplanner/compiler/array.h"

namespace plnnrc {

// initialize `String_Buffer` with allocator `mem` and initial capacities of `max_chars` & `max_size`.
void init(String_Buffer& buffer, Memory* mem, uint32_t max_size, uint32_t max_chars);
// release `String_Buffer` memory.
void destroy(String_Buffer& buffer);

// adds a new string to the buffer.
void push_back(String_Buffer& buffer, const char* str);
// adds a new token to the buffer.
void push_back(String_Buffer& buffer, const Token_Value& token);

// returns the string at `index`.
Token_Value get(const String_Buffer& buffer, uint32_t index);

// number of strings stored.
uint32_t size(const String_Buffer& buffer);

// low-level string building
void begin_string(String_Buffer& buffer);
void put_chars(String_Buffer& buffer, const char* chars, uint32_t count);
void end_string(String_Buffer& buffer);

}

inline void plnnrc::init(plnnrc::String_Buffer& buffer, Memory* mem, uint32_t max_size, uint32_t max_chars)
{
    plnnrc::init(buffer.buffer, mem, max_chars);
    plnnrc::init(buffer.offsets, mem, max_size);
    plnnrc::init(buffer.lengths, mem, max_size);
}

inline void plnnrc::destroy(plnnrc::String_Buffer& buffer)
{
    plnnrc::destroy(buffer.lengths);
    plnnrc::destroy(buffer.offsets);
    plnnrc::destroy(buffer.buffer);
}

inline void plnnrc::push_back(plnnrc::String_Buffer& buffer, const char* str)
{
    uint32_t length = (uint32_t)strlen(str);
    plnnrc::Token_Value token = { length, str };
    plnnrc::push_back(buffer, token);
}

inline void plnnrc::push_back(plnnrc::String_Buffer& buffer, const plnnrc::Token_Value& token)
{
    uint32_t offset = plnnrc::size(buffer.buffer);
    plnnrc::push_back(buffer.buffer, token.str, token.length);
    plnnrc::push_back(buffer.offsets, offset);
    plnnrc::push_back(buffer.lengths, token.length);
}

inline plnnrc::Token_Value plnnrc::get(const String_Buffer& buffer, uint32_t index)
{
    uint32_t offset = buffer.offsets[index];
    uint32_t length = buffer.lengths[index];
    const char* str = &buffer.buffer[offset];
    plnnrc::Token_Value token = { length, str };
    return token;
}

inline uint32_t plnnrc::size(const plnnrc::String_Buffer& buffer) { return plnnrc::size(buffer.lengths); }

inline void plnnrc::begin_string(plnnrc::String_Buffer& buffer)
{
    uint32_t offset = plnnrc::size(buffer.buffer);
    plnnrc::push_back(buffer.offsets, offset);
}

inline void plnnrc::put_chars(plnnrc::String_Buffer& buffer, const char* chars, uint32_t count)
{
    plnnrc::push_back(buffer.buffer, chars, count);
}

inline void plnnrc::end_string(plnnrc::String_Buffer& buffer)
{
    uint32_t length = plnnrc::size(buffer.buffer) - plnnrc::back(buffer.offsets);
    plnnrc::push_back(buffer.lengths, length);
}

#endif
