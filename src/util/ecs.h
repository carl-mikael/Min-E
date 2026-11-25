#pragma once

#include "arena.h"
#include <cstdint>

namespace ecs
{
struct storage
{
	// I think it would be most efficent if everything was stored in one arena,
	// having bookkeeping first. I'll simplify it to get started tho.

	mem::arena bookkeeping;
	mem::arena components;
};

struct entity
{
	uint32_t id;
};

bool get_component_chill(const storage &in_storage, const entity &in_entity,
                         const size_t component_size,
                         void **component) noexcept;

bool allocate_component_chill(storage &in_storage, const entity &in_entity,
                              const size_t component_size,
                              void **out_mem) noexcept;

} // namespace ecs
