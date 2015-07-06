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

// Linear allocator.
class Memory_Stack;

struct Array_Base
{
    Array_Base();
    ~Array_Base();

    uint32_t    size;
    uint32_t    max_size;
    void*       data;
    Memory*     memory;
};

// Dynamic array of POD types.
template <typename T>
struct Array : public Array_Base
{
    T&          operator[](uint32_t index);
    const T&    operator[](uint32_t index) const;
};

struct Id_Table_Base
{
    Id_Table_Base();
    ~Id_Table_Base();

    uint32_t        size;
    uint32_t        max_size;
    uint32_t*       hashes;
    const char**    keys;
    uint32_t*       lengths;
    void*           values;
    Memory*         memory;
};

// Dynamic open-addressing hash table, maps constant C-strings to POD values.
template <typename T>
struct Id_Table : Id_Table_Base
{
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
    #define PLNNRC_ERROR(TAG, FORMAT_STR) Error_##TAG,
    #include "derplanner/compiler/error_tags.inl"
    #undef PLNNRC_ERROR
    Error_Count,
};

// Token type tags.
enum Token_Type
{
    #define PLNNRC_TOKEN(TAG) Token_##TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN
    Token_Count,
};

// Token type groups.
enum Token_Group
{
    Token_Group_Unknown = 0,
    #define PLNNRC_TOKEN_GROUP(GROUP_TAG, FIRST_TOKEN_TAG, LAST_TOKEN_TAG) Token_Group_##GROUP_TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_TOKEN_GROUP
    Token_Group_Count,
};

// Token group ranges: <Group>_First, <Group>_Last
enum Token_Group_Ranges
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

// Location in the input buffer.
struct Location
{
    uint32_t        line;
    uint32_t        column;
};

// Token data returned by the lexer.
struct Token
{
    // indicates that parser has generated this token.
    uint8_t         error : 1;
    // type of the token.
    Token_Type      type;
    // location in the input buffer.
    Location        loc;
    // string value of the token.
    Token_Value     value;
};

// Emitted error information.
struct Error
{
    enum { Max_Args = 4 };

    // error format argument type.
    enum Arg_Type
    {
        Arg_Type_None = 0,
        Arg_Type_Token,
        Arg_Type_Token_Value,
        Arg_Type_Token_Type,
        Arg_Type_Token_Group,
    };

    // error format argument.
    union Arg
    {
        Token           token;
        Token_Value     token_value;
        Token_Type      token_type;
        Token_Group     token_group;
    };

    // type code of the error.
    Error_Type      type;
    // error location.
    Location        loc;
    // error format string.
    const char*     format;
    // number of arguments.
    uint8_t         num_args;
    // error format arguments.
    Arg             args[Max_Args];
    // error format argument types.
    Arg_Type        arg_types[Max_Args];
};

// Keeps track of lexer progress through the input buffer.
struct Lexer
{
    // points to the first character in input buffer.
    const char*             buffer_start;
    // points to the next character to be lexed.
    const char*             buffer_ptr;
    // current location.
    Location                loc;
    // maps keyword names to keyword types.
    Id_Table<Token_Type>    keywords;
    // allocator used for lexer data.
    Memory_Stack*           scratch;
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
            struct Const;
            struct Domain;
                struct Macro;
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
        Node_Type               type;
    };

    // Root of the Abstract-Syntax-Tree.
    struct Root
    {
        // maps fact name -> Fact node.
        Id_Table<Fact*>         fact_lookup;
        // maps task name -> Task node.
        Id_Table<Task*>         task_lookup;
        // maps primitive task name -> Fact node.
        Id_Table<Fact*>         primitive_lookup;
        // all cases in the order of definition.
        Array<Case*>            cases;
        // parsed `world` block.
        World*                  world;
        // parsed `primitive` block.
        Primitive*              primitive;
        // parsed `domain` block.
        Domain*                 domain;
        // errors.
        Array<Error>*           errs;
        // tree data allocator.
        Memory_Stack*           pool;
        // allocator for temporary data.
        Memory_Stack*           scratch;
    };

    // Parsed `world` block.
    struct World : public Node
    {
        // fact declarations.
        Array<Fact*>            facts;
    };

    // Parsed `primitive` block.
    struct Primitive : public Node
    {
        Array<Fact*>            tasks;
    };

    // Parsed `domain` block.
    struct Domain : public Node
    {
        // domain name.
        Token_Value             name;
        // tasks.
        Array<Task*>            tasks;
        // macros defined inside `domain`.
        Array<Macro*>           macros;
        // macro name -> `ast::Macro`.
        Id_Table<Macro*>        macro_lookup;
    };

    // Fact: Id + Parameters.
    struct Fact : public Node
    {
        // name of the fact.
        Token_Value             name;
        // input buffer location where this fact is defined.
        Location                loc;
        // parameters.
        Array<Data_Type*>       params;
    };

    // Fact parameter type.
    struct Data_Type : public Node
    {
        // must be one of the type tokens.
        Token_Type              data_type;
    };

    // Parsed `macro`.
    struct Macro : public Node
    {
        // name of the macro.
        Token_Value             name;
        // input buffer location where this macro is defined.
        Location                loc;
        // parameters.
        Array<Param*>           params;
        // expression this macro is expanded into.
        Expr*                   expression;
    };

    // Parsed `task` block.
    struct Task : public Node
    {
        // name of the task.
        Token_Value             name;
        // input buffer location where this task is defined.
        Location                loc;
        // task parameters.
        Array<Param*>           params;
        // expansion cases.
        Array<Case*>            cases;
        // param name -> node.
        Id_Table<Param*>        param_lookup;
        // macros defined inside `task`.
        Array<Macro*>           macros;
        // macro name -> `ast::Macro`.
        Id_Table<Macro*>        macro_lookup;
    };

    // Parsed `case` block.
    struct Case : public Node
    {
        // is this `each` case? `each` cases iterate all precondition satisfiers.
        uint8_t                 foreach : 1;
        // a pointer to the task this case is part of.
        Task*                   task;
        // precondition.
        Expr*                   precond;
        // task list expressions.
        Array<Expr*>            task_list;
        // precondition variable name -> first occurence.
        Id_Table<ast::Var*>     precond_var_lookup;
        // array of all vars in precondition.
        Array<ast::Var*>        precond_vars;
        // task list variable name -> first occurence.
        Id_Table<ast::Var*>     task_list_var_lookup;
        // array of all vars in task list.
        Array<ast::Var*>        task_list_vars;
        // ast::Fact for each ast::Func instance. 
        Array<ast::Fact*>       precond_facts;
    };

    // Parameter: Id + Data type.
    struct Param : public Node
    {
        // name of the parameter.
        Token_Value             name;
        // input buffer location where this param is defined.
        Location                loc;
        // inferred or defined data type (one of the type tokens).
        Token_Type              data_type;
    };

    // Parsed precondition expression or task list item.
    struct Expr : public Node
    {
        // input buffer location of this expression.
        Location                loc;
        // parent node.
        Expr*                   parent;
        // first child node.
        Expr*                   child;
        // next sibling.
        Expr*                   next_sibling;
        // previous sibling (forms cyclic list).
        Expr*                   prev_sibling_cyclic;
    };

    // Operator, used in precondition expression.
    struct Op : public Expr {};

    // Variable, used in precondition and task list expressions.
    struct Var : public Expr
    {
        // name of the variable.
        Token_Value             name;
        // `ast::Param` or `ast::Var` when this variable was first bound (i.e. the first occurence in expression)
        Node*                   definition;
        // inferred data type for this variable.
        Token_Type              data_type;

        union
        {
            // index of the variable in the input signature.
            uint32_t            input_index;
            // index of the variable in the output signature.
            uint32_t            output_index;
        };
    };

    // Literal.
    struct Literal : public Expr
    {
        // string value of the literal.
        Token_Value             value;
        // type of the literal (one of Token_Literal_*).
        Token_Type              value_type;
    };

    // Functional Symbol: fact/function/task/macro used in precondtition or task list.
    struct Func : public Expr
    {
        // name of the fact/function/task.
        Token_Value             name;
        // an expression for each argument.
        Array<ast::Expr*>       args;
    };
}

// Parser state.
struct Parser
{
    // token source for parsing.
    Lexer*              lexer;
    // last lexed token.
    Token               token;
    // output Abstract-Syntax-Tree.
    ast::Root*          tree;
    // errors.
    Array<Error>*       errs;
    // allocator for parsing data.
    Memory_Stack*       scratch;
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
    // input AST.
    ast::Root*          tree;
    // allocator for scratch data.
    Memory_Stack*       scratch;

    // expand function names `<composite_task_name>_case_<case_index>`, in order of definition.
    String_Buffer       expand_names;
    // signatures in the following order: primitive, composite, precondition bindings.
    Signature_Table     task_and_binding_sigs;
    // signatures of task parameters & precondition output structs.
    Signature_Table     struct_sigs;
};

}

#endif
