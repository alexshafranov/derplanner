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
#include <derplanner/runtime/worldstate.h>

using namespace plnnr;

namespace
{
    struct Tuple
    {
        int data;
        void* parent;
        Tuple* next;
        Tuple* prev;
    };

    struct holder
    {
        tuple_list::Handle* list;

        holder(size_t items_per_page=4096)
        {
            list = tuple_list::create<Tuple>(items_per_page);
        }

        ~holder()
        {
            tuple_list::destroy(list);
        }
    };

    TEST(append)
    {
        holder h;

        for (int i = 0; i < 10; ++i)
        {
            Tuple* new_tuple = tuple_list::append<Tuple>(h.list);
            CHECK(new_tuple);
            new_tuple->data = i;
        }

        int count = 0;

        Tuple* head = tuple_list::head<Tuple>(h.list);

        for (Tuple* t = head; t != 0; t = t->next, ++count)
        {
            CHECK_EQUAL(count, t->data);
        }

        CHECK_EQUAL(10, count);
    }

    TEST(undo_sequantial_add)
    {
        void* journal[10];

        holder h;

        // add items
        for (int i = 0; i < (int)(sizeof(journal)/sizeof(journal[0])); ++i)
        {
            Tuple* new_tuple = tuple_list::append<Tuple>(h.list);
            new_tuple->data = i;
        }

        int count = 0;

        // detach all except the first one
        for (Tuple* t = tuple_list::head<Tuple>(h.list); t != 0;)
        {
            Tuple* next = t->next;
            tuple_list::detach(h.list, t);
            journal[count++] = t;
            t = next;
        }

        // undo journal
        for (int i = sizeof(journal)/sizeof(journal[0]) - 1; i >= 0; --i)
        {
            tuple_list::undo(h.list, journal[i]);
        }

        // check undo
        count = 0;

        Tuple* head = tuple_list::head<Tuple>(h.list);
        CHECK(head != 0);

        for (Tuple* t = head; t != 0; t = t->next, ++count)
        {
            CHECK_EQUAL(count, t->data);
        }

        CHECK_EQUAL(sizeof(journal)/sizeof(journal[0]), (size_t)count);

        for (Tuple* tail = head; ; tail = tail->next)
        {
            if (!tail->next)
            {
                CHECK_EQUAL(head->prev, tail);
                break;
            }
        }
    }

    TEST(undo_add_and_random_delete)
    {
        void* journal[20];

        int rand_index[] = {9, 14, 5, 2, 12, 0, 19, 7, 18, 13};

        holder h;

        // add items.
        for (int i = 0; i < 10; ++i)
        {
            Tuple* new_tuple = tuple_list::append<Tuple>(h.list);
            new_tuple->data = i;
        }

        // add 10 more items.
        for (int i = 0; i < 10; ++i)
        {
            Tuple* new_tuple = tuple_list::append<Tuple>(h.list);
            new_tuple->data = 10 + i;
            journal[i] = new_tuple;
        }

        // remove 10 random items out of 20.
        for (int i = 0; i < int(sizeof(rand_index)/sizeof(rand_index[0])); ++i)
        {
            int index = rand_index[i];

            for (Tuple* t = tuple_list::head<Tuple>(h.list); t != 0; t = t->next)
            {
                if (index == t->data)
                {
                    tuple_list::detach(h.list, t);
                    journal[10 + i] = t;
                    break;
                }
            }
        }

        // undo journal
        for (int i = sizeof(journal)/sizeof(journal[0]) - 1; i >= 0; --i)
        {
            tuple_list::undo(h.list, journal[i]);
        }

        // check undo
        int count = 0;

        Tuple* head = tuple_list::head<Tuple>(h.list);
        CHECK(head != 0);

        for (Tuple* t = head; t != 0; t = t->next, ++count)
        {
            CHECK_EQUAL(count, t->data);
        }

        CHECK_EQUAL(10, count);

        for (Tuple* tail = head; ; tail = tail->next)
        {
            if (!tail->next)
            {
                CHECK_EQUAL(head->prev, tail);
                break;
            }
        }
    }

    TEST(undo_random_add_random_delete)
    {
        void* journal[5];

        int rand_index[] = {1, 4, 6, 0, 2};

        holder h;

        // add items.
        for (int i = 0; i < 10; ++i)
        {
            Tuple* new_tuple = tuple_list::append<Tuple>(h.list);
            new_tuple->data = i;
        }

        // add or delete random indices
        for (int i = 0; i < int(sizeof(rand_index)/sizeof(rand_index[0])); ++i)
        {
            int index = rand_index[i];

            if (i % 2 == 0)
            {
                for (Tuple* t = tuple_list::head<Tuple>(h.list); t != 0; t = t->next)
                {
                    if (index == t->data)
                    {
                        Tuple* new_tuple = tuple_list::append<Tuple>(h.list);
                        new_tuple->data = 10 + i;
                        journal[i] = new_tuple;
                        break;
                    }
                }
            }
            else
            {
                for (Tuple* t = tuple_list::head<Tuple>(h.list); t != 0; t = t->next)
                {
                    if (index == t->data)
                    {
                        tuple_list::detach(h.list, t);
                        journal[i] = t;
                        break;
                    }
                }
            }
        }

        // undo journal
        for (int i = sizeof(journal)/sizeof(journal[0]) - 1; i >= 0; --i)
        {
            tuple_list::undo(h.list, journal[i]);
        }

        // check undo
        int count = 0;

        Tuple* head = tuple_list::head<Tuple>(h.list);
        CHECK(head != 0);

        for (Tuple* t = head; t != 0; t = t->next, ++count)
        {
            CHECK_EQUAL(count, t->data);
        }

        CHECK_EQUAL(10, count);

        for (Tuple* tail = head; ; tail = tail->next)
        {
            if (!tail->next)
            {
                CHECK_EQUAL(head->prev, tail);
                break;
            }
        }
    }
}
