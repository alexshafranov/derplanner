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

#ifndef PLNNRC_ATTRIBUTE
    #define PLNNRC_ATTRIBUTE(TAG, STR)
#endif

#ifndef PLNNRC_ATTRIBUTE_ARG
    #define PLNNRC_ATTRIBUTE_ARG(ARG_TYPE)
#endif

#ifndef PLNNRC_ATTRIBUTE_END
    #define PLNNRC_ATTRIBUTE_END
#endif

PLNNRC_ATTRIBUTE(Sorted,    ":sorted")
    PLNNRC_ATTRIBUTE_ARG(Expression)
PLNNRC_ATTRIBUTE_END

PLNNRC_ATTRIBUTE(Size,      ":size")
    PLNNRC_ATTRIBUTE_ARG(Constant_Expression)
PLNNRC_ATTRIBUTE_END

#undef PLNNRC_ATTRIBUTE_END
#undef PLNNRC_ATTRIBUTE_ARG
#undef PLNNRC_ATTRIBUTE
