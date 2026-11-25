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

// Only for arenas which content is aligned all the same
bool allocate_same_alignment(const size_t bytes, arena &in_arena,
                             void **out_mem) noexcept;

bool get_obj_at_chill(const arena &in_arena, const size_t obj_size,
                      const size_t index, void **out_obj) noexcept;

inline void *get_obj_at(const arena &in_arena, const size_t obj_size,
                        const size_t index) noexcept
{
	return in_arena.buffer + (obj_size * index);
}

inline bool obj_in_range(const arena &in_arena, const size_t obj_size,
                         const size_t index) noexcept
{
	return obj_size * index + obj_size < in_arena.base;
}

} // namespace mem
