#pragma once

#include "../src/macros.h"
#include "../src/renderer/renderer.h"
#include "../src/util/arena.h"
#include "../src/util/ecs.h"
#include "glm/ext/vector_float3.hpp"

#include <cassert>

int main(int, char **)
{
	LOG("Test main");
	constexpr size_t arena_size = mem::MIN_ARENA_CAPACITY;
	mem::arena transform_b{};
	if (!mem::create_arena(arena_size, transform_b))
	{
		LOG("Failed creating transform book arena");
		return -1;
	}
	mem::arena name_b{};
	if (!mem::create_arena(arena_size, name_b))
	{
		LOG("Failed creating nname book arena");
		return -1;
	}
	mem::arena mesh_b{};
	if (!mem::create_arena(arena_size, mesh_b))
	{
		LOG("Failed creating mesh book arena");
		return -1;
	}

	mem::arena transform_a{};
	if (!mem::create_arena(arena_size, transform_a))
	{
		LOG("Failed creating transforms arena");
		return -1;
	}
	mem::arena name_a{};
	if (!mem::create_arena(arena_size, name_a))
	{
		LOG("Failed creating names arena");
		return -1;
	}
	mem::arena mesh_a{};
	if (!mem::create_arena(arena_size, mesh_a))
	{
		LOG("Failed creating mesh arena");
		return -1;
	}

	ecs::storage transform_storage{transform_b, transform_a};
	ecs::storage name_storage{name_b, name_a};
	ecs::storage mesh_storage{mesh_b, mesh_a};

	ecs::entity e1{42};
	ecs::entity e2{0};
	ecs::entity e3{8};

	// Just for testing
	ecs::entity *entities[]{
		&e1,
		&e2,
		&e3,
	};

	struct transform
	{
		glm::vec3 pos;
		glm::vec3 scale;
	} *e_transform;

	struct name
	{
		char value[50];
	} *e_name;

	if (!ecs::allocate_component_chill(transform_storage, e1, sizeof(transform),
	                                   (void **)&e_transform))
	{
		return -1;
	}
	new (e_transform) transform{glm::vec3{0}, glm::vec3{1}};
	LOG_MEM("e1 pos: ", e_transform->pos, 3);
	LOG_MEM("e1 scale: ", e_transform->scale, 3);

	if (!ecs::allocate_component_chill(transform_storage, e2, sizeof(transform),
	                                   (void **)&e_transform))
	{
		return -1;
	}
	new (e_transform) transform{glm::vec3{5, 5, 0}, glm::vec3{1}};
	LOG_MEM("e2 pos: ", e_transform->pos, 3);
	LOG_MEM("e2 scale: ", e_transform->scale, 3);

	if (!ecs::allocate_component_chill(name_storage, e1, sizeof(name),
	                                   (void **)&e_name))
	{
		return -1;
	}
	new (e_name) name{"Ooone!!!"};
	LOG_ARR("e1 name: ", e_name->value);

	e_name = ecs::add_raw(name_storage, e2, name{"Twwwwwwwwooo?"});
	LOG_ARR("e2 name: ", e_name->value);

	e_name = ecs::add_raw(name_storage, e3, name{"Thre?"});
	LOG_ARR("e3 name: ", e_name->value);

	auto *mesh_p =
		ecs::add_raw(mesh_storage, e1, renderer::draw_command{36, 1, 0, 0, 0});

	LOG("e1 mesh index count: " << mesh_p->index_count);
	LOG("e1 mesh instance count: " << mesh_p->instance_count);
	LOG("e1 mesh first index: " << mesh_p->first_index);
	LOG("e1 mesh base vetex: " << mesh_p->base_vertex);
	LOG("e1 mesh base instance: " << mesh_p->base_instance);

	for (size_t i{0};
	     i < mem::get_index_of_end(name_storage.components, sizeof(name)); ++i)
	{
		// ecs::entity *e = entities[i];
		// auto *comp_idx = static_cast<decltype(ecs::entity::id) *>(
		// 	mem::get_obj_at(name_b, sizeof(ecs::entity::id), e->id));

		// auto *name_comp = static_cast<name *>(
		// 	mem::get_obj_at(name_a, sizeof(name), *comp_idx));

		const auto *name_comp = reinterpret_cast<name *>(
			name_storage.components.buffer + (i * sizeof(name)));

		LOG("Name: " << name_comp->value);

		// LOG("Entity: " << e->id << " Name: " << name_comp->value);
	}

	// Free arenas
	if (!mem::destroy_arena(transform_b))
	{
		LOG("Failed freeing book arena");
		return -1;
	}

	if (!mem::destroy_arena(name_b))
	{
		LOG("Failed freeing names book arena");
		return -1;
	}

	if (!mem::destroy_arena(mesh_b))
	{
		LOG("Failed freeing mesh book arena");
		return -1;
	}

	if (!mem::destroy_arena(transform_a))
	{
		LOG("Failed freeing transforms arena");
		return -1;
	}

	if (!mem::destroy_arena(name_a))
	{
		LOG("Failed freeing names arena");
		return -1;
	}

	if (!mem::destroy_arena(mesh_a))
	{
		LOG("Failed freeing mesh arena");
		return -1;
	}
	return 1;
}
