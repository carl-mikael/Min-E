#include "ecs.h"
#include "../macros.h"
#include "arena.h"
#include <cstring>

bool ecs::get_component_chill(const storage &in_storage,
                              const entity &in_entity,
                              const size_t component_size,
                              void **component) noexcept
{
	using entity_id_t = decltype(in_entity.id);

	entity_id_t *component_index_p;
	component_index_p = (entity_id_t *)mem::get_obj_at(
		in_storage.bookkeeping, sizeof(entity_id_t), in_entity.id);

	GOODERT((mem::obj_in_range(in_storage.components, component_size,
	                           *component_index_p)),
	        "Brrrrrotha, you've messed up");

	const bool component_idx_in_range = mem::obj_in_range(
		in_storage.components, component_size, *component_index_p);

	if (!component_idx_in_range)
	{
		LOG("Component idx not in range, component idx: " << in_entity.id);
		return false;
	}

	*component = mem::get_obj_at(in_storage.components, component_size,
	                             *component_index_p);
	return true;
}

bool ecs::allocate_component_chill(storage &in_storage, const entity &in_entity,
                                   const size_t component_size,
                                   void **out_mem) noexcept
{
	using arena_base_t = decltype(mem::arena::base);
	arena_base_t mem_base;
	const bool allocated_component = mem::allocate_same_alignment_handle(
		component_size, in_storage.components, out_mem, mem_base);

	if (!allocated_component)
	{
		LOG("Failed to allocate component");
		return false;
	}

	arena_base_t component_idx = mem_base / component_size;

	void *redirect_slot =
		in_storage.bookkeeping.buffer + (sizeof(in_entity.id) * in_entity.id);

	std::memcpy(redirect_slot, &component_idx, sizeof(component_idx));

	return true;
}

bool ecs::create_storage(const size_t bookkeeping_capacity,
                         const size_t components_capacity,
                         storage &out_storage) noexcept
{
	return mem::create_arena(bookkeeping_capacity, out_storage.bookkeeping) &&
	       mem::create_arena(components_capacity, out_storage.components);
}
