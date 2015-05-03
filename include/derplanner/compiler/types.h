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

// Memory allocator base interface.
class Memory;

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
    Memory*     memory;
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
    Memory*         memory;
};

// A bunch of strings stored in a single buffer.
struct String_Buffer
{
    // linear string storage, trailing zeros are excluded.
    Array<char>     buffer;
    // string start offsets in `buffer`.
    Array<uint32_t> offsets;
    // length of each string.
    Array<uint32_t> lengths;
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

// Token group ranges: <Group>_First, <Group>_Last
enum Token_Groups
{
    #define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG) Token_Group_##GROUP_TAG##_First = Token_##FIRST_TOKEN_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN_GROUP

    #define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG) Token_Group_##GROUP_TAG##_Last = Token_##LAST_TOKEN_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN_GROUP
};

// Reference to signature (tuple of types) stored `Signature_Table`.
struct Signature
{
    // type of each parameter.
    const Token_Type*   types;
    // number of types.
    uint32_t            length;
    // offset in types array in `Signature_Table`.
    uint32_t            offset;
};

// Collection of hashed signatures (type tuples).
struct Signature_Table
{
    // compacted signature types.
    Array<Token_Type>   types;
    // hash of each signature.
    Array<uint32_t>     hashes;
    // offset to `types` for each signature.
    Array<uint32_t>     offsets;
    // length (number of params) for each signature.
    Array<uint32_t>     lengths;
    // maps signature index to index into `hashes`, `offsets` & `lengths` arrays.
    Array<uint32_t>     remap;
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
    // allocator used for lexer memory.
    Memory*                 memory;
};

/// Abstract-Syntax-Tree nodes, produced by the parser.

namespace ast
{
    struct Root;
        struct Node;
            struct World;
                struct Fact;
                    struct Data_Type;
            struct Primitive;
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
        Node_Type           type;
    };

    // Root of the Abstract-Syntax-Tree.
    struct Root
    {
        Root();
        ~Root();

        // maps fact name -> Fact node.
        Id_Table<Fact*>     fact_lookup;
        // maps task name -> Task node.
        Id_Table<Task*>     task_lookup;
        // maps primitive task name -> Fact node.
        Id_Table<Fact*>     primitive_lookup;
        // all cases in the order of definition.
        Array<Case*>        cases;
        // parsed `world` block.
        World*              world;
        // parsed `primitive` block.
        Primitive*          primitive;
        // parsed `domain` block.
        Domain*             domain;
        // paged memory pool ast nodes are allocated from.
        Memory*             pool;
    };

    // Parsed `world` block.
    struct World : public Node
    {
        // fact declarations.
        Array<Fact*>        facts;
    };

    // Parsed `primitive` block.
    struct Primitive : public Node
    {
        Array<Fact*>        tasks;
    };

    // Parsed `domain` block.
    struct Domain : public Node
    {
        // domain name.
        Token_Value         name;
        // tasks.
        Array<Task*>        tasks;
    };

    // Fact: Id + Parameters.
    struct Fact : public Node
    {
        // name of the fact.
        Token_Value         name;
        // parameters.
        Array<Data_Type*>   params;
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
        Array<Param*>       params;
        // expansion cases.
        Array<Case*>        cases;
        // param name -> node.
        Id_Table<Param*>    param_lookup;
    };

    // Parsed `case` block.
    struct Case : public Node
    {
        // a pointer to the task this case is part of.
        Task*               task;
        // precondition.
        Expr*               precond;
        // task list expressions.
        Array<Expr*>        task_list;
        // precondition variable name -> first occurence.
        Id_Table<ast::Var*> precond_var_lookup;
        // array of all vars in precondition.
        Array<ast::Var*>    precond_vars;
        // task list variable name -> first occurence.
        Id_Table<ast::Var*> task_list_var_lookup;
        // array of all vars in task list.
        Array<ast::Var*>    task_list_vars;
        // ast::Fact for each ast::Func instance. 
        Array<ast::Fact*>   precond_facts;
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
        // `ast::Param` or `ast::Var` when this variable was first bound (i.e. the first occurence in expression)
        Node*               definition;
        // inferred data type for this variable.
        Token_Type          data_type;

        union
        {
            // index of the variable in the input signature.
            uint32_t        input_index;
            // index of the variable in the output signature.
            uint32_t        output_index;
        };
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
    // temporary storage for created AST nodes.
    Array<ast::Node*>   scratch;
    // allocator used for parsed data.
    Memory*             memory;
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

// Code generator state.
struct Codegen
{
    Codegen();
    ~Codegen();

    // input AST.
    ast::Root*          tree;
    // formatter used to write files.
    Formatter           fmtr;
    // paged pool for codegen data.
    Memory*             pool;

    // expand function names `<composite_task_name>_case_<case_index>`, in order of definition.
    String_Buffer       expand_names;
    // signatures in the following order: primitive, composite, precondition outputs.
    Signature_Table     task_and_pout_sigs;
    // precondition input signatures.
    Signature_Table     pin_sigs;
};

}

#endif
