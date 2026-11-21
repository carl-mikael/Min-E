#pragma once

#include <cstdint>
namespace input
{
struct input_owner
{
	uint32_t id;
};

void get_owner(input_owner &out_owner);
bool bind_key(input_owner owner, int key, int state_mask, void *arg_data,
              void (*action_func)(void *, int new_state));
int get_key_state(int key);
bool execute_action(int key, int new_state);
} // namespace input
