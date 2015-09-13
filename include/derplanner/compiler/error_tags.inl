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


PLNNRC_ERROR(error_unexpected, "unexpected expression.")
PLNNRC_ERROR(error_expected_type, "expected $0{<none>|list|symbol|int|float} expression.")
PLNNRC_ERROR(error_expected_token, "expected '$0'.")
PLNNRC_ERROR(error_expected_parameter, "expected parameter identifier.")
PLNNRC_ERROR(error_multiple_definitions, "multiple definitions of $0{worldstate|domain|delete effects|add effects}.")
PLNNRC_ERROR(error_redefinition, "redefinition of '$0', originally defined at $1.")
PLNNRC_ERROR(error_invalid_id, "invalid identifier '$0'.")
PLNNRC_ERROR(error_unbound_var, "unbound variable '$0' in $1{task list|call term|operation}.")
PLNNRC_ERROR(error_undefined, "'$0' is undefined.")
PLNNRC_ERROR(error_wrong_number_of_arguments, "wrong number of arguments for '$0'.")
PLNNRC_ERROR(error_type_mismatch, "expected argument of type '$0', got '$1'.")
PLNNRC_ERROR(error_unable_to_infer_type, "unable to infer type of '$0'.")
