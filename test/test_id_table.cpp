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
#include "derplanner/compiler/id_table.h"

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

    const uint32_t num_keys = sizeof(keys)/sizeof(keys[0]);

    TEST(insert_and_find)
    {
        plnnrc::Id_Table<int*> table;
        plnnrc::init(table, num_keys);

        int values[num_keys] = {0};

        for (unsigned i = 0; i < num_keys; ++i)
        {
            plnnrc::set(table, keys[i], &values[i]);
        }

        for (unsigned i = 0; i < num_keys; ++i)
        {
            int* const* actual = plnnrc::get(table, keys[i]);
            CHECK(*actual == &values[i]);
        }

        CHECK_EQUAL(num_keys, plnnrc::size(table));
    }

    TEST(lookup_non_existing_keys)
    {
        plnnrc::Id_Table<int*> table;
        plnnrc::init(table, num_keys);

        int values[num_keys] = {0};

        for (unsigned i = 0; i < num_keys / 2; ++i)
        {
            plnnrc::set(table, keys[i], &values[i]);
        }

        for (unsigned i = num_keys / 2; i < num_keys; ++i)
        {
            int* const* actual = plnnrc::get(table, keys[i]);
            CHECK(!actual);
        }
    }

    TEST(insert_twice)
    {
        plnnrc::Id_Table<int*> table;
        plnnrc::init(table, 1);

        const char* key = "the_key";
        int value1 = 0;
        int value2 = 0;

        plnnrc::set(table, key, &value1);
        plnnrc::set(table, key, &value2);

        int* const* actual = plnnrc::get(table, key);

        CHECK_EQUAL(&value2, *actual);
        CHECK_EQUAL(1u, size(table));
    }

    TEST(table_grows)
    {
        plnnrc::Id_Table<int*> table;
        // allocate for 2 elems initially
        plnnrc::init(table, 2);

        // but insert much more
        int values[num_keys];

        for (unsigned i = 0; i < num_keys; ++i)
        {
            plnnrc::set(table, keys[i], &values[i]);
        }

        for (unsigned i = 0; i < num_keys; ++i)
        {
            int* const* actual = plnnrc::get(table, keys[i]);
            CHECK(*actual == &values[i]);
        }

        CHECK_EQUAL(num_keys, size(table));
    }
}
