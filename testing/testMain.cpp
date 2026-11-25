#pragma once

#include "../src/macros.h"
#include "../src/util/arena.h"
#include <cstdint>

int main(int, char **) { LOG("Test main"); }

int arena_example()
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
	if (!mem::allocate_same_alignment(3, test_arena, &mem))
	{
		LOG("Failed to alloacte");
		return -1;
	}

	new (mem) char[]{"One"};
	std::string_view one{(char *)test_arena.buffer,
	                     (char *)(test_arena.buffer + 3)};

	if (!mem::allocate_same_alignment(3, test_arena, &mem))
	{
		LOG("Failed to alloacte");
		return -1;
	}
	char *str2 = new (mem) char[]{"Two"};

	if (!mem::allocate_same_alignment(5, test_arena, &mem))
	{
		LOG("Failed to alloacte");
		return -1;
	}
	char *str3 = new (mem) char[]{"Three"};

	LOG("Str: " << one);
	LOG("Str2: " << str2);
	LOG("Str3: " << str3);

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
