#pragma once

#include <cstddef>
#include <cstdint>

namespace mem
{
struct arena
{
	std::byte *buffer;
	size_t base;
	size_t capacity;
};

bool create_arena(const size_t capacity, arena &out_arena) noexcept;

bool destroy_arena(arena &in_arena) noexcept;

// Only for arenas which content is aligned all the same
bool allocate_same_alignment(const size_t bytes, arena &in_arena,
                             void **out_mem) noexcept;

} // namespace mem
