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
#include "source/compiler/id_table.h"

namespace
{
    const char* keys[] = {
        "Ailurophile",
        "Assemblage",
        "Becoming",
        "Beleaguer",
        "Brood",
        "Bucolic",
        "Bungalow",
        "Chatoyant",
        "Comely",
        "Conflate",
        "Cynosure",
        "Dalliance",
        "Demesne",
        "Demure",
        "Denouement",
        "Desuetude",
        "Desultory",
        "Diaphanous",
        "Dissemble",
        "Dulcet",
        "Ebullience",
        "Effervescent",
        "Efflorescence",
        "Elision",
        "Elixir",
        "Eloquence",
        "Embrocation",
        "Emollient",
        "Ephemeral",
        "Epiphany",
        "Erstwhile",
        "Ethereal",
        "Evanescent",
        "Evocative",
        "Fetching",
        "Felicity",
        "Forbearance",
        "Fugacious",
        "Furtive",
        "Gambol",
        "Glamour",
        "Gossamer",
        "Halcyon",
        "Harbinger",
        "Imbrication",
        "Imbroglio",
        "Imbue",
        "Incipient",
        "Ineffable",
        "Ingenue",
        "Inglenook",
        "Insouciance",
        "Inure",
        "Labyrinthine",
        "Lagniappe",
        "Lagoon",
        "Languor",
        "Lassitude",
        "Leisure",
        "Lilt",
        "Lissome",
        "Lithe",
        "Love",
        "Mellifluous",
        "Moiety",
        "Mondegreen",
        "Murmurous",
        "Nemesis",
        "Offing",
        "Onomatopoeia",
        "Opulent",
        "Palimpsest",
        "Panacea",
        "Panoply",
        "Pastiche",
        "Penumbra",
        "Petrichor",
        "Plethora",
        "Propinquity",
        "Pyrrhic",
        "Quintessential",
        "Ratatouille",
        "Ravel",
        "Redolent",
        "Riparian",
        "Ripple",
        "Scintilla",
        "Sempiternal",
        "Seraglio",
        "Serendipity",
        "Summery",
        "Sumptuous",
        "Surreptitious",
        "Susquehanna",
        "Susurrous",
        "Talisman",
        "Tintinnabulation",
        "Umbrella",
        "Untoward",
        "Vestigial",
        "Wafture",
        "Wherewithal",
        "Woebegone",
    };

    const size_t num_keys = sizeof(keys)/sizeof(keys[0]);

    struct buffer
    {
        plnnrc::id_table_entry* entries;
        size_t count;

        buffer(size_t count_)
            : count(count_)
        {
            entries = new plnnrc::id_table_entry[count];
        }

        ~buffer()
        {
            delete [] entries;
        }
    };

    TEST(insert_and_find)
    {
        buffer b(plnnrc::id_table_required_capacity(num_keys));

        plnnrc::id_table table(b.entries, b.count);

        plnnrc::ast::node nodes[num_keys];

        for (unsigned i = 0; i < num_keys; ++i)
        {
            table.insert(keys[i], &nodes[i]);
        }

        for (unsigned i = 0; i < num_keys; ++i)
        {
            plnnrc::ast::node* actual = table.find(keys[i]);
            CHECK(actual == &nodes[i]);
        }
    }

    TEST(lookup_non_exising_keys)
    {
        buffer b(plnnrc::id_table_required_capacity(num_keys));

        plnnrc::id_table table(b.entries, b.count);

        plnnrc::ast::node nodes[num_keys];

        for (unsigned i = 0; i < num_keys / 2; ++i)
        {
            table.insert(keys[i], &nodes[i]);
        }

        for (unsigned i = num_keys / 2; i < num_keys; ++i)
        {
            plnnrc::ast::node* actual = table.find(keys[i]);
            CHECK(!actual);
        }
    }
}
