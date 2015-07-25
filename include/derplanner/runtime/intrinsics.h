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

#ifndef DERPLANNER_RUNTIME_INTRINSICS_H_
#define DERPLANNER_RUNTIME_INTRINSICS_H_

#include <math.h>
#include "derplanner/runtime/types.h"

namespace plnnr {

inline float abs(float x) { return float(::fabs(x)); }

inline float cos(float x) { return ::cosf(x); }

inline float sin(float x) { return ::sinf(x); }

inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline float x(Vec3 v) { return v.x; }

inline float y(Vec3 v) { return v.y; }

inline float z(Vec3 v) { return v.z; }

}

#endif
