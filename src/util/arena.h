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

inline constexpr size_t MIN_ARENA_CAPACITY{UINT16_MAX + 1};

bool create_arena(const size_t capacity, arena &out_arena) noexcept;

bool destroy_arena(arena &in_arena) noexcept;

// Only for arenas which content is aligned all the same
bool allocate_same_alignment(const size_t bytes, arena &in_arena,
                             void **out_mem) noexcept;

bool allocate_same_alignment_handle(
	const size_t bytes, arena &in_arena, void **out_mem,
	decltype(mem::arena::base) &out_mem_base) noexcept;

bool get_obj_at_chill(const arena &in_arena, const size_t obj_size,
                      const size_t index, void **out_obj) noexcept;

constexpr inline void *get_obj_at(const arena &in_arena, const size_t obj_size,
                                  const size_t index) noexcept
{
	return in_arena.buffer + (obj_size * index);
}

constexpr inline bool obj_in_range(const arena &in_arena, const size_t obj_size,
                                   const size_t index) noexcept
{
	return obj_size * index < in_arena.base;
}

constexpr inline decltype(mem::arena::base)
get_index_of_end(const arena &in_arena, size_t element_size) noexcept
{
	return in_arena.base / element_size;
}

constexpr inline decltype(mem::arena::base)
get_last_e_idx(const arena &in_arena, size_t element_size) noexcept
{
	return get_index_of_end(in_arena, element_size) - 1;
}

} // namespace mem
