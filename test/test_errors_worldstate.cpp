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
    TEST(test_1)  { check_error("(:worldstate)", error_expected_type, 1, 13); }
    TEST(test_2)  { check_error("(:worldstate test)", error_expected_type, 1, 14); }
    TEST(test_3)  { check_error("(:worldstate (test1 (test2)))", error_expected_type, 1, 21); }
    TEST(test_5)  { check_error("(:worldstate (~))", error_invalid_id, 1, 15); }
    TEST(test_6)  { check_error("(:worldstate (t) a)", error_expected_type, 1, 18); }
    TEST(test_7)  { check_error("(:worldstate (t) ())", error_expected_type, 1, 18); }
    TEST(test_8)  { check_error("(:worldstate (t) (()))", error_expected_type, 1, 18); }
    TEST(test_9)  { check_error("(:worldstate (t) (~))", error_invalid_id, 1, 19); }
    TEST(test_10) { check_error("(:worldstate (t) (a x))", error_expected_type, 1, 21); }
    TEST(test_11) { check_error("(:worldstate (t) (a ()))", error_expected_type, 1, 21); }
    TEST(test_12) { check_error("(:worldstate (t) (a ()))", error_expected_type, 1, 21); }
    TEST(test_13) { check_error("(:worldstate (t) (a) (a))", error_redefinition, 1, 23); }
    TEST(test_14) { check_error("(:worldstate (t) (:function))", error_expected_type, 1, 28); }
    TEST(test_15) { check_error("(:worldstate (t) (:function (f)))", error_expected_token, 1, 31); }
    TEST(test_16) { check_error("(:worldstate (t) (:function (f)->))", error_expected_type, 1, 34); }
    TEST(test_17) { check_error("(:worldstate (t) (:function (f)->(t)) (:function (f)->(t)))", error_redefinition, 1, 51); }
}
