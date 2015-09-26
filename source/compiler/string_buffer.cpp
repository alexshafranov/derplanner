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

#include "derplanner/compiler/string_buffer.h"

void plnnrc::init(plnnrc::String_Buffer& buffer, Memory* mem, uint32_t max_size, uint32_t max_chars)
{
    plnnrc::init(buffer.buffer, mem, max_chars);
    plnnrc::init(buffer.offsets, mem, max_size);
    plnnrc::init(buffer.lengths, mem, max_size);
}

void plnnrc::destroy(plnnrc::String_Buffer& buffer)
{
    plnnrc::destroy(buffer.lengths);
    plnnrc::destroy(buffer.offsets);
    plnnrc::destroy(buffer.buffer);
}

void plnnrc::push_back(plnnrc::String_Buffer& buffer, const char* str)
{
    const uint32_t length = (uint32_t)strlen(str);
    plnnrc::Token_Value token = { length, str };
    plnnrc::push_back(buffer, token);
}

void plnnrc::push_back(plnnrc::String_Buffer& buffer, const plnnrc::Token_Value& token)
{
    const uint32_t offset = plnnrc::size(buffer.buffer);
    plnnrc::push_back(buffer.buffer, token.str, token.length);
    plnnrc::push_back(buffer.offsets, offset);
    plnnrc::push_back(buffer.lengths, token.length);
}

plnnrc::Token_Value plnnrc::get(const String_Buffer& buffer, uint32_t index)
{
    const uint32_t offset = buffer.offsets[index];
    const uint32_t length = buffer.lengths[index];
    const char* str = &buffer.buffer[offset];
    const plnnrc::Token_Value token = { length, str };
    return token;
}

uint32_t plnnrc::size(const plnnrc::String_Buffer& buffer)
{
    return plnnrc::size(buffer.lengths);
}

void plnnrc::begin_string(plnnrc::String_Buffer& buffer)
{
    const uint32_t offset = plnnrc::size(buffer.buffer);
    plnnrc::push_back(buffer.offsets, offset);
}

void plnnrc::put_chars(plnnrc::String_Buffer& buffer, const char* chars, uint32_t count)
{
    plnnrc::push_back(buffer.buffer, chars, count);
}

void plnnrc::end_string(plnnrc::String_Buffer& buffer)
{
    const uint32_t length = plnnrc::size(buffer.buffer) - plnnrc::back(buffer.offsets);
    plnnrc::push_back(buffer.lengths, length);
}
