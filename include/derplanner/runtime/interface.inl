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

#ifdef DERPLANNER_RUNTIME_INTERFACE_H_

namespace plnnr {

namespace
{
    inline tuple_list::Handle* get_handle(void* data, size_t id)
    {
        return reinterpret_cast<tuple_list::Handle**>(data)[id];
    }
}

template <typename T>
void Worldstate::append(const T& tuple)
{
    tuple_list::Handle* list = get_handle(_data, T::id);
    T* new_tuple_ptr = tuple_list::append<T>(list);
    T* next = new_tuple_ptr->next;
    T* prev = new_tuple_ptr->prev;
    *new_tuple_ptr = tuple;
    new_tuple_ptr->next = next;
    new_tuple_ptr->prev = prev;
}

template <typename T,
          typename A0>
T atom(const A0& a0)
{
    T t;
    t._0 = a0;
    return t;
}

template <typename T,
          typename A0,
          typename A1>
T atom(const A0& a0, const A1& a1)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2>
T atom(const A0& a0, const A1& a1, const A2& a2)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2,
          typename A3>
T atom(const A0& a0, const A1& a1, const A2& a2, const A3& a3)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    t._3 = a3;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2,
          typename A3,
          typename A4>
T atom(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    t._3 = a3;
    t._4 = a4;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5>
T atom(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    t._3 = a3;
    t._4 = a4;
    t._5 = a5;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6>
T atom(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    t._3 = a3;
    t._4 = a4;
    t._5 = a5;
    t._6 = a6;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7>
T atom(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    t._3 = a3;
    t._4 = a4;
    t._5 = a5;
    t._6 = a6;
    t._7 = a7;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8>
T atom(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    t._3 = a3;
    t._4 = a4;
    t._5 = a5;
    t._6 = a6;
    t._7 = a7;
    t._8 = a8;
    return t;
}

template <typename T,
          typename A0,
          typename A1,
          typename A2,
          typename A3,
          typename A4,
          typename A5,
          typename A6,
          typename A7,
          typename A8,
          typename A9>
T atom(const A0& a0, const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9)
{
    T t;
    t._0 = a0;
    t._1 = a1;
    t._2 = a2;
    t._3 = a3;
    t._4 = a4;
    t._5 = a5;
    t._6 = a6;
    t._7 = a7;
    t._8 = a8;
    t._9 = a9;
    return t;
}

}

#endif
