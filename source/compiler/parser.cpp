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

#include <stdio.h>

#include <string.h>
#include "derplanner/compiler/memory.h"
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/ast.h"
#include "derplanner/compiler/id_table.h"
#include "derplanner/compiler/errors.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/parser.h"

using namespace plnnrc;

// partial parsing functions, exposed for unit testing
namespace plnnrc
{
    ast::Domain*        parse_domain(Parser* self);
    ast::World*         parse_world(Parser* self);
    ast::Primitive*     parse_primitive(Parser* self);
    ast::Task*          parse_task(Parser* self);
    ast::Expr*          parse_precond(Parser* self);
    void                init_look_ahead(Parser* self);
}

template <typename T>
struct Children_Builder;

template <typename T_Parser>
static bool parse_tuple(Parser* self, T_Parser parse_element);

template <typename T_Node, typename T_Parser>
static bool parse_declaration(Parser* self, Children_Builder<T_Node>& builder, T_Parser parse_item, Token_Type keyword);

static ast::Attribute* parse_attribute(Parser* self);

static ast::Expr* parse_expr(Parser* self);
static ast::Expr* parse_binary_expr(Parser* self, uint8_t precedence);
static ast::Expr* parse_term_expr(Parser* self);
static ast::Expr* parse_postfix_expr(Parser* self, ast::Expr* lhs);

static ast::Fact*  parse_single_fact(Parser* self);
static ast::Macro* parse_single_macro(Parser* self);
static ast::Macro* parse_single_constant(Parser* self);

static bool parse_attributes(Parser* self, Children_Builder<ast::Attribute>& builder);
static bool parse_param_types(Parser* self, Children_Builder<ast::Data_Type>& builder);
static bool parse_params(Parser* self, Children_Builder<ast::Param>& builder);
static bool parse_world(Parser* self, Children_Builder<ast::Fact>& builder);
static bool parse_primitive(Parser* self, Children_Builder<ast::Fact>& builder);
static bool parse_task_body(Parser* self, ast::Task* task);
static bool parse_case(Parser* self, ast::Task* task, Children_Builder<ast::Case>& builder);
static bool parse_task_list(Parser* self, ast::Case* case_);
static bool parse_macros(Parser* self, Children_Builder<ast::Macro>& builder);
static bool parse_constants(Parser* self, Children_Builder<ast::Macro>& builder);

void plnnrc::init(Parser* self, Lexer* lexer, ast::Root* tree, Array<Error>* errors, Memory_Stack* scratch)
{
    plnnrc_assert(lexer);
    plnnrc_assert(tree);
    plnnrc_assert(scratch);

    memset(self, 0, sizeof(Parser));
    self->lexer = lexer;
    self->tree = tree;
    self->errs = errors;
    init(self->attrs, scratch, Attribute_Count);
    self->scratch = scratch;

#define PLNNRC_ATTRIBUTE(ATTR_TAG, ATTR_STR)            \
    set(self->attrs, ATTR_STR, Attribute_##ATTR_TAG);   \

    #include "derplanner/compiler/attribute_tags.inl"
#undef PLNNRC_ATTRIBUTE
}

static Token peek(const Parser* self, uint32_t look_ahead = 0)
{
    plnnrc_assert(look_ahead < Parser::Look_Ahead_Size);
    const uint32_t index = (self->buffer_index + look_ahead) & (Parser::Look_Ahead_Size - 1);
    return self->buffer[index];
}

static Token eat(Parser* self)
{
    const uint32_t index = self->buffer_index;
    const Token tok = self->buffer[index];
    self->buffer[index] = lex(self->lexer);
    self->buffer_index = (self->buffer_index + 1) & (Parser::Look_Ahead_Size - 1);
    return tok;
}

static Token make_error_token(const Parser* self, Token_Type type)
{
    Token tok;
    tok.error = true;
    tok.type = type;
    tok.loc = peek(self).loc;
    const char* error_str = "<error>";
    tok.value.str = error_str;
    tok.value.length = (uint32_t) strlen(error_str);
    return tok;
}

static Error& emit(Parser* self, Error_Type error_type)
{
    Error err;
    init(err, error_type, peek(self).loc);
    push_back(*self->errs, err);
    return back(*self->errs);
}

static Token expect(Parser* self, Token_Type token_type)
{
    const Token tok = peek(self);

    if (tok.type != token_type)
    {
        emit(self, Error_Expected) << token_type << tok;
        eat(self);
        return make_error_token(self, token_type);
    }

    return eat(self);
}

static Token expect(Parser* self, Token_Group token_group)
{
    const Token_Type actual_type = peek(self).type;
    if (actual_type < get_group_first(token_group) || actual_type > get_group_last(token_group))
    {
        emit(self, Error_Expected) << token_group << peek(self);
        eat(self);
        return make_error_token(self, get_group_first(token_group));
    }

    return eat(self);
}

// error recovery: look for sync tokens inside the domain scope.
static bool skip_inside_domain(Parser* self)
{
    eat(self);

    while (!is_Eos(peek(self)))
    {
        const Token tok = peek(self);
        if (is_Fact(tok) || is_Primitive(tok) || is_Macro(tok) || is_Const(tok) || is_Task(tok))
            return true;

        eat(self);
    }

    return false;
}

// error recovery: look for sync tokens inside the task scope.
static bool skip_inside_task(Parser* self)
{
    eat(self);

    while (!is_Eos(peek(self)))
    {
        const Token tok = peek(self);
        if (is_Case(tok) || is_Each(tok) || is_Macro(tok) || is_Const(tok))
            return true;

        eat(self);
    }

    return false;
}

// RAII helper to keep AST child nodes allocated in temp memory, moving to persistent in destructor.
template <typename T>
struct Children_Builder
{
    const Parser*       state;
    Array<T*>*          output;
    Memory_Stack_Scope  mem_scope;
    Array<T*>           nodes;

    Children_Builder(const Parser* state, Array<T*>* output)
        : state(state)
        , output(output)
        , mem_scope(state->scratch)
    {
        init(nodes, state->scratch, 16);
    }

    void push_back(T* node)
    {
        plnnrc::push_back(nodes, node);
    }

    ~Children_Builder()
    {
        const uint32_t node_count = size(nodes);
        if (!node_count)
        {
            return;
        }

        if (!output->memory)
        {
            init(*output, state->tree->pool, node_count);
        }

        if (node_count > 0)
        {
            plnnrc::push_back(*output, &nodes[0], node_count);
        }
    }
};

void plnnrc::init_look_ahead(Parser* self)
{
    self->buffer_index = 0;

    for (uint32_t i = 0; i < Parser::Look_Ahead_Size; ++i)
    {
        self->buffer[i] = lex(self->lexer);
    }
}

void plnnrc::parse(Parser* self)
{
    // initialize the look-ahead buffer.
    init_look_ahead(self);

    ast::Domain* domain = parse_domain(self);

    if (!domain)
    {
        Token tok = make_error_token(self, Token_Id);
        self->tree->domain = create_domain(self->tree, tok.value);
    }

    if (!self->tree->world)
    {
        self->tree->world = create_world(self->tree);
    }

    if (!self->tree->primitive)
    {
        self->tree->primitive = create_primitive(self->tree);
    }

    if (domain && !is_Eos(peek(self)))
    {
        emit(self, Error_Expected_End_Of_Stream);
    }
}

#define plnnrc_check_return(COND)               \
    if (!(COND))                                \
    {                                           \
        return 0;                               \
    }                                           \

#define plnnrc_check_skip(STATE, NODE, SKIP)    \
    if (!NODE)                                  \
    {                                           \
        if (!SKIP(STATE))                       \
            return 0;                           \
                                                \
        continue;                               \
    }                                           \

#define plnnrc_expect_return(STATE, ARG)        \
    if (is_Error(expect(STATE, ARG)))           \
    {                                           \
        return 0;                               \
    }                                           \

ast::Domain* plnnrc::parse_domain(Parser* self)
{
    plnnrc_expect_return(self, Token_Domain);
    const Token name = expect(self, Token_Id);
    plnnrc_check_return(!is_Error(name));

    ast::Domain* domain = create_domain(self->tree, name.value);
    ast::World* world = create_world(self->tree);
    ast::Primitive* prim = create_primitive(self->tree);

    self->tree->domain = domain;
    self->tree->world = world;
    self->tree->primitive = prim;

    plnnrc_expect_return(self, Token_L_Curly);
    {
        Children_Builder<ast::Task> task_builder(self, &domain->tasks);

        while (!is_Eos(peek(self)))
        {
            const Token tok = peek(self);

            if (is_R_Curly(tok))
            {
                break;
            }

            if (is_Fact(tok))
            {
                Children_Builder<ast::Fact> fact_builder(self, &world->facts);
                const bool ok = parse_world(self, fact_builder);
                plnnrc_check_skip(self, ok, skip_inside_domain);
                continue;
            }

            if (is_Primitive(tok))
            {
                Children_Builder<ast::Fact> fact_builder(self, &prim->tasks);
                const bool ok = parse_primitive(self, fact_builder);
                plnnrc_check_skip(self, ok, skip_inside_domain);
                continue;
            }

            if (is_Macro(tok))
            {
                Children_Builder<ast::Macro> macro_builder(self, &domain->macros);
                const bool ok = parse_macros(self, macro_builder);
                plnnrc_check_skip(self, ok, skip_inside_domain);
                continue;
            }

            if (is_Const(tok))
            {
                Children_Builder<ast::Macro> macro_builder(self, &domain->macros);
                const bool ok = parse_constants(self, macro_builder);
                plnnrc_check_skip(self, ok, skip_inside_domain);
                continue;
            }

            if (is_Task(tok))
            {
                ast::Task* task = parse_task(self);
                plnnrc_check_skip(self, task, skip_inside_domain)
                task_builder.push_back(task);
                continue;
            }

            emit(self, Error_Unexpected_Token) << tok;
            skip_inside_domain(self);
        }
    }

    plnnrc_expect_return(self, Token_R_Curly);
    return domain;
}

ast::World* plnnrc::parse_world(Parser* self)
{
    ast::World* world = create_world(self->tree);
    Children_Builder<ast::Fact> builder(self, &world->facts);
    plnnrc_check_return(parse_world(self, builder));
    return world;
}

ast::Primitive* plnnrc::parse_primitive(Parser* self)
{
    ast::Primitive* prim = create_primitive(self->tree);
    Children_Builder<ast::Fact> builder(self, &prim->tasks);
    plnnrc_check_return(parse_primitive(self, builder));
    return prim;
}

ast::Task* plnnrc::parse_task(Parser* self)
{
    plnnrc_expect_return(self, Token_Task);
    const Token tok = expect(self, Token_Id);
    plnnrc_check_return(!is_Error(tok));
    ast::Task* task = create_task(self->tree, tok.value, tok.loc);
    Children_Builder<ast::Param> param_builder(self, &task->params);
    plnnrc_check_return(parse_params(self, param_builder));
    plnnrc_check_return(parse_task_body(self, task));
    return task;
}

ast::Expr* plnnrc::parse_precond(Parser* self)
{
    ast::Expr* expr = parse_expr(self);
    plnnrc_check_return(expr);
    return expr;
}

template <typename T_Parser>
static bool parse_tuple(Parser* self, T_Parser parse_element)
{
    plnnrc_expect_return(self, Token_L_Paren);

    while (!is_Eos(peek(self)))
    {
        if (is_R_Paren(peek(self)))
        {
            break;
        }

        plnnrc_check_return(parse_element(self));

        if (!is_Comma(peek(self)))
        {
            break;
        }

        plnnrc_expect_return(self, Token_Comma);
    }

    plnnrc_expect_return(self, Token_R_Paren);
    return true;
}

template <typename T_Node, typename T_Parser>
static bool parse_declaration(Parser* self, Children_Builder<T_Node>& builder, T_Parser parse_item, Token_Type keyword)
{
    plnnrc_expect_return(self, keyword);

    // a declaration can be a block of items: `fact { a() b() }`
    if (is_L_Curly(peek(self)))
    {
        plnnrc_expect_return(self, Token_L_Curly);

        while (!is_Eos(peek(self)))
        {
            const Token tok = peek(self);

            if (is_R_Curly(tok))
            {
                break;
            }

            if (is_Id(tok))
            {
                T_Node* node = parse_item(self);
                plnnrc_check_return(node);
                builder.push_back(node);
                continue;
            }

            emit(self, Error_Unexpected_Token) << tok;
            eat(self);
            break;
        }

        plnnrc_expect_return(self, Token_R_Curly);
        return true;
    }

    // or a single item: `fact a()`
    T_Node* node = parse_item(self);
    plnnrc_check_return(node);
    builder.push_back(node);
    return true;
}

struct Parse_Data_Type
{
    Children_Builder<ast::Data_Type>* builder;
    Parse_Data_Type(Children_Builder<ast::Data_Type>* builder) : builder(builder) {}

    bool operator()(Parser* state)
    {
        const Token tok = expect(state, Token_Group_Type);
        plnnrc_check_return(!is_Error(tok));
        ast::Data_Type* param = create_type(state->tree, tok.type);
        builder->push_back(param);
        return true;
    }
};

struct Parse_Param
{
    Children_Builder<ast::Param>* builder;
    Parse_Param(Children_Builder<ast::Param>* builder) : builder(builder) {}

    bool operator()(Parser* state)
    {
        const Token tok = expect(state, Token_Id);
        plnnrc_check_return(!is_Error(tok));
        ast::Param* param = create_param(state->tree, tok.value, tok.loc);
        builder->push_back(param);
        return true;
    }
};

struct Parse_Argument
{
    ast::Func* func;
    Parse_Argument(ast::Func* func) : func(func) {}

    bool operator()(Parser* state)
    {
        ast::Expr* node = parse_expr(state);
        plnnrc_check_return(node);
        append_child(func, node);
        return true;
    }
};

struct Parse_Attribute_Argument
{
    Children_Builder<ast::Expr>* builder;
    Parse_Attribute_Argument(Children_Builder<ast::Expr>* builder) : builder(builder) {}

    bool operator()(Parser* state)
    {
        ast::Expr* node = parse_expr(state);
        plnnrc_check_return(node);
        builder->push_back(node);
        return true;
    }
};

static bool parse_param_types(Parser* self, Children_Builder<ast::Data_Type>& builder)
{
    return parse_tuple(self, Parse_Data_Type(&builder));
}

static bool parse_params(Parser* self, Children_Builder<ast::Param>& builder)
{
    return parse_tuple(self, Parse_Param(&builder));
}

static bool parse_attributes(Parser* self, Children_Builder<ast::Attribute>& builder)
{
    while (is_Literal_Symbol(peek(self, 0)) && is_L_Paren(peek(self, 1)))
    {
        ast::Attribute* attr = parse_attribute(self);
        plnnrc_check_return(attr);
        builder.push_back(attr);
    }

    return true;
}

static ast::Macro* parse_single_macro(Parser* self)
{
    const Token tok = expect(self, Token_Id);
    plnnrc_check_return(!is_Error(tok));
    ast::Macro* macro = create_macro(self->tree, tok.value, tok.loc);

    Children_Builder<ast::Param> param_builder(self, &macro->params);
    parse_params(self, param_builder);
    plnnrc_expect_return(self, Token_Assign);
    macro->expression = parse_precond(self);
    plnnrc_check_return(macro->expression);
    return macro;
}

static ast::Macro* parse_single_constant(Parser* self)
{
    const Token tok = expect(self, Token_Id);
    plnnrc_check_return(!is_Error(tok));
    ast::Macro* macro = create_macro(self->tree, tok.value, tok.loc);
    plnnrc_expect_return(self, Token_Assign);
    macro->expression = parse_expr(self);
    plnnrc_check_return(macro->expression);
    return macro;
}

static ast::Fact* parse_single_fact(Parser* self)
{
    const Token tok = expect(self, Token_Id);
    plnnrc_check_return(!is_Error(tok));
    ast::Fact* fact = create_fact(self->tree, tok.value, tok.loc);
    Children_Builder<ast::Data_Type> param_builder(self, &fact->params);
    plnnrc_check_return(parse_param_types(self, param_builder));
    Children_Builder<ast::Attribute> attrs_builder(self, &fact->attrs);
    plnnrc_check_return(parse_attributes(self, attrs_builder));
    return fact;
}

static bool parse_macros(Parser* self, Children_Builder<ast::Macro>& builder)
{
    return parse_declaration(self, builder, parse_single_macro, Token_Macro);
}

static bool parse_constants(Parser* self, Children_Builder<ast::Macro>& builder)
{
    return parse_declaration(self, builder, parse_single_constant, Token_Const);
}

static bool parse_world(Parser* self, Children_Builder<ast::Fact>& builder)
{
    return parse_declaration(self, builder, parse_single_fact, Token_Fact);
}

static bool parse_primitive(Parser* self, Children_Builder<ast::Fact>& builder)
{
    return parse_declaration(self, builder, parse_single_fact, Token_Primitive);
}

static bool parse_case(Parser* self, ast::Task* task, Children_Builder<ast::Case>& builder)
{
    const Token tok = eat(self);
    plnnrc_assert(is_Case(tok) || is_Each(tok));

    ast::Case* case_ = create_case(self->tree);
    case_->foreach = is_Each(tok);
    case_->task = task;

    Children_Builder<ast::Attribute> attrs_builder(self, &case_->attrs);
    plnnrc_check_return(parse_attributes(self, attrs_builder));

    if (!is_Arrow(peek(self)))
    {
        ast::Expr* precond = parse_precond(self);
        plnnrc_check_return(precond);
        case_->precond = precond;
    }
    else
    {
        // empty expression
        ast::Expr* node_And = create_op(self->tree, ast::Node_And);
        case_->precond = node_And;
    }

    plnnrc_expect_return(self, Token_Arrow);

    plnnrc_check_return(parse_task_list(self, case_));

    builder.push_back(case_);
    return true;
}

static bool parse_task_body(Parser* self, ast::Task* task)
{
    plnnrc_expect_return(self, Token_L_Curly);

    while (!is_Eos(peek(self)))
    {
        const Token tok = peek(self);

        if (is_R_Curly(tok))
        {
            break;
        }

        if (is_Case(tok) || is_Each(tok))
        {
            Children_Builder<ast::Case> cases_builder(self, &task->cases);
            bool ok = parse_case(self, task, cases_builder);
            plnnrc_check_skip(self, ok, skip_inside_task);
            continue;
        }

        if (is_Macro(tok))
        {
            Children_Builder<ast::Macro> macro_builder(self, &task->macros);
            bool ok = parse_macros(self, macro_builder);
            plnnrc_check_skip(self, ok, skip_inside_task);
            continue;
        }

        if (is_Const(tok))
        {
            Children_Builder<ast::Macro> macro_builder(self, &task->macros);
            bool ok = parse_constants(self, macro_builder);
            plnnrc_check_skip(self, ok, skip_inside_task);
            continue;
        }

        emit(self, Error_Unexpected_Token) << tok;
        skip_inside_task(self);
    }

    plnnrc_expect_return(self, Token_R_Curly);
    return true;
}

static bool parse_task_list(Parser* self, ast::Case* case_)
{
    plnnrc_expect_return(self, Token_L_Square);
    Children_Builder<ast::Expr> builder(self, &case_->task_list);

    while (!is_Eos(peek(self)))
    {
        const Token tok = peek(self);

        if (is_R_Square(tok))
        {
            break;
        }

        ast::Expr* node_task = parse_term_expr(self);
        plnnrc_check_return(node_task);

        builder.push_back(node_task);

        if (!is_Comma(peek(self)))
        {
            break;
        }

        plnnrc_expect_return(self, Token_Comma);
    }

    plnnrc_expect_return(self, Token_R_Square);
    return true;
}

static ast::Attribute* parse_attribute(Parser* self)
{
    const Token tok = expect(self, Token_Literal_Symbol);
    plnnrc_check_return(!is_Error(tok));
    ast::Attribute* attr = create_attribute(self->tree, tok.value, tok.loc);

    const Attribute_Type* attr_type = get(self->attrs, tok.value);
    attr->attr_type = attr_type ? *attr_type : Attribute_Custom;

    if (is_L_Paren(peek(self)))
    {
        Children_Builder<ast::Expr> builder(self, &attr->args);
        plnnrc_check_return(parse_tuple(self, Parse_Attribute_Argument(&builder)));
    }

    return attr;
}

static ast::Expr* parse_expr(Parser* self)
{
    return parse_binary_expr(self, 0);
}

// NOTE: depends on the order of token definition in token_tags.inl.
static uint8_t s_precedence[] =
{
    1, // Token_Or
    2, // Token_And
    3, // Token_Assign
    3, // Token_Equal
    3, // Token_NotEqual
    4, // Token_Less
    4, // Token_LessEqual
    4, // Token_Greater
    4, // Token_GreaterEqual
    5, // Token_Plus
    5, // Token_Minus
    6, // Token_Mul
    6, // Token_Div
};

static ast::Node_Type s_token_to_ast[] =
{
    #define PLNNRC_OPERATOR_TOKEN(TAG, STR) ast::Node_##TAG,
    #include "derplanner/compiler/token_tags.inl"
    #undef PLNNRC_OPERATOR_TOKEN
};

static uint8_t get_precedence(Token_Type token_type)
{
    plnnrc_assert(is_Binary(token_type));
    const uint32_t idx = (uint32_t)(token_type - Token_Group_Binary_First);
    return s_precedence[idx];
}

static ast::Node_Type get_op_type(Token_Type token_type)
{
    plnnrc_assert(is_Operator(token_type));
    const uint32_t idx = (uint32_t)(token_type - Token_Group_Operator_First);
    return s_token_to_ast[idx];
}

static ast::Expr* parse_binary_expr(Parser* self, uint8_t precedence)
{
    ast::Expr* root = parse_term_expr(self);
    plnnrc_check_return(root);

    while (!is_Eos(peek(self)))
    {
        const Token tok = peek(self);
        if (!is_Binary(tok))
        {
            break;
        }

        const uint8_t new_precedence = get_precedence(tok.type);
        if (new_precedence < precedence)
        {
            break;
        }

        eat(self);

        ast::Expr* node_Op = create_op(self->tree, get_op_type(tok.type), tok.loc);
        append_child(node_Op, root);

        ast::Expr* next_expr = parse_binary_expr(self, new_precedence);
        plnnrc_check_return(next_expr);
        append_child(node_Op, next_expr);

        root = node_Op;
    }

    return root;
}

static ast::Expr* parse_term_expr(Parser* self)
{
    const Token tok = eat(self);

    if (is_Literal(tok))
    {
        if (is_Literal_Symbol(tok))
        {
            set(self->tree->symbols, tok.value, tok.value);
        }

        return create_literal(self->tree, tok, tok.loc);
    }

    // unary operators
    if (is_Not(tok) || is_Plus(tok) || is_Minus(tok))
    {
        ast::Expr* node_op = create_op(self->tree, get_op_type(tok.type), tok.loc);
        ast::Expr* node_arg = parse_term_expr(self);
        plnnrc_check_return(node_arg);
        append_child(node_op, node_arg);
        return node_op;
    }

    if (is_L_Paren(tok))
    {
        // empty expression -> add dummy node.
        if (is_R_Paren(peek(self)))
        {
            eat(self);
            return create_op(self->tree, ast::Node_And);
        }

        ast::Expr* expr = parse_expr(self);
        plnnrc_check_return(expr);
        plnnrc_expect_return(self, Token_R_Paren);
        return parse_postfix_expr(self, expr);
    }

    if (is_Id(tok))
    {
        // variable or constant
        if (!is_L_Paren(peek(self)))
        {
            ast::Expr* node_Var = create_var(self->tree, tok.value, tok.loc);
            return parse_postfix_expr(self, node_Var);
        }

        // otherwise a function call.
        ast::Func* node_Func = create_func(self->tree, tok.value, tok.loc);
        plnnrc_check_return(parse_tuple(self, Parse_Argument(node_Func)));
        return parse_postfix_expr(self, node_Func);
    }

    emit(self, Error_Unexpected_Token) << tok;
    return 0;
}

static ast::Expr* parse_postfix_expr(Parser* self, ast::Expr* lhs)
{
    if (is_Dot(peek(self)))
    {
        eat(self);
        ast::Expr* node_Dot = create_op(self->tree, ast::Node_Dot, peek(self).loc);
        const Token tok = expect(self, Token_Id);
        plnnrc_check_return(!is_Error(tok));
        ast::Expr* rhs = create_var(self->tree, tok.value, tok.loc);
        append_child(node_Dot, lhs);
        append_child(node_Dot, rhs);
        return node_Dot;
    }

    return lhs;
}
