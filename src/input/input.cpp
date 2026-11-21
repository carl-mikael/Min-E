#include "input.h"

#include "../macros.h"
#include <cstdint>
#include <unordered_map>
#include <unordered_set>

namespace
{
struct input_action
{
	// 0 reserved for unbound
	input::input_owner owner;
	void *user_data;
	void (*user_action)(void *);
};

uint32_t id_count{0};
std::unordered_set<uint32_t> owner_ids{};

// using input_map = std::unordered_map<int, input_action>;
// player id to their input map
std::unordered_map<uint32_t, input_action> input_map{};
} // namespace

namespace input
{
void get_owner(input_owner &out_owner)
{
	// Keeps 0 free, so it can be used to check if an input isn't owned
	out_owner.id = ++id_count;
	owner_ids.insert(out_owner.id);
}

bool bind_key(input_owner owner, int key, void *arg_data,
              void (*action_func)(void *))
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

	input_action ia{
		.owner = owner, .user_data = arg_data, .user_action = action_func};
	input_map.insert_or_assign(key, ia);
	return true;
}

bool execute_action(int key)
{
	if (!input_map.contains(key))
	{
		LOG("Missing input binding");
		return false;
	}

	const input_action &ia = input_map.at(key);
	ia.user_action(ia.user_data);
	return true;
}
} // namespace input
