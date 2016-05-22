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

#include "unittestpp.h"
#include "derplanner/runtime/domain.h"

using namespace plnnr;

namespace plnnr
{
    inline bool operator==(const Vec3& a, const Vec3& b) { return a.x == b.x && a.y == b.y && a.z == b.z; }
}

namespace UnitTest
{
    MemoryOutStream& operator<<(MemoryOutStream& stream, const Vec3& v) { stream << "(" << v.x << ", " << v.y << ", " << v.z << ")"; return stream; }
}

namespace
{
    struct S
    {
        uint8_t _0;
        Vec3    _1;
        Vec3    _2;
    };

    TEST(layout_struct_aliasing)
    {
        Type types[] = { Type_Int8, Type_Vec3, Type_Vec3 };
        const size_t num_types = plnnr_static_array_size(types);
        size_t offsets[num_types];

        Param_Layout layout;
        layout.num_params = num_types;
        layout.types = types;
        layout.size = 0;
        layout.offsets = offsets;
        compute_offsets_and_size(&layout);

        char buffer[2 * sizeof(S)];
        void* object = plnnr::align(buffer + 1, layout.alignment);

        int8_t  a0 = 111;
        Vec3    a1(5.f, 4.f, 3.f);
        Vec3    a2(1.f, 2.f, 3.f);

        set_arg(object, &layout, 0, a0);
        set_arg(object, &layout, 1, a1);
        set_arg(object, &layout, 2, a2);

        const S* data = plnnr::align<S>(object);

        CHECK_EQUAL(a0, data->_0);
        CHECK_EQUAL(a1, data->_1);
        CHECK_EQUAL(a2, data->_2);
    }
}
