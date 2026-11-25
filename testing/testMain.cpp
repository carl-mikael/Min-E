#pragma once

#include "../src/macros.h"
#include "../src/util/arena.h"
#include "../src/util/ecs.h"

#include <cassert>
#include <cstdint>

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
	ecs::entity e{2432};

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
	LOG("Component pos: " << component_mem->pos[0] << component_mem->pos[1]
	                      << component_mem->pos[2] << " bs: "
	                      << component_mem->bs[0] << component_mem->bs[1]);

	ecs::entity e2{4};
	transform *component2;
	if (!ecs::allocate_component_chill(storage, e2, sizeof(transform),
	                                   (void **)&component2))
	{
		return -1;
	}
	new (component2) transform{{6, 7, 8}, {9, 10}};
	LOG("Component2 pos: " << component2->pos[0] << component2->pos[1]
	                       << component2->pos[2] << " bs: " << component2->bs[0]
	                       << component2->bs[1]);

	transform *component_ref;
	if (!ecs::get_component_chill(storage, e, sizeof(transform),
	                              (void **)&component_ref))
	{
		return -1;
	}
	LOG("Component ref pos: " << component_ref->pos[0] << component_ref->pos[1]
	                          << component_ref->pos[2] << " bs: "
	                          << component_ref->bs[0] << component_ref->bs[1]);

	transform *component_ref2;
	if (!ecs::get_component_chill(storage, e2, sizeof(transform),
	                              (void **)&component_ref2))
	{
		return -1;
	}
	LOG("Component ref2 pos: "
	    << component_ref2->pos[0] << component_ref2->pos[1]
	    << component_ref2->pos[2] << " bs: " << component_ref2->bs[0]
	    << component_ref2->bs[1]);

	assert(component_mem == component_ref);
	assert(component2 == component_ref2);
	LOG("It's wooooooooorking");

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
