#include "input.h"

#include "../macros.h"
#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace
{
struct input_action
{
	// 0 reserved for unbound
	input::input_owner owner;
	int state_mask;
	int current_state;
	void *user_data;
	void (*user_action)(void *user_data, int new_state);
};

uint32_t id_count{0};
std::unordered_set<uint32_t> owner_ids{};

// using input_map = std::unordered_map<int, input_action>;
// player id to their input map
std::unordered_map<uint32_t, input_action> input_map{};
} // namespace

namespace input
{
void get_owner(input_owner &out_owner) noexcept
{
	// Keeps 0 free, so it can be used to check if an input isn't owned
	out_owner.id = ++id_count;
	owner_ids.insert(out_owner.id);
}

bool bind_key(input_owner owner, int key, int state_mask, void *arg_data,
              void (*action_func)(void *, int new_state)) noexcept
{
	if (!owner_ids.contains(owner.id))
	{
		LOG("Owner doesn't exist!");
		return false;
	}

	if (input_map.contains(key))
	{
		LOG("Key is already bound!");
		return false;
	}

	input_action ia{.owner = owner,
	                .state_mask = state_mask,
	                .current_state = 0,
	                .user_data = arg_data,
	                .user_action = action_func};
	input_map.insert_or_assign(key, ia);
	return true;
}

int get_key_state(int key) noexcept
{
	GOODERT(input_map.contains(key), "This key havn't been bound: " << key);
	return input_map.at(key).current_state;
}

bool execute_action(int key, int new_state) noexcept
{
	if (!input_map.contains(key))
	{
		LOG("Missing input binding");
		return false;
	}

	input_action &ia = input_map.at(key);

	ia.current_state = new_state;

	// It. did. not. work
	// GLFW using 0 for key release state is weird behavior
	// if ((ia.state_mask & new_state) == 0)
	// {
	// 	LOG("FUUUUUUUUUUUU");
	// 	return false;
	// }

	ia.user_action(ia.user_data, new_state);
	return true;
}
} // namespace input
