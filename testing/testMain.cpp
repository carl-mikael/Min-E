#pragma once

#include "../src/macros.h"
#include "../src/util/arena.h"
#include <cstdint>

int main(int, char **)
{
	LOG("Test main!");
	mem::arena test_arena{};
	if (!mem::create_arena(1 * (UINT16_MAX + 1), test_arena))
	{
		LOG("Failed creating arena");
	}
	else
	{
		LOG("Successfully created arena");
	}

	if (!mem::destroy_arena(test_arena))
	{
		LOG("Failed freeing arena");
	}
	else
	{
		LOG("Successfully freed arena");
	}
}
