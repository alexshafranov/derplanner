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

#ifndef PLNNRC_ERROR
    #define PLNNRC_ERROR(TAG, FORMAT_STR)
#endif

#ifndef PLNNRC_PARSER_ERROR
    #define PLNNRC_PARSER_ERROR(TAG, FORMAT_STR) PLNNRC_ERROR(TAG, FORMAT_STR)
#endif

#ifndef PLNNRC_AST_ERROR
    #define PLNNRC_AST_ERROR(TAG, FORMAT_STR) PLNNRC_ERROR(TAG, FORMAT_STR)
#endif

PLNNRC_PARSER_ERROR(Unexpected_Token,       "unexpected token $0.")
PLNNRC_PARSER_ERROR(Expected,               "expected $0, got $1.")
PLNNRC_PARSER_ERROR(Redefinition,           "multiple definitions of $0 found.")
PLNNRC_PARSER_ERROR(Expected_End_Of_Stream, "expected end-of-stream.")

PLNNRC_AST_ERROR(Recursive_Predicate, "recursive predicate $0 found.")

#undef PLNNRC_AST_ERROR
#undef PLNNRC_PARSER_ERROR
#undef PLNNRC_ERROR
