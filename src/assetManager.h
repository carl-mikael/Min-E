#pragma once

#include <cstdint>
#include <vector>
// Asset manager
namespace am
{
struct resource
{
	uint32_t id;
};

// Right now I'm assuming vertex layout & attributes. Not a big fan of this but
// due to deadlines I'll do it for now.
// Don't want to decide which types the user should use, so I'm keeping it
// generic.
// Move to common, I think
struct vertex
{
	float pos[3];
	float normal[3];
	float uv[2];
};

struct mesh_data
{
	// @TODO: AGHHHHHHHH, create arena for asset manager. Don't use std::vector.
	// Replace std::vector.
	std::vector<vertex> vertices;
	std::vector<uint32_t> indices;
};

struct models
{
};

bool load_models(const char *files[], uint32_t count);
} // namespace am
