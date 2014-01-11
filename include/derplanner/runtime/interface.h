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
#include <derplanner/runtime/assert.h>

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
    void operator()(const R& /*reflected*/, V& /*visitor*/)
    {
        // specialized in the generated code.
    }
};

template <typename T, typename V>
struct task_type_dispatcher
{
    void operator()(const T& /*task_type*/, void* /*args*/, V& /*visitor*/)
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

template <typename T, typename I, typename V>
void dispatch(I* task_instance, V& visitor)
{
    task_type_dispatcher<T, V> dispatcher;
    dispatcher(static_cast<T>(task_instance->type), arguments(task_instance), visitor);
}

template <typename T, typename I, typename V>
void walk_stack_down(I* top_task, V& visitor)
{
    for (I* task = top_task; task != 0; task = task->prev)
    {
        dispatch<T>(task, visitor);
    }
}

template <typename T, typename I, typename V>
void walk_stack_up(I* top_task, V& visitor)
{
    for (I* task = top_task; task != 0; task = task->next)
    {
        dispatch<T>(task, visitor);
    }
}

template <typename I>
struct task_compare
{
    I* task_a;
    bool result;

    void task(int task_type, const char* /*task_name*/)
    {
        result = (task_a->type == task_type);
    }

    template <typename A>
    void task(int task_type, const char* /*task_name*/, const A* args)
    {
        result = (task_a->type == task_type);

        if (result)
        {
            const A* task_args = static_cast<const A*>(arguments(task_a));
            result = (*(task_args) == *(args));
        }
    }
};

template <typename T, typename I>
bool compare(I* a, I* b)
{
    task_compare<I> comparator;
    comparator.task_a = a;
    comparator.result = false;
    dispatch<T>(b, comparator);
    return comparator.result;
}

}

#define PLNNR_GENCODE_VISIT_ATOM_LIST(GENCODE_NAMESPACE, ATOM_TYPE, ATOM_TUPLE, VISITOR_INSTANCE)               \
    VISITOR_INSTANCE.atom_list(                                                                                 \
        GENCODE_NAMESPACE::ATOM_TYPE,                                                                           \
        GENCODE_NAMESPACE::atom_name(GENCODE_NAMESPACE::ATOM_TYPE),                                             \
        plnnr::tuple_list::head<GENCODE_NAMESPACE::ATOM_TUPLE>(world.atoms[GENCODE_NAMESPACE::ATOM_TYPE]))      \

#define PLNNR_GENCODE_VISIT_TUPLE_ELEMENT(VISITOR_INSTANCE, TUPLE_INSTANCE, INDEX)      \
    VISITOR_INSTANCE.atom_element(TUPLE_INSTANCE._ ## INDEX)                            \

#define PLNNR_GENCODE_VISIT_TUPLE_BEGIN(VISITOR_INSTANCE, GENCODE_NAMESPACE, ATOM_NAME_FUNC, ATOM_TYPE, ELEMENT_COUNT)      \
    VISITOR_INSTANCE.atom_begin(                                                                                            \
        GENCODE_NAMESPACE::ATOM_TYPE,                                                                                       \
        GENCODE_NAMESPACE::ATOM_NAME_FUNC(GENCODE_NAMESPACE::ATOM_TYPE),                                                    \
        ELEMENT_COUNT)                                                                                                      \

#define PLNNR_GENCODE_VISIT_TUPLE_END(VISITOR_INSTANCE, GENCODE_NAMESPACE, ATOM_NAME_FUNC, ATOM_TYPE, ELEMENT_COUNT)        \
    VISITOR_INSTANCE.atom_end(                                                                                              \
        GENCODE_NAMESPACE::ATOM_TYPE,                                                                                       \
        GENCODE_NAMESPACE::ATOM_NAME_FUNC(GENCODE_NAMESPACE::ATOM_TYPE),                                                    \
        ELEMENT_COUNT)                                                                                                      \

#define PLNNR_GENCODE_VISIT_TASK_WITH_ARGS(VISITOR_INSTANCE, GENCODE_NAMESPACE, TASK_ID, TASK_ARGS_TYPE)    \
    VISITOR_INSTANCE.task(                                                                                  \
        GENCODE_NAMESPACE::TASK_ID,                                                                         \
        GENCODE_NAMESPACE::task_name(GENCODE_NAMESPACE::TASK_ID),                                           \
        static_cast<GENCODE_NAMESPACE::TASK_ARGS_TYPE*>(args))                                              \

#define PLNNR_GENCODE_VISIT_TASK_NO_ARGS(VISITOR_INSTANCE, GENCODE_NAMESPACE, TASK_ID)  \
    VISITOR_INSTANCE.task(                                                              \
        GENCODE_NAMESPACE::TASK_ID,                                                     \
        GENCODE_NAMESPACE::task_name(GENCODE_NAMESPACE::TASK_ID))                       \

#include <derplanner/runtime/interface.inl>

#endif
