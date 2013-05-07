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

#include <stdio.h>
#include <unittestpp.h>
#include <derplanner/compiler/s_expression.h>

using namespace derplanner::s_expression;

namespace
{
    TEST(trivial)
    {
        tree s_exp;
        const char* text = "(hello world)";
        s_exp.parse(text);

        node* root = s_exp.root;

        CHECK(!root->next_sibling);
        CHECK(!root->prev_sibling_cyclic);

        for (node* n = root->first_child; n != 0; n = n->next_sibling)
        {
            printf("text:");

            for (const char* c = n->text_begin; c != n->text_end; ++c)
            {
                printf("%c", *c);
            }

            printf("\n");
        }
    }
}
