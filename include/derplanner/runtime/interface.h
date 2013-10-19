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

#ifndef DERPLANNER_RUNTIME_INTERFACE_H_
#define DERPLANNER_RUNTIME_INTERFACE_H_

#include <derplanner/runtime/worldstate.h>

namespace plnnr {

class worldstate
{
public:
    worldstate(void* data);

    template <typename T>
    void append(const T& tuple);

    void* data() const { return _data; }

private:
    void* _data;
};

template <typename R, typename V>
struct generated_type_reflector
{
    void operator()(const R& reflected, V& visitor)
    {
        // specialized in the generated code.
    }
};

template <typename T, typename V>
void reflect(const T& generated_type, V& visitor)
{
    generated_type_reflector<T, V> reflector;
    reflector(generated_type, visitor);
}

}

#define PLNNR_GENCODE_VISIT_ATOM_LIST(GENCODE_NAMESPACE, ATOM_TYPE, ATOM_TUPLE, VISITOR_INSTANCE)            \
    VISITOR_INSTANCE.atom_list(                                                                             \
        GENCODE_NAMESPACE::ATOM_TYPE,                                                                        \
        GENCODE_NAMESPACE::atom_name(GENCODE_NAMESPACE::ATOM_TYPE),                                           \
        plnnr::tuple_list::head<GENCODE_NAMESPACE::ATOM_TUPLE>(world.atoms[GENCODE_NAMESPACE::ATOM_TYPE]))    \

#define PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(VISITOR_INSTANCE, TUPLE_INSTANCE, INDEX)  \
    VISITOR_INSTANCE.atom_element(TUPLE_INSTANCE._ ## INDEX)                        \

#define PLNNR_GENCODE_VISIT_TUPLE_BEGIN(VISITOR_INSTANCE, GENCODE_NAMESPACE, ATOM_TYPE, ELEMENT_COUNT)   \
    VISITOR_INSTANCE.atom_begin(                                                                        \
        GENCODE_NAMESPACE::ATOM_TYPE,                                                                    \
        GENCODE_NAMESPACE::atom_name(GENCODE_NAMESPACE::ATOM_TYPE),                                       \
        ELEMENT_COUNT)                                                                                  \

#define PLNNR_GENCODE_VISIT_TUPLE_END(VISITOR_INSTANCE, GENCODE_NAMESPACE, ATOM_TYPE, ELEMENT_COUNT)     \
    VISITOR_INSTANCE.atom_end(                                                                          \
        GENCODE_NAMESPACE::ATOM_TYPE,                                                                    \
        GENCODE_NAMESPACE::atom_name(GENCODE_NAMESPACE::ATOM_TYPE),                                       \
        ELEMENT_COUNT)                                                                                  \

#include <derplanner/runtime/interface.inl>

#endif
