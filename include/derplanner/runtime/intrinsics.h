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

const float PLNNR_PI = 3.14159265f;

inline float abs(float x) { return float(::fabs(x)); }

inline float cos(float x) { return ::cosf(x); }

inline float sin(float x) { return ::sinf(x); }

inline float rad(float x) { return x * (PLNNR_PI / 180.0f); }

inline float deg(float x) { return x * (180.0f / PLNNR_PI); }

inline float pi() { return PLNNR_PI; }

inline float clamp(float v, float a, float b) { return (v < a) ? a : ((v > b) ? b : v); }

inline Vec3 vec3(float x, float y, float z) { return Vec3(x, y, z); }

inline float x(Vec3 v) { return v.x; }

inline float y(Vec3 v) { return v.y; }

inline float z(Vec3 v) { return v.z; }

inline float dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

inline Vec3 cross(Vec3 a, Vec3 b) { return Vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }

inline float len(Vec3 a) { return ::sqrtf(a.x*a.x + a.y*a.y + a.z*a.z); }

inline float dist(Vec3 a, Vec3 b) { return len(Vec3(a.x - b.x, a.y - b.y, a.z - b.z)); }

inline Vec3 norm(Vec3 a) { float l = len(a); return Vec3(a.x / l, a.y / l, a.z / l); }

inline Vec3 operator+(Vec3 a, Vec3 b) { return Vec3(a.x + b.x, a.y + b.y, a.z + b.z); }

inline Vec3 operator-(Vec3 a, Vec3 b) { return Vec3(a.x - b.x, a.y - b.y, a.z - b.z); }

inline Vec3 operator*(Vec3 a, Vec3 b) { return Vec3(a.x * b.x, a.y * b.y, a.z * b.z); }

}

#endif
