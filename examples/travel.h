#ifndef travel_H_
#define travel_H_

#include "derplanner/runtime/types.h"

namespace travel {

class Domain : public plnnr::Domain
{
public:
	virtual void init();

	virtual plnnr::Database_Format get_database_requirements() const;
	virtual plnnr::Task_Info get_task_info() const;

	enum { num_fact_types = 5 };
	enum { num_tasks = 5 };
	enum { num_composite_tasks = 3 };
	enum { num_primitive_tasks = 3 };

	plnnr::Fact_Type fact_types[num_fact_types];
	uint32_t fact_name_hashes[num_fact_types];
	const char* fact_names[num_fact_types];

	uint32_t task_name_hashes[num_tasks];
	const char* task_names[num_tasks];
	plnnr::Fact_Type task_parameters[num_tasks];
	plnnr::Composite_Task_Expand task_expands[num_composite_tasks];
};

}

#endif
