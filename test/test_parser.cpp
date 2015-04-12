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

#include <string>
#include "unittestpp.h"
#include "derplanner/compiler/array.h"
#include "derplanner/compiler/lexer.h"
#include "derplanner/compiler/parser.h"
#include "derplanner/compiler/transforms.h"

// bring in parser implementation details.
namespace plnnrc
{
    extern ast::World*  parse_world(Parser& state);
    extern ast::Expr*   parse_precond(Parser& state);

    extern void         flatten(ast::Expr* root);
    extern ast::Expr*   convert_to_nnf(ast::Root& tree, ast::Expr* root);
}

namespace
{
    std::string to_string(const plnnrc::ast::World* world)
    {
        std::string output;

        for (plnnrc::ast::Fact_Type* fact = world->facts; fact != 0; fact = fact->next)
        {
            output.append(fact->name.str, fact->name.length);
            output.append("[");

            for (plnnrc::ast::Fact_Param* param = fact->params; param != 0; param = param->next)
            {
                const char* token_name = plnnrc::get_type_name(param->type);
                output.append(token_name);

                if (param->next != 0)
                {
                    output.append(", ");
                }
            }

            output.append("]");

            if (fact->next != 0)
            {
                output.append(" ");
            }
        }

        return output;
    }

    void to_string(const plnnrc::ast::Expr* expr, std::string& output)
    {
        output += plnnrc::get_type_name(expr->type);
        if (expr->value.length > 0)
        {
            output += "[";
            output.append(expr->value.str, expr->value.length);
            output += "]";
        }

        if (expr->child)
        {
            output += "{ ";
            for (plnnrc::ast::Expr* child = expr->child; child != 0; child = child->next_sibling)
            {
                to_string(child, output);
                output += " ";
            }
            output += "}";
        }
    }

    struct Test_Compiler
    {
        plnnrc::Lexer   lexer;
        plnnrc::Parser  parser;
    };

    void init(Test_Compiler& compiler, const char* input)
    {
        plnnrc::init(compiler.lexer, input);
        plnnrc::init(compiler.parser, &compiler.lexer);
        compiler.parser.token = plnnrc::lex(compiler.lexer);
    }

    TEST(world_parsing)
    {
        Test_Compiler compiler;
        init(compiler, "{ f1(int32) f2(float, int32) f3() }");
        const char* expected = "f1[Int32] f2[Float, Int32] f3[]";
        plnnrc::ast::World* world = plnnrc::parse_world(compiler.parser);
        std::string world_str = to_string(world);
        CHECK_EQUAL(expected, world_str.c_str());
    }

    void check_expr(const char* input, const char* expected)
    {
        Test_Compiler compiler;
        init(compiler, input);
        plnnrc::ast::Expr* expr = plnnrc::parse_precond(compiler.parser);
        std::string actual;
        to_string(expr, actual);
        CHECK_EQUAL(expected, actual.c_str());
    }

    TEST(precondition_parsing)
    {
        check_expr("()", "And");
        check_expr("( f(x, y, z) )", "Id[f]{ Id[x] Id[y] Id[z] }");
        check_expr("( ~(a & b) | c )", "Or{ Not{ And{ Id[a] Id[b] } } Id[c] }");
        check_expr("( a & (b & c) )", "And{ Id[a] And{ Id[b] Id[c] } }");
    }

    void check_flattened_expr(const char* input, const char* expected)
    {
        Test_Compiler compiler;
        init(compiler, input);
        plnnrc::ast::Expr* expr = plnnrc::parse_precond(compiler.parser);
        plnnrc::flatten(expr);
        std::string actual;
        to_string(expr, actual);
        CHECK_EQUAL(expected, actual.c_str());
    }

    TEST(minimizing_expression_depth)
    {
        check_flattened_expr("( ~(a & (b & (c | d | (e | f))) & (g & h)) )",
                             "Not{ And{ Id[a] Id[b] Or{ Id[c] Id[d] Id[e] Id[f] } Id[g] Id[h] } }");
    }

    void check_nnf_expr(const char* input, const char* expected)
    {
        Test_Compiler compiler;
        init(compiler, input);
        plnnrc::ast::Expr* expr = plnnrc::parse_precond(compiler.parser);
        expr = plnnrc::convert_to_nnf(compiler.parser.tree, expr);
        std::string actual;
        to_string(expr, actual);
        CHECK_EQUAL(expected, actual.c_str());
    }

    TEST(nnf_conversion)
    {
        // trivial (already nnf).
        check_nnf_expr("(  )", "And");
        check_nnf_expr("( ~x & ~y )", "And{ Not{ Id[x] } Not{ Id[y] } }");
        // double-negation.
        check_nnf_expr("(   ~~x )", "Id[x]");
        check_nnf_expr("( ~~~~x )", "Id[x]");
        check_nnf_expr("(  ~~~x )", "Not{ Id[x] }");
        // De-Morgan's law.
        check_nnf_expr("( ~(x & (y | ~z)) )", "Or{ Not{ Id[x] } And{ Not{ Id[y] } Id[z] } }");
    }

    void check_dnf_expr(const char* input, const char* expected)
    {
        Test_Compiler compiler;
        init(compiler, input);
        plnnrc::ast::Expr* expr = plnnrc::parse_precond(compiler.parser);
        expr = plnnrc::convert_to_dnf(compiler.parser.tree, expr);
        std::string actual;
        to_string(expr, actual);
        CHECK_EQUAL(expected, actual.c_str());
    }

    TEST(dnf_conversion)
    {
        // test trivial conversions.
        check_dnf_expr("( )", "Or{ And }");
        check_dnf_expr("( x )", "Or{ Id[x] }");
        check_dnf_expr("( ~x )", "Or{ Not{ Id[x] } }");
        check_dnf_expr("( a | b )", "Or{ Id[a] Id[b] }");
        // test expression is converted to nnf.
        check_dnf_expr("( a & ~(b | c) )", "Or{ And{ Id[a] Not{ Id[b] } Not{ Id[c] } } }");
        // distributive law.
        check_dnf_expr("( t1 & (~t2 | (t2 & t3 & t4)) )", "Or{ And{ Id[t1] Not{ Id[t2] } } And{ Id[t1] Id[t2] Id[t3] Id[t4] } }");
        check_dnf_expr("( q1 & (r1 | r2) & q2 & (r3 | r4) & q3 )",
                       "Or{ And{ Id[q1] Id[r1] Id[q2] Id[r3] Id[q3] } And{ Id[q1] Id[r1] Id[q2] Id[r4] Id[q3] } And{ Id[q1] Id[r2] Id[q2] Id[r3] Id[q3] } And{ Id[q1] Id[r2] Id[q2] Id[r4] Id[q3] } }");
    }
}
