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
#include <derplanner/compiler/errors.h>
#include "test_errors.h"

using namespace plnnrc;
using namespace test;

namespace
{
    TEST(test_0)  { check_error("(:domain (t1))\n(:domain\n(t2))", error_multiple_definitions, 2, 1); }
    TEST(test_1)  { check_error("(:domain)", error_expected_type, 1, 9); }
    TEST(test_2)  { check_error("(:domain test)", error_expected_type, 1, 10); }
    TEST(test_3)  { check_error("(:domain (test1 (test2)))", error_expected_type, 1, 17); }
    TEST(test_4)  { check_error("(:domain (~))", error_invalid_id, 1, 11); }
    TEST(test_5)  { check_error("(:domain (t) x)", error_unexpected, 1, 14); }
    TEST(test_6)  { check_error("(:domain (t) (:method))", error_expected_type, 1, 22); }
    TEST(test_7)  { check_error("(:domain (t) (:method\n()))", error_expected_type, 2, 1); }
    TEST(test_8)  { check_error("(:domain (t) (:method\n(())))", error_expected_type, 2, 1); }
    TEST(test_9)  { check_error("(:domain (t) (:method\n(:lazy)))", error_expected_type, 2, 7); }
    TEST(test_10) { check_error("(:domain (t) (:method\n(~)))", error_invalid_id, 2, 2); }
    TEST(test_11) { check_error("(:domain (t) (:method\n(:lazy ~)))", error_invalid_id, 2, 8); }
    TEST(test_12) { check_error("(:domain (t) (:method\n(m ~)))", error_invalid_id, 2, 4); }
    TEST(test_13) { check_error("(:domain (t) (:method\n(m)) (:method\n(m)))", error_redefinition, 3, 2); }
    TEST(test_14) { check_error("(:domain (t) (:method\n(m) b))", error_expected_type, 2, 5); }
    TEST(test_15) { check_error("(:domain (t) (:method\n(m) (:foreach b)))", error_expected_type, 2, 15); }
    TEST(test_16) { check_error("(:domain (t) (:method\n(m) (:foreach ())))", error_expected_type, 2, 16); }
    TEST(test_17) { check_error("(:domain (t) (:method\n(m) ()))", error_expected_type, 2, 6); }
    TEST(test_18) { check_error("(:domain (t) (:method\n(m) (()) ()))", error_expected_type, 2, 6); }
    TEST(test_19) { check_error("(:domain (t) (:method\n(m) (and x) ()))", error_expected_type, 2, 10); }
    TEST(test_20) { check_error("(:domain (t) (:method\n(m) () (x)))", error_expected_type, 2, 9); }
    TEST(test_21) { check_error("(:domain (t) (:operator))", error_expected_type, 1, 24); }
}
