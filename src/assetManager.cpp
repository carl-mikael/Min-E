#include "assetManager.h"
#include "macros.h"
#include "util/util.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <vector>

namespace // Helpers
{
bool load_asset(const char *file, am::models_data &out_models) noexcept
{
	LOG("Integrated loader, loading: " << file);
	Assimp::Importer importer;
	// @Warning: Triangulate doesn't work on points and lines
	constexpr unsigned int process_flags =
		aiProcess_Triangulate | aiProcess_OptimizeMeshes |
		aiProcess_JoinIdenticalVertices | aiProcess_FindInvalidData |
		aiProcess_ValidateDataStructure | aiProcess_PreTransformVertices;
	if (!importer.ValidateFlags(process_flags))
	{
		LOG("process flags for importing are invalid");
		return false;
	}

	const aiScene *scene_node = importer.ReadFile(file, process_flags);
	std::string error = importer.GetErrorString();
	if (scene_node == nullptr)
	{
		LOG("Error reading file: " << error);
		return false;
	}

	int model_base_index{0};
	for (uint32_t mesh_idx{0}; mesh_idx < scene_node->mNumMeshes; ++mesh_idx)
	{
		aiMesh *ai_mesh = scene_node->mMeshes[mesh_idx];

		// Need this before resize so we know where to copy the new mem to
		size_t vertex_offset = out_models.vertices.size();
		size_t index_offset = out_models.indices.size();

		out_models.vertices.resize(out_models.vertices.size() +
		                           (size_t)ai_mesh->mNumVertices);
		out_models.indices.resize(out_models.indices.size() +
		                          ai_mesh->mNumFaces * 3);

		for (uint32_t i{0}; i < ai_mesh->mNumVertices; ++i)
		{
			am::vertex v{};
			v.pos[0] = ai_mesh->mVertices[i].x;
			v.pos[1] = ai_mesh->mVertices[i].y;
			v.pos[2] = ai_mesh->mVertices[i].z;
			v.normal[0] = ai_mesh->mNormals[i].x;
			v.normal[1] = ai_mesh->mNormals[i].y;
			v.normal[2] = ai_mesh->mNormals[i].z;
			v.uv[0] = ai_mesh->mTextureCoords[0][i].x;
			v.uv[1] = ai_mesh->mTextureCoords[0][i].y;
			out_models.vertices[vertex_offset + i] = v;
		}

		for (uint32_t face_idx{0}; face_idx < ai_mesh->mNumFaces; ++face_idx)
		{
			for (int index{0}; index < 3; ++index)
			{
				aiFace face = ai_mesh->mFaces[face_idx];
				out_models.indices[index_offset + face_idx * 3 + index] =
					model_base_index + face.mIndices[index];
			}
		}

		// If it has multiple meshes we need to count to offset the index value
		model_base_index += ai_mesh->mNumVertices;
	}

	return true;
}
} // namespace

bool am::load_models(const char *files[], uint32_t file_count,
                     models_data &out_models,
                     model_handles &out_model_handles) noexcept
{
	size_t handle_count = static_cast<size_t>(file_count);
	std::vector<model_handle> handles(handle_count);
	for (uint32_t i{0}; i < file_count; ++i)
	{
		const char *file = *(files + i);
		LOG("loading model from file: " << file);

		model_handle mh{};
		mh.first_index = out_models.indices.size();
		mh.base_vertex = out_models.vertices.size();
		load_asset(file, out_models);
		mh.index_count =
			static_cast<uint32_t>(out_models.indices.size() - mh.first_index);
		handles[i] = mh;
	}

	out_model_handles.handles = std::move(handles);

	return true;
}

bool am::load_scene(const char *file, scene_data &out_scene) noexcept
{
	LOG("Loading scene" << file);

	std::ifstream file_stream{file, std::ios::binary};
	if (!file_stream.is_open())
	{
		LOG("Failed to load scene: " << file);
		return false;
	}

	std::string_view line_view{};
	std::string_view token{};
	std::string line{};
	while (std::getline(file_stream, line))
	{
		line_view = line;
		size_t delim_idx = line_view.find_first_of(" ");
		token = line_view.substr(0, delim_idx);

		if ("sid" == token)
		{
			util::set_values(delim_idx, line, &out_scene.id, " ", 1);
			LOG("SID: " << out_scene.id);
		}
		else if ("#" == token)
		{
		}
		else if ("\r" == token)
		{
		}
		else if ("o" == token)
		{
			uint32_t id{};
			util::set_values(delim_idx, line, &id, " ", 1);
			out_scene.object_data.ids.push_back(id);
			LOG("Object id: " << id);
			std::vector<float> &vertices = out_scene.object_data.positions;
			uint32_t instances{0};

			while (true)
			{
				++instances;
				float pos[3];

				bool consumed_line =
					util::set_values(delim_idx, line, &pos[0], " /", 3);

				vertices.push_back(pos[0]);
				vertices.push_back(pos[1]);
				vertices.push_back(pos[2]);

				if (consumed_line)
				{
					break;
				}
			}

			out_scene.object_data.instances.push_back(instances);
		}
		else if ("sp" == token)
		{
			util::set_values(delim_idx, line, &out_scene.start_pos[0], " /", 3);
			LOG("start pos: " << out_scene.start_pos[0]
			                  << out_scene.start_pos[1]
			                  << out_scene.start_pos[2]);
		}
		else if ("gp" == token)
		{
			util::set_values(delim_idx, line, &out_scene.goal_pos[0], " /", 3);
			LOG("goal pos: " << out_scene.goal_pos[0] << out_scene.goal_pos[1]
			                 << out_scene.goal_pos[2]);
		}
	}

	return true;
}
