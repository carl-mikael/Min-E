#pragma once

#include <cstdint>
#include <vector>
// Asset manager
namespace am
{

struct model_handle
{
	uint32_t index_count;
	size_t first_index;
	size_t base_vertex;
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

inline constexpr const float *get_data(const vertex &vert) noexcept
{
	return &vert.pos[0];
}

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

struct model_data_sizes
{
	size_t vertices_size;
	size_t indices_size;
};

inline constexpr const model_data_sizes
get_bytes(const models_data &md) noexcept
{
	return {.vertices_size = md.vertices.size() * sizeof(md.vertices[0]),
	        .indices_size = md.indices.size() * sizeof(md.indices[0])};
}

bool load_models(const char *files[], uint32_t file_count,
                 models_data &out_models,
                 model_handles &out_model_handles) noexcept;

struct scene_object_data
{
	std::vector<uint32_t> ids;
	std::vector<uint32_t> instances;
	std::vector<float> positions;
};

struct scene_data
{
	uint32_t id;
	float start_pos[3];
	float goal_pos[3];
	scene_object_data object_data;
};

bool load_scene(const char *file, scene_data &out_scene) noexcept;
} // namespace am
