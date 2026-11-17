#pragma once

#include <cstdint>
#include <vector>

namespace MINE_input
{
enum class action_state : uint8_t
{
	NONE,
	PRESSED,
	HELD,
	RELEASED,
};

// For user enum to create token_bindings
using key_size = uint32_t;

// Exposed keys. To be translated in cpp
enum class KEY : key_size
{
	W,
	UP,
};

// Allows for 31 unique inputs. And one combination for no input
using input_slots = uint32_t;

// Input manager
struct token_binding
{
	key_size key_token;
	input_slots slot;

	token_binding(KEY key, input_slots action)
	{
		key_token = key_size{1} << static_cast<key_size>(key);
		slot = input_slots{1} << action;
	};
};

void bind(const std::vector<token_binding> &bindings);
} // namespace MINE_input
