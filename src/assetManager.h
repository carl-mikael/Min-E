#pragma once

#include <cstdint>
#include <vector>
// Asset manager
namespace am
{

struct model_handle
{
	uint32_t index_count;
	uint32_t first_index;
	uint32_t base_vertex;
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

struct models_data
{
	// @TODO: AGHHHHHHHH, create arena for asset manager. Don't use std::vector.
	// Replace std::vector.
	std::vector<vertex> vertices;
	std::vector<uint32_t> indices;
};

struct model_handles
{
	std::vector<model_handle> handles;
};

bool load_models(const char *files[], uint32_t file_count,
                 models_data &out_models,
                 model_handles &out_model_handles) noexcept;
} // namespace am
