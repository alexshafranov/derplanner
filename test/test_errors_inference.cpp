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
    TEST(_0)  { check_error("(:domain (d) (:method (m x) (and\n(a x)) ()))", error_undefined, 2, 2); }
    TEST(_1)  { check_error("(:worldstate (w) (a (t))) (:domain (d) (:method (m x) (a\n(f x)) ()))", error_undefined, 2, 2); }
    TEST(_2)  { check_error("(:domain (d) (:method (m\nx) () ()))", error_unable_to_infer_type, 2, 1); }
    TEST(_3)  { check_error("(:domain (d) (:method (m\nx) () ()))", error_unable_to_infer_type, 2, 1); }
    TEST(_4)  { check_error("(:worldstate (w) (a (t1) (t2))) (:domain (d) (:method (m)\n(a x y w) ()))", error_wrong_number_of_arguments, 2, 2); }
    TEST(_5)  { check_error("(:worldstate (w) (a (t1) (t2))) (:domain (d) (:method (m)\n(a x) ()))", error_wrong_number_of_arguments, 2, 2); }
    TEST(_6)  { check_error("(:worldstate (w) (a (t1) (t2) (t3)) (:function (f (t1) (t2))->(b))) (:domain (d) (:method (m) ((a x y w)\n(f x y w)) ()))", error_wrong_number_of_arguments, 2, 2); }
    TEST(_7)  { check_error("(:worldstate (w) (a (t1)) (:function (f (t1) (t2))->(b))) (:domain (d) (:method (m) ((a x)\n(f x)) ()))", error_wrong_number_of_arguments, 2, 2); }
    TEST(_8)  { check_error("(:worldstate (w) (a (t1)) (b (t2))) (:domain (d) (:method (m) ((a x)\n(b x)) ()))", error_type_mismatch, 2, 4); }
    TEST(_9)  { check_error("(:worldstate (w) (a (t1)) (:function (f (t2))->(b))) (:domain (d) (:method (m) ((a x)\n(f x)) ()))", error_type_mismatch, 2, 4); }
    TEST(_10) { check_error("(:worldstate (w) (a (t1)) (b (t2))) (:domain (d) (:method (m\nx) ((a x) (b x)) ()))", error_type_mismatch, 2, 12); }
    TEST(_11) { check_error("(:worldstate (w) (a (t1)) (:function (f)->(t2))) (:domain (d) (:method (m) ((a\n(f))) ()))", error_type_mismatch, 2, 2); }
}
