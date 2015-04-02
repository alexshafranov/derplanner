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
    Error_Count,
};

// Token identifiers.
enum Token_Type
{
    Token_Unknown = 0,
    #define PLNNRC_TOKEN(TAG) Token_##TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN
    Token_Count,
};

// String value of the token.
struct Token_Value
{
    // number of characters in the string.
    uint32_t        length;
    // pointer to the beginning of the string in an input buffer.
    const char*     str;
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
    // string value of the token.
    Token_Value     value;
};

// Keeps track of lexer progress through the input buffer.
struct Lexer
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
};

/// Abstract-Syntax-Tree nodes, produced by the parser.
namespace ast
{
    struct World;
        struct Fact_Type;

    struct Domain;
        struct Task;
            struct Case;
                struct Expr;

    // Database fact type declaration in `world` block.
    struct Fact_Type
    {
        // name of the fact.
        Token_Value     name;
        // number of parameters.
        uint32_t        num_params;
        // parameter types. (one of the type tokens).
        Token_Type*     param_types;
    };

    // Parsed `world` block.
    struct World
    {
        // number of fact types declared.
        uint32_t        num_facts;
        // fact type declarations.
        Fact_Type**     facts;
    };

    // Parsed `task` block.
    struct Task
    {
        // name of the task.
        Token_Value     name;
        // number of cases in this task. 
        uint32_t        num_cases;
        // number of parameters.
        uint32_t        num_params;
        // name of each parameter.
        Token_Value*    param_names;
        // inferred parameter types.
        Token_Type*     param_types;
        // task case definitions.
        Case**          cases;
    };

    // Parsed case precondition/task-list expression.
    struct Expr
    {
        // could be operation or literal or fact/variable identifier.
        Token_Type      type;
        // literal or identifier or operation name.
        Token_Value     value;
        // inferred type for variables.
        Token_Type      inferred;
        // parent node.
        Expr*           parent;
        // first child node.
        Expr*           child;
        // next sibling.
        Expr*           next_sibling;
        // previous sibling (forms cyclic list).
        Expr*           prev_sibling_cyclic;
    };

    // Parsed `case`.
    struct Case
    {
        // precondition expression.
        Expr*           precond;
        // task list expression.
        uint32_t        num_tasks;
        // expression node for each task in task list.
        Expr**          task_list;
    };
}

struct Pool_Handle;

// Parser state.
struct Parser
{
    // maps fact name -> index
    Id_Table<uint32_t>  fact_ids;
    // maps task name -> index.
    Id_Table<uint32_t>  task_ids;
    // parsed `world`.
    ast::World*         world;
    // parsed `domain`.
    ast::Domain*        domain;
    // token source for parsing.
    Lexer*              lexer;
    // memory pool `ast::*` types are allocated from.
    Pool_Handle*        pool;
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
