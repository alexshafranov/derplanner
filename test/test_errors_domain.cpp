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
    TEST(_0)  { check_error("(:domain (t1))\n(:domain\n(t2))", error_multiple_definitions, 2, 1); }
    TEST(_1)  { check_error("(:domain)", error_expected_type, 1, 9); }
    TEST(_2)  { check_error("(:domain test)", error_expected_type, 1, 10); }
    TEST(_3)  { check_error("(:domain (test1 (test2)))", error_expected_type, 1, 17); }
    TEST(_4)  { check_error("(:domain (~))", error_invalid_id, 1, 11); }
    TEST(_5)  { check_error("(:domain (t) x)", error_unexpected, 1, 14); }
    TEST(_6)  { check_error("(:domain (t) (:method))", error_expected_type, 1, 22); }
    TEST(_7)  { check_error("(:domain (t) (:method\n()))", error_expected_type, 2, 1); }
    TEST(_8)  { check_error("(:domain (t) (:method\n(())))", error_expected_type, 2, 1); }
    TEST(_9)  { check_error("(:domain (t) (:method\n(:lazy)))", error_expected_type, 2, 7); }
    TEST(_10) { check_error("(:domain (t) (:method\n(~)))", error_invalid_id, 2, 2); }
    TEST(_11) { check_error("(:domain (t) (:method\n(:lazy ~)))", error_invalid_id, 2, 8); }
    TEST(_12) { check_error("(:domain (t) (:method\n(m ~)))", error_invalid_id, 2, 4); }
    TEST(_13) { check_error("(:domain (t) (:method\n(m)) (:method\n(m)))", error_redefinition, 3, 2); }
    TEST(_14) { check_error("(:domain (t) (:method\n(m) b))", error_expected_type, 2, 5); }
    TEST(_15) { check_error("(:domain (t) (:method\n(m) (:foreach b)))", error_expected_type, 2, 15); }
    TEST(_16) { check_error("(:domain (t) (:method\n(m) (:foreach ())))", error_expected_type, 2, 16); }
    TEST(_17) { check_error("(:domain (t) (:method\n(m) ()))", error_expected_type, 2, 6); }
    TEST(_18) { check_error("(:domain (t) (:method\n(m) (()) ()))", error_expected_type, 2, 6); }
    TEST(_19) { check_error("(:domain (t) (:method\n(m) (and x) ()))", error_expected_type, 2, 10); }
    TEST(_20) { check_error("(:domain (t) (:method\n(m) () (x)))", error_expected_type, 2, 9); }
    TEST(_21) { check_error("(:domain (t) (:operator))", error_expected_type, 1, 24); }
    TEST(_22) { check_error("(:domain (t) (:operator\n()))", error_expected_type, 2, 1); }
    TEST(_23) { check_error("(:domain (t) (:operator\n(~)))", error_invalid_id, 2, 2); }
    TEST(_24) { check_error("(:domain (t) (:operator\n(o)\n()))", error_unexpected, 3, 1); }
    TEST(_25) { check_error("(:domain (t) (:operator\n(o)) (:operator\n(o)))", error_redefinition, 3, 2); }
    TEST(_26) { check_error("(:domain (t) (:operator (o) (:delete)\n(:delete)))", error_multiple_definitions, 2, 1); }
    TEST(_27) { check_error("(:domain (t) (:operator (o) (:add)\n(:add)))", error_multiple_definitions, 2, 1); }
    TEST(_28) { check_error("(:domain (t) (:operator (o) (:add\nx)))", error_expected_type, 2, 1); }
    TEST(_29) { check_error("(:domain (t) (:operator (o) (:delete\nx)))", error_expected_type, 2, 1); }
    TEST(_30) { check_error("(:worldstate (w) (a (t))) (:domain (d) (:method (m) (a\n()) ()))", error_expected_type, 2, 1); }
    TEST(_31) { check_error("(:domain (d) (:method (m\n(x)) () ()))", error_expected_parameter, 2, 2); }
    TEST(_32) { check_error("(:domain (d) (:method (m) () ((!t\nx))))", error_unbound_var, 2, 1); }
    TEST(_33) { check_error("(:worldstate (w) (:function (f (t))->(b))) (:domain (d) (:method (m)\n((f u)) ()))", error_unbound_var, 2, 5); }
    TEST(_34) { check_error("(:domain (d) (:method (m)\n((== u u)) ()))", error_unbound_var, 2, 6); }
}
