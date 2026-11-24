#pragma once

#include <cstdint>
#include <vector>
// Asset manager
namespace am
{
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

struct model_handle
{
	uint32_t index_count;
	size_t first_index;
	size_t base_vertex;
};

struct model_handles
{
	std::vector<model_handle> handles;
};

// Not really needed, could just return/get the values instead of using a struct
struct model_data_sizes
{
	size_t vertices_size;
	size_t indices_size;
};

struct objects_data
{
	// The ids specified in the file. "Instance groups"
	std::vector<uint32_t> ids;
	// How many instances per id. First index, is nr inctences for the first id
	std::vector<uint32_t> instance_counts;
	// Where are the objects placed
	std::vector<float> positions;
};

struct scene_data
{
	uint32_t id;
	float start_pos[3];
	float goal_pos[3];
	objects_data objects_data;
};

inline constexpr const float *get_data(const vertex &vert) noexcept
{
	return &vert.pos[0];
}

inline constexpr const model_data_sizes
get_bytes(const models_data &md) noexcept
{
	return {.vertices_size = md.vertices.size() * sizeof(md.vertices[0]),
	        .indices_size = md.indices.size() * sizeof(md.indices[0])};
}

// Load model assets from disk
bool load_models(const char *files[], uint32_t file_count,
                 models_data &out_models,
                 model_handles &out_model_handles) noexcept;

// Load scene data from disk
bool load_scene(const char *file, scene_data &out_scene) noexcept;

} // namespace am
