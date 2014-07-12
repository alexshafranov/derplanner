//
// Copyright (c) 2013 Alexander Shafranov shafranov@gmail.com
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

#include <unittestpp.h>
#include <derplanner/compiler/ast.h>
#include <derplanner/compiler/id_table.h>

namespace
{
    const char* keys[] = {
        "Assemblage",
        "Becoming",
        "Bucolic",
        "Chatoyant",
        "Conflate",
        "Dalliance",
        "Demure",
        "Desuetude",
        "Diaphanous",
        "Ebullience",
        "Efflorescence",
        "Eloquence",
        "Emollient",
        "Epiphany",
        "Ethereal",
        "Evocative",
        "Felicity",
        "Fugacious",
        "Gambol",
        "Halcyon",
        "Imbroglio",
        "Incipient",
        "Ingenue",
        "Insouciance",
        "Labyrinthine",
        "Languor",
        "Leisure",
        "Lissome",
        "Love",
        "Moiety",
        "Murmurous",
        "Offing",
        "Opulent",
        "Panacea",
        "Pastiche",
        "Petrichor",
        "Propinquity",
        "Ratatouille",
        "Riparian",
        "Scintilla",
        "Seraglio",
        "Summery",
        "Surreptitious",
        "Susurrous",
        "Tintinnabulation",
        "Vestigial",
        "Wherewithal",
        "Woebegone",
    };

    const size_t num_keys = sizeof(keys)/sizeof(keys[0]);

    TEST(insert_and_find)
    {
        plnnrc::Id_Table table;
        table.init(num_keys);

        plnnrc::ast::Node nodes[num_keys];

        for (unsigned i = 0; i < num_keys; ++i)
        {
            table.insert(keys[i], &nodes[i]);
        }

        for (unsigned i = 0; i < num_keys; ++i)
        {
            plnnrc::ast::Node* actual = table.find(keys[i]);
            CHECK(actual == &nodes[i]);
        }

        CHECK_EQUAL(num_keys, table.count());
    }

    TEST(lookup_non_existing_keys)
    {
        plnnrc::Id_Table table;
        table.init(num_keys);

        plnnrc::ast::Node nodes[num_keys];

        for (unsigned i = 0; i < num_keys / 2; ++i)
        {
            table.insert(keys[i], &nodes[i]);
        }

        for (unsigned i = num_keys / 2; i < num_keys; ++i)
        {
            plnnrc::ast::Node* actual = table.find(keys[i]);
            CHECK(!actual);
        }
    }

    TEST(insert_twice)
    {
        plnnrc::Id_Table table;
        table.init(1);

        plnnrc::ast::Node value1;
        plnnrc::ast::Node value2;

        const char* key = "the_key";

        table.insert(key, &value1);
        table.insert(key, &value2);

        plnnrc::ast::Node* actual = table.find(key);

        CHECK_EQUAL(&value2, actual);
        CHECK_EQUAL(1u, table.count());
    }

    TEST(value_iteration)
    {
        plnnrc::Id_Table table;
        table.init(3);

        plnnrc::ast::Node value1;
        plnnrc::ast::Node value2;
        plnnrc::ast::Node value3;

        CHECK(table.values().empty());

        table.insert("key1", &value1);
        table.insert("key2", &value2);
        table.insert("key3", &value3);

        plnnrc::Id_Table_Values values = table.values();

        plnnrc::ast::Node* v;

        CHECK(!values.empty());
        v = values.value();
        CHECK(v == &value1 || v == &value2 || v == &value3);
        values.pop();

        CHECK(!values.empty());
        v = values.value();
        CHECK(v == &value1 || v == &value2 || v == &value3);
        values.pop();

        CHECK(!values.empty());
        v = values.value();
        CHECK(v == &value1 || v == &value2 || v == &value3);
        values.pop();

        CHECK(values.empty());
    }

    TEST(table_grows)
    {
        plnnrc::Id_Table table;

        // allocate for 2 elems initially
        table.init(2);

        // but insert much more
        plnnrc::ast::Node nodes[num_keys];

        for (unsigned i = 0; i < num_keys; ++i)
        {
            table.insert(keys[i], &nodes[i]);
        }

        for (unsigned i = 0; i < num_keys; ++i)
        {
            plnnrc::ast::Node* actual = table.find(keys[i]);
            CHECK(actual == &nodes[i]);
        }

        CHECK_EQUAL(num_keys, table.count());
    }
}
