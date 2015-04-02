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

#include <stdlib.h>
#include "unittestpp.h"
#include "derplanner/compiler/lexer.h"

namespace
{
    TEST(test_eof)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, "");
        plnnrc::Token actual = plnnrc::lex(lexer);
        CHECK_EQUAL(plnnrc::Token_Eof, actual.type);
    }

    TEST(test_id)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, "id1");
        plnnrc::Token actual = plnnrc::lex(lexer);
        CHECK_EQUAL(plnnrc::Token_Identifier, actual.type);
    }

    TEST(test_ids_and_kws)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, "id1 id2 domain id3");
        plnnrc::Token_Type expected_types[] = { plnnrc::Token_Identifier, plnnrc::Token_Identifier, plnnrc::Token_Domain, plnnrc::Token_Identifier, plnnrc::Token_Eof };
        const char* expected_strings[] = { "id1", "id2", "domain", "id3", 0 };

        for (unsigned i = 0; i < sizeof(expected_types)/sizeof(expected_types[0]); ++i)
        {
            plnnrc::Token actual = plnnrc::lex(lexer);
            CHECK_EQUAL(expected_types[i], actual.type);
            if (expected_strings[i] != 0)
            {
                CHECK_ARRAY_EQUAL(expected_strings[i], actual.value.str, actual.value.length);
            }
        }
    }

    TEST(location)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, "id1 \n id2");
        plnnrc::Token tok1 = plnnrc::lex(lexer);
        plnnrc::Token tok2 = plnnrc::lex(lexer);
        CHECK_EQUAL(1u, tok1.line);
        CHECK_EQUAL(1u, tok1.column);
        CHECK_EQUAL(2u, tok2.line);
        CHECK_EQUAL(2u, tok2.column);
    }

    TEST(unknown_token)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, "$");
        plnnrc::Token tok = plnnrc::lex(lexer);
        CHECK_EQUAL(plnnrc::Token_Unknown, tok.type);
    }

    void check_number(const char* str, plnnrc::Token_Type expected)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, str);
        plnnrc::Token tok = plnnrc::lex(lexer);
        CHECK_EQUAL(expected, tok.type);
    }

    void check_number(const char* str, float expected)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, str);
        plnnrc::Token tok = plnnrc::lex(lexer);
        float actual = (float)strtod(tok.value.str, 0);
        CHECK_EQUAL(expected, actual);
    }

    void check_number(const char* str, int expected)
    {
        plnnrc::Scoped<plnnrc::Lexer> lexer;
        plnnrc::init(lexer, str);
        plnnrc::Token tok = plnnrc::lex(lexer);
        int actual = (int)strtol(tok.value.str, 0, 10);
        CHECK_EQUAL(expected, actual);
    }

    TEST(numeric_literals)
    {
        check_number("123", plnnrc::Token_Literal_Integer);
        check_number("123.", plnnrc::Token_Literal_Float);
        check_number("1.23", plnnrc::Token_Literal_Float);
        check_number("0.1e+23", plnnrc::Token_Literal_Float);
        check_number("0.1e+23", plnnrc::Token_Literal_Float);
        check_number("0.1e+23", plnnrc::Token_Literal_Float);
        check_number("1e23", plnnrc::Token_Literal_Float);
        check_number("1e-23", plnnrc::Token_Literal_Float);
        check_number("1e+23", plnnrc::Token_Literal_Float);
        check_number("123", 123);
        check_number("1.23", 1.23f);
    }
}
