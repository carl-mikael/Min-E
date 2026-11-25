#pragma once

#include "../src/macros.h"
#include "../src/util/arena.h"
#include "../src/util/ecs.h"

#include <cstdint>
#include <string_view>

int main(int, char **)
{
	LOG("Test main");
	mem::arena book{};
	constexpr size_t arena_size = 1 * (UINT16_MAX + 1);
	if (!mem::create_arena(arena_size, book))
	{
		LOG("Failed creating book arena");
		return -1;
	}

	mem::arena components{};
	if (!mem::create_arena(arena_size, components))
	{
		LOG("Failed creating components arena");
		return -1;
	}

	ecs::storage storage{book, components};
	ecs::entity e{};

	struct transform
	{
		float pos[3];
		float bs[2];
	};

	transform *component_mem;
	if (!ecs::allocate_component_chill(storage, e, sizeof(transform),
	                                   (void **)&component_mem))
	{
		return -1;
	}
	new (component_mem) transform{{1, 2, 3}, {4, 5}};
	LOG("1 - Component pos: " << component_mem->pos[0] << component_mem->pos[1]
	                          << component_mem->pos[2] << " bs: "
	                          << component_mem->bs[0] << component_mem->bs[1]);

	transform *component_ref;
	if (!ecs::get_component_chill(storage, e, sizeof(transform),
	                              (void **)&component_ref))
	{
		return -1;
	}

	LOG("2 - Component pos: " << component_ref->pos[0] << component_ref->pos[1]
	                          << component_ref->pos[2] << " bs: "
	                          << component_ref->bs[0] << component_ref->bs[1]);

	if (!mem::destroy_arena(book))
	{
		LOG("Failed freeing book arena");
		return -1;
	}

	if (!mem::destroy_arena(components))
	{
		LOG("Failed freeing book arena");
		return -1;
	}

	return 1;
}
