#pragma once

#include <cstddef>
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

} // namespace mem
