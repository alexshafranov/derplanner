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

#ifndef DERPLANNER_COMPILER_TYPES_H_
#define DERPLANNER_COMPILER_TYPES_H_

#include <stdint.h>

namespace plnnrc {

// Array of POD types.
template <typename T>
struct Array
{
    T& operator[](uint32_t index);
    const T& operator[](uint32_t index) const;

    uint32_t    size;
    uint32_t    max_size;
    T*          data;
};

// Open-addressing hash table, mapping constant strings to POD values.
template <typename T>
struct Id_Table
{
    uint32_t        size;
    uint32_t        max_size;
    uint32_t*       hashes;
    const char**    keys;
    uint32_t*       lengths;
    T*              values;
};

// Error/Warning IDs.
enum Error_Type
{
    Error_None = 0,
    #define PLNNRC_ERROR(TAG) Error_##TAG,
    #include "derplanner/compiler/error_tags.inl"
    #undef PLNNRC_ERROR
};

// Token identifiers.
enum Token_Type
{
    Token_Unknown = 0,
    #define PLNNRC_TOKEN(TAG) Token_##TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN
};

// Token data returned by the lexer.
struct Token
{
    // type of the token.
    Token_Type      type;
    // input buffer column.
    uint32_t        column;
    // input buffer line.
    uint32_t        line;
    // number of characters in the token.
    uint32_t        length;
    // points to the token first character.
    const char*     str;
};

// Error/Warning emitted by lexer.
struct Lexer_Error
{
    Error_Type type;
};

// Keeps track of lexer progress through the input buffer.
struct Lexer_State
{
    // points to the first character in input buffer.
    const char*             buffer_start;
    // points to the next character to be lexed.
    const char*             buffer_ptr;
    // current column.
    uint32_t                column;
    // current line.
    uint32_t                line;
    // maps keyword strings to keyword types.
    Id_Table<Token_Type>    keywords;
    // emitted errors & warnings.
    Array<Lexer_Error>      errors;
};

// RAII destruction.
template <typename T>
struct Scoped : public T
{
    inline ~Scoped()
    {
        destroy(*this);
    }
};

}

#endif
