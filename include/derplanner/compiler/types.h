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

// Dynamic array of POD types.
template <typename T>
struct Array
{
    Array();
    ~Array();

    T&          operator[](uint32_t index);
    const T&    operator[](uint32_t index) const;

    uint32_t    size;
    uint32_t    max_size;
    T*          data;
};

// Dynamic open-addressing hash table, maps constant C-strings to POD values.
template <typename T>
struct Id_Table
{
    Id_Table();
    ~Id_Table();

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

// Token type tags.
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
    Token_Value();

    // number of characters in the string.
    uint32_t        length;
    // pointer to the beginning of the string in an input buffer.
    const char*     str;
};

// Token data returned by the lexer.
struct Token
{
    Token();

    // type of the token.
    Token_Type      type;
    // input buffer line.
    uint32_t        line;
    // input buffer column.
    uint32_t        column;
    // string value of the token.
    Token_Value     value;
};

// Keeps track of lexer progress through the input buffer.
struct Lexer
{
    Lexer();
    ~Lexer();

    // points to the first character in input buffer.
    const char*             buffer_start;
    // points to the next character to be lexed.
    const char*             buffer_ptr;
    // current column.
    uint32_t                column;
    // current line.
    uint32_t                line;
    // maps keyword names to keyword types.
    Id_Table<Token_Type>    keywords;
};

// Paged pool handle.
struct Pool;
// Visitor interface
class Visitor;

/// Abstract-Syntax-Tree nodes, produced by the parser.

namespace ast
{
    struct Root;
        struct Node;
            struct World;
                struct Fact;
                    struct Data_Type;
            struct Domain;
                struct Task;
                    struct Param;
                    struct Case;
                        struct Expr;
                            struct Var;
                            struct Func;
                            struct Op;
                            struct Literal;

    // Node type tags.
    enum Node_Type
    {
        Node_None = 0,
        #define PLNNRC_NODE(TAG, TYPE) Node_##TAG,
        #include "derplanner/compiler/ast_tags.inl"
        #undef PLNNRC_NODE
        Node_Count,
    };

    // AST base node.
    struct Node
    {
        // type tag of the node.
        Node_Type       type;
    };

    // Array of node pointers.
    template <typename T>
    struct Nodes
    {
        uint32_t        size;
        T**             array;

        T* operator[](uint32_t index);
        const T* operator[](uint32_t index) const;
    };

    // Root of the Abstract-Syntax-Tree.
    struct Root
    {
        Root();
        ~Root();

        // maps fact name -> Fact node.
        Id_Table<Fact*> fact_lookup;
        // maps task name -> Task node.
        Id_Table<Task*> task_lookup;
        // parsed `world` block.
        World*          world;
        // parsed `domain` block.
        Domain*         domain;
        // paged memory pool ast nodes are allocated from.
        Pool*           pool;
    };

    // Parsed `world` block.
    struct World : public Node
    {
        // fact declarations.
        Nodes<Fact>         facts;
    };

    // Parsed `domain` block.
    struct Domain : public Node
    {
        // domain name.
        Token_Value         name;
        // tasks.
        Nodes<Task>         tasks;
    };

    // Fact: Id + Parameters.
    struct Fact : public Node
    {
        // name of the fact.
        Token_Value         name;
        // parameters.
        Nodes<Data_Type>    params;
    };

    // Fact parameter type.
    struct Data_Type : public Node
    {
        // must be one of the type tokens.
        Token_Type          data_type;
    };

    // Parsed `task` block.
    struct Task : public Node
    {
        // name of the task.
        Token_Value         name;
        // task parameters.
        Nodes<Param>        params;
        // expansion cases.
        Nodes<Case>         cases;
    };

    // Parsed `case` block.
    struct Case : public Node
    {
        // precondition.
        Expr*               precond;
        // task list expressions.
        Nodes<Expr>         task_list;
    };

    // Parameter: Id + Data type.
    struct Param : public Node
    {
        // name of the parameter.
        Token_Value         name;
        // inferred or defined data type (one of the type tokens).
        Token_Type          data_type;
    };

    // Parsed precondition expression or task list item.
    struct Expr : public Node
    {
        // parent node.
        Expr*               parent;
        // first child node.
        Expr*               child;
        // next sibling.
        Expr*               next_sibling;
        // previous sibling (forms cyclic list).
        Expr*               prev_sibling_cyclic;
    };

    // Operator, used in precondition expression.
    struct Op : public Expr {};

    // Variable, used in precondition and task list expressions.
    struct Var : public Expr
    {
        // name of the variable.
        Token_Value         name;
        // first occurence of this variable, could be ast::Param or ast::Var.
        Node*               definition;
        // inferred data type for this variable.
        Token_Type          data_type;
    };

    // Literal.
    struct Literal : public Expr
    {
        // string value of the literal.
        Token_Value         value;
        // type of the literal.
        Token_Type          data_type;
    };

    // Fact/function/task used in precondtition or task list.
    struct Func : public Expr
    {
        // name of the fact/function/task.
        Token_Value         name;
    };
}

// Parser state.
struct Parser
{
    Parser();
    ~Parser();

    // token source for parsing.
    Lexer*              lexer;
    // last lexed token.
    Token               token;
    // stores resulting AST.
    ast::Root           tree;
    // scratch memory to store node pointers.
    Array<ast::Node*>   scratch;
};

class Writer;

// Buffered output stream, supporting formatting operations.
struct Formatter
{
    Formatter();
    ~Formatter();

    enum { Output_Buffer_Size = 4096 };

    // number of `tab` symbols to put when a new line starts.
    uint32_t        indent;
    // constant string used for virtual tab symbol (e.g. 4 spaces).
    const char*     tab;
    // constant string used to start a new line.
    const char*     newline;
    // embedded output buffer.
    uint8_t         buffer[Output_Buffer_Size];
    // current position in output buffer.
    uint8_t*        buffer_ptr;
    // end of the output buffer, `buffer` <= `buffer_ptr` <= `buffer_end`.
    uint8_t*        buffer_end;
    // output interface used to flush buffer.
    Writer*         output;
};

}

#endif
