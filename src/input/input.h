#pragma once

#include <cstdint>
namespace input
{
struct input_owner
{
	uint32_t id;
};

void get_owner(input_owner &out_owner);
bool bind_key(input_owner owner, int key, void *arg_data,
              void (*action_func)(void *));
bool execute_action(int key);
} // namespace input
