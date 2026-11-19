#include "assetManager.h"
#include "macros.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cstdint>

#ifdef DEBUG
#include <iostream>
#endif

bool am::load_models(const char *files[], uint32_t count)
{
	std::vector<vertex> vertices{};
	std::vector<unsigned int> indices{};

	for (int i{0}; i < count; ++i)
	{
		LOG("loading model from file: " << *(files + i));
	}

	return true;
}

namespace
{
bool load_asset(const char *file, am::mesh_data &out_mesh)
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

	unsigned int index_count{0};
	unsigned int first_index{(unsigned int)out_mesh.indices.size()};
	int base_vertex{(int)out_mesh.vertices.size()};

	int model_base_index{0};
	for (int mesh_idx{0}; mesh_idx < scene_node->mNumMeshes; ++mesh_idx)
	{
		aiMesh *ai_mesh = scene_node->mMeshes[mesh_idx];
		index_count += ai_mesh->mNumFaces * 3;

		// Need this before resize so we know where to copy the new mem to
		int vertex_offset = out_mesh.vertices.size();
		int index_offset = out_mesh.indices.size();

		out_mesh.vertices.resize(out_mesh.vertices.size() +
		                         (size_t)ai_mesh->mNumVertices);
		out_mesh.indices.resize(out_mesh.indices.size() +
		                        ai_mesh->mNumFaces * 3);

		for (int i{0}; i < ai_mesh->mNumVertices; ++i)
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
			out_mesh.vertices[vertex_offset + i] = v;
		}

		for (int face_idx{0}; face_idx < ai_mesh->mNumFaces; ++face_idx)
		{
			for (int index{0}; index < 3; ++index)
			{
				aiFace face = ai_mesh->mFaces[face_idx];
				out_mesh.indices[index_offset + face_idx * 3 + index] =
					model_base_index + face.mIndices[index];
			}
		}

		// If it has multiple meshes we need to count to offset the index value
		model_base_index += ai_mesh->mNumVertices;
	}

	return true;
}
} // namespace
