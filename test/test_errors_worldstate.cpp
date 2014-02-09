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
    TEST(_0)  { check_error("(:xxx)", error_unexpected, 1, 1); }
    TEST(_1)  { check_error("(:worldstate (t1))\n(:worldstate\n(t2))", error_multiple_definitions, 2, 1); }
    TEST(_2)  { check_error("(:worldstate)", error_expected_type, 1, 13); }
    TEST(_3)  { check_error("(:worldstate test)", error_expected_type, 1, 14); }
    TEST(_4)  { check_error("(:worldstate (test1 (test2)))", error_expected_type, 1, 21); }
    TEST(_5)  { check_error("(:worldstate (~))", error_invalid_id, 1, 15); }
    TEST(_6)  { check_error("(:worldstate (t) a)", error_expected_type, 1, 18); }
    TEST(_7)  { check_error("(:worldstate (t) ())", error_expected_type, 1, 18); }
    TEST(_8)  { check_error("(:worldstate (t) (()))", error_expected_type, 1, 18); }
    TEST(_9)  { check_error("(:worldstate (t) (~))", error_invalid_id, 1, 19); }
    TEST(_10) { check_error("(:worldstate (t) (a x))", error_expected_type, 1, 21); }
    TEST(_11) { check_error("(:worldstate (t) (a ()))", error_expected_type, 1, 21); }
    TEST(_12) { check_error("(:worldstate (t) (a ()))", error_expected_type, 1, 21); }
    TEST(_13) { check_error("(:worldstate (t) (a) (a))", error_redefinition, 1, 23); }
    TEST(_14) { check_error("(:worldstate (t) (:function))", error_expected_type, 1, 28); }
    TEST(_15) { check_error("(:worldstate (t) (:function (f)))", error_expected_token, 1, 31); }
    TEST(_16) { check_error("(:worldstate (t) (:function (f)->))", error_expected_type, 1, 34); }
    TEST(_17) { check_error("(:worldstate (t) (:function (f)->(t)) (:function (f)->(t)))", error_redefinition, 1, 51); }
}
