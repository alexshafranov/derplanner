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

#include "unittestpp.h"
#include "derplanner/compiler/lexer.h"

namespace
{
    TEST(test_eof)
    {
        plnnrc::Scoped<plnnrc::Lexer_State> lexer;
        plnnrc::init(lexer, "");
        plnnrc::Token actual = plnnrc::lex(lexer);
        CHECK_EQUAL(plnnrc::Token_Eof, actual.type);
    }

    TEST(test_id)
    {
        plnnrc::Scoped<plnnrc::Lexer_State> lexer;
        plnnrc::init(lexer, "id1");
        plnnrc::Token actual = plnnrc::lex(lexer);
        CHECK_EQUAL(plnnrc::Token_Identifier, actual.type);
    }

    TEST(test_ids_and_kws)
    {
        plnnrc::Scoped<plnnrc::Lexer_State> lexer;
        plnnrc::init(lexer, "id1 id2 domain id3");
        plnnrc::Token_Type expected_types[] = { plnnrc::Token_Identifier, plnnrc::Token_Identifier, plnnrc::Token_Domain, plnnrc::Token_Identifier, plnnrc::Token_Eof };
        const char* expected_strings[] = { "id1", "id2", "domain", "id3", 0 };

        for (unsigned i = 0; i < sizeof(expected_types)/sizeof(expected_types[0]); ++i)
        {
            plnnrc::Token actual = plnnrc::lex(lexer);
            CHECK_EQUAL(expected_types[i], actual.type);
            if (expected_strings[i] != 0)
            {
                CHECK_ARRAY_EQUAL(expected_strings[i], actual.str, actual.length);
            }
        }
    }

    TEST(location)
    {
        plnnrc::Scoped<plnnrc::Lexer_State> lexer;
        plnnrc::init(lexer, "id1 \n id2");
        plnnrc::Token tok1 = plnnrc::lex(lexer);
        plnnrc::Token tok2 = plnnrc::lex(lexer);
        CHECK_EQUAL(1u, tok1.line);
        CHECK_EQUAL(1u, tok1.column);
        CHECK_EQUAL(2u, tok2.line);
        CHECK_EQUAL(2u, tok2.column);
    }
}
