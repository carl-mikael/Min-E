#pragma once

#include "../src/macros.h"
#include "../src/util/arena.h"
#include <cstdint>

int main(int, char **)
{
	LOG("Test main!");
	mem::arena test_arena{};
	constexpr size_t arena_size = 1 * (UINT16_MAX + 1);
	if (!mem::create_arena(arena_size, test_arena))
	{
		LOG("Failed creating arena");
		return -1;
	}
	else
	{
		LOG("Successfully created arena");
	}

	void *mem;
	constexpr uint32_t size = 4;
	if (!mem::allocate_same_alignment(size, test_arena, &mem))
	{
		LOG("Failed to alloacte");
		return -1;
	}

	char *str = new (mem) char[]{"This is fucked tho?"};
	LOG("Str: " << str);

	if (!mem::allocate_same_alignment(size, test_arena, &mem))
	{
		LOG("Failed to alloacte");
		return -1;
	}

	char *str2 = new (mem) char[]{"This is fucked tho?"};
	LOG("Str: " << str);
	LOG("Str2: " << str2);

	if (!mem::destroy_arena(test_arena))
	{
		LOG("Failed freeing arena");
		return -1;
	}
	else
	{
		LOG("Successfully freed arena");
	}
}
