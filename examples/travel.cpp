// generated by derplanner [http://www.github.com/alexshafranov/derplanner]
#include "derplanner/runtime/domain_support.h"
#include "travel.h"

using namespace plnnr;

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4189) // local variable is initialized but not referenced
#endif

static bool root_case_0(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_case_0(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_case_1(Planning_State*, Expansion_Frame*, Fact_Database*);
static bool travel_by_plane_case_0(Planning_State*, Expansion_Frame*, Fact_Database*);

static Composite_Task_Expand* s_task_expands[] = {
  root_case_0,
  travel_case_0,
  travel_by_plane_case_0,
};

static const char* s_fact_names[] = {
  "start",
  "finish",
  "short_distance",
  "long_distance",
  "airport",
 };

static const char* s_task_names[] = {
  "taxi!",
  "plane!",
  "root",
  "travel",
  "travel_by_plane",
 };

static Fact_Type s_fact_types[] = {
  { 1, {Type_Int32, } },
  { 1, {Type_Int32, } },
  { 2, {Type_Int32, Type_Int32, } },
  { 2, {Type_Int32, Type_Int32, } },
  { 2, {Type_Int32, Type_Int32, } },
};

static Type s_layout_types[] = {
  Type_Int32,
  Type_Int32,
};

static size_t s_layout_offsets[2];

static Param_Layout s_task_parameters[] = {
  { 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
  { 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
  { 0, 0, 0, 0 },
  { 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
  { 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
};

static Param_Layout s_precond_output[] = {
  { 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 2, s_layout_types + 0, 0, s_layout_offsets + 0 },
};

static uint32_t s_num_cases[] = {
  1, 
  2, 
  1, 
};

static uint32_t s_size_hints[] = {
  0, 
  0, 
  0, 
  0, 
  0, 
};

static uint32_t s_fact_name_hashes[] = {
  1878359220, 
  1582309657, 
  2043850409, 
  2181535055, 
  3449045131, 
};

static uint32_t s_task_name_hashes[] = {
  3423006977, 
  511751884, 
  2484197952, 
  405413667, 
  660913924, 
};

static Domain_Info s_domain_info = {
  { 5, 2, 3, s_num_cases, 0, s_task_name_hashes, s_task_names, s_task_parameters, s_precond_output, s_task_expands },
  { 5, 0, s_size_hints, s_fact_types, s_fact_name_hashes, s_fact_names },
};

void travel_init_domain_info()
{
  for (size_t i = 0; i < plnnr_static_array_size(s_task_parameters); ++i) {
    compute_offsets_and_size(s_task_parameters[i]);
  }

  for (size_t i = 0; i < plnnr_static_array_size(s_precond_output); ++i) {
    compute_offsets_and_size(s_precond_output[i]);
  }
}

const Domain_Info* travel_get_domain_info() { return &s_domain_info; }

struct input_1 {
  int32_t _0;
  int32_t _1;
};

static bool p0_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
  Fact_Handle* handles = frame->handles;
  const Param_Layout& output_layout = s_precond_output[0];

  plnnr_coroutine_begin(frame, precond_label);
  handles = allocate_precond_handles(state, frame, 2);
  allocate_precond_output(state, frame, output_layout);

  for (handles[0] = first(db, 0); is_valid(db, handles[0]); handles[0] = next(db, handles[0])) { // start
    set_precond_output(frame, output_layout, 0, as_Int32(db, handles[0], 0));
    for (handles[1] = first(db, 1); is_valid(db, handles[1]); handles[1] = next(db, handles[1])) { // finish
      set_precond_output(frame, output_layout, 1, as_Int32(db, handles[1], 0));
      plnnr_coroutine_yield(frame, precond_label, 1);
    }
  }

  plnnr_coroutine_end();
}

static bool p1_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, const input_1* args)
{
  Fact_Handle* handles = frame->handles;
  const Param_Layout& output_layout = s_precond_output[1];

  plnnr_coroutine_begin(frame, precond_label);
  handles = allocate_precond_handles(state, frame, 1);

  for (handles[0] = first(db, 2); is_valid(db, handles[0]); handles[0] = next(db, handles[0])) { // short_distance
    if (args->_0 != as_Int32(db, handles[0], 0)) {
      continue;
    }

    if (args->_1 != as_Int32(db, handles[0], 1)) {
      continue;
    }

    plnnr_coroutine_yield(frame, precond_label, 1);
  }

  plnnr_coroutine_end();
}

static bool p2_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, const input_1* args)
{
  Fact_Handle* handles = frame->handles;
  const Param_Layout& output_layout = s_precond_output[2];

  plnnr_coroutine_begin(frame, precond_label);
  handles = allocate_precond_handles(state, frame, 1);

  for (handles[0] = first(db, 3); is_valid(db, handles[0]); handles[0] = next(db, handles[0])) { // long_distance
    if (args->_0 != as_Int32(db, handles[0], 0)) {
      continue;
    }

    if (args->_1 != as_Int32(db, handles[0], 1)) {
      continue;
    }

    plnnr_coroutine_yield(frame, precond_label, 1);
  }

  plnnr_coroutine_end();
}

static bool p3_next(Planning_State* state, Expansion_Frame* frame, Fact_Database* db, const input_1* args)
{
  Fact_Handle* handles = frame->handles;
  const Param_Layout& output_layout = s_precond_output[3];

  plnnr_coroutine_begin(frame, precond_label);
  handles = allocate_precond_handles(state, frame, 2);
  allocate_precond_output(state, frame, output_layout);

  for (handles[0] = first(db, 4); is_valid(db, handles[0]); handles[0] = next(db, handles[0])) { // airport
    if (args->_0 != as_Int32(db, handles[0], 0)) {
      continue;
    }

    set_precond_output(frame, output_layout, 0, as_Int32(db, handles[0], 1));
    for (handles[1] = first(db, 4); is_valid(db, handles[1]); handles[1] = next(db, handles[1])) { // airport
      if (args->_1 != as_Int32(db, handles[1], 0)) {
        continue;
      }

      set_precond_output(frame, output_layout, 1, as_Int32(db, handles[1], 1));
      plnnr_coroutine_yield(frame, precond_label, 1);
    }
  }

  plnnr_coroutine_end();
}

static bool root_case_0(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
  plnnr_coroutine_begin(frame, expand_label);

  while (p0_next(state, frame, db)) {
    begin_composite(state, 3, travel_case_0, s_task_parameters[3]); // travel
    set_composite_arg(state, s_task_parameters[3], 0, as_Int32(frame->precond_output, s_precond_output[0], 0));
    set_composite_arg(state, s_task_parameters[3], 1, as_Int32(frame->precond_output, s_precond_output[0], 1));
    frame->flags |= Expansion_Frame::Flags_Expanded;
    plnnr_coroutine_yield(frame, expand_label, 1);

  }

  plnnr_coroutine_end();
}

static bool travel_case_0(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
  input_1 args;
  args._0 = as_Int32(frame->arguments, s_task_parameters[3], 0);
  args._1 = as_Int32(frame->arguments, s_task_parameters[3], 1);

  plnnr_coroutine_begin(frame, expand_label);

  while (p1_next(state, frame, db, &args)) {
    begin_task(state, 0, s_task_parameters[0]); // taxi!
    set_task_arg(state, s_task_parameters[0], 0, as_Int32(frame->arguments, s_task_parameters[3], 0));
    set_task_arg(state, s_task_parameters[0], 1, as_Int32(frame->arguments, s_task_parameters[3], 1));
    frame->flags |= Expansion_Frame::Flags_Expanded;
    plnnr_coroutine_yield(frame, expand_label, 1);

  }

  return expand_next_case(state, frame, db, travel_case_1, s_task_parameters[3]);

  plnnr_coroutine_end();
}

static bool travel_case_1(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
  input_1 args;
  args._0 = as_Int32(frame->arguments, s_task_parameters[3], 0);
  args._1 = as_Int32(frame->arguments, s_task_parameters[3], 1);

  plnnr_coroutine_begin(frame, expand_label);

  while (p2_next(state, frame, db, &args)) {
    begin_composite(state, 4, travel_by_plane_case_0, s_task_parameters[4]); // travel_by_plane
    set_composite_arg(state, s_task_parameters[4], 0, as_Int32(frame->arguments, s_task_parameters[3], 0));
    set_composite_arg(state, s_task_parameters[4], 1, as_Int32(frame->arguments, s_task_parameters[3], 1));
    frame->flags |= Expansion_Frame::Flags_Expanded;
    plnnr_coroutine_yield(frame, expand_label, 1);

  }

  plnnr_coroutine_end();
}

static bool travel_by_plane_case_0(Planning_State* state, Expansion_Frame* frame, Fact_Database* db)
{
  input_1 args;
  args._0 = as_Int32(frame->arguments, s_task_parameters[4], 0);
  args._1 = as_Int32(frame->arguments, s_task_parameters[4], 1);

  plnnr_coroutine_begin(frame, expand_label);

  while (p3_next(state, frame, db, &args)) {
    begin_composite(state, 3, travel_case_0, s_task_parameters[3]); // travel
    set_composite_arg(state, s_task_parameters[3], 0, as_Int32(frame->arguments, s_task_parameters[4], 0));
    set_composite_arg(state, s_task_parameters[3], 1, as_Int32(frame->precond_output, s_precond_output[3], 0));
    plnnr_coroutine_yield(frame, expand_label, 1);
    if ((frame->flags & Expansion_Frame::Flags_Failed) != 0) { continue; }

    begin_task(state, 1, s_task_parameters[1]); // plane!
    set_task_arg(state, s_task_parameters[1], 0, as_Int32(frame->precond_output, s_precond_output[3], 0));
    set_task_arg(state, s_task_parameters[1], 1, as_Int32(frame->precond_output, s_precond_output[3], 1));
    plnnr_coroutine_yield(frame, expand_label, 2);

    begin_composite(state, 3, travel_case_0, s_task_parameters[3]); // travel
    set_composite_arg(state, s_task_parameters[3], 0, as_Int32(frame->precond_output, s_precond_output[3], 1));
    set_composite_arg(state, s_task_parameters[3], 1, as_Int32(frame->arguments, s_task_parameters[4], 1));
    frame->flags |= Expansion_Frame::Flags_Expanded;
    plnnr_coroutine_yield(frame, expand_label, 3);

  }

  plnnr_coroutine_end();
}

