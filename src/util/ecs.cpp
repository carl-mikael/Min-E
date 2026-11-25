#include "ecs.h"
#include "../macros.h"
#include "arena.h"

bool ecs::get_component_chill(const storage &in_storage,
                              const entity &in_entity,
                              const size_t component_size,
                              void **component) noexcept
{
	using entity_id_t = decltype(in_entity.id);

	const bool component_idx_in_range = mem::obj_in_range(
		in_storage.bookkeeping, sizeof(entity_id_t), in_entity.id);

	if (!component_idx_in_range)
	{
		LOG("Component idx not in range");
		return false;
	}

	entity_id_t *component_index_p;
	component_index_p = (entity_id_t *)mem::get_obj_at(
		in_storage.bookkeeping, sizeof(entity_id_t), in_entity.id);

	GOODERT((mem::obj_in_range(in_storage.components, component_size,
	                           *component_index_p)),
	        "Brrrrrotha, you've messed up");

	*component = mem::get_obj_at(in_storage.components, component_size,
	                             *component_index_p);
	return true;
}

bool ecs::allocate_component_chill(storage &in_storage, const entity &in_entity,
                                   const size_t component_size,
                                   void **out_mem) noexcept
{
	// @WIP: Add component to components, save the "index" of its base, add it
	// to bookkeeping
}
