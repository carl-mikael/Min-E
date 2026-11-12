#include "statics.h"
#include "glm/common.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glslSemantics.h"
#include "macros.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <optional>
#include <string_view>
#include <vector>

namespace statics
{
bool read_file(const char *file_path, std::string &out_content)
{
	LOG("Opening file: " << file_path);

	std::ifstream file(file_path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << file_path << std::endl;
		return false;
	}

	std::istreambuf_iterator<char> file_start{file}, end{};
	out_content = {file_start, end};

	return true;
}

bool could_compile(unsigned int shader, const char *shader_path)
{
	int success;
	char info_log[LOG_SIZE];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);
		std::cerr << "Failed to compile shader: " << shader_path
		          << " info_log: " << info_log << std::endl;

		return false;
	}

	LOG("Successfully compiled: " << shader_path);

	return true;
}

bool create_shader(unsigned int *out_shader, const char *shader_path,
                   const unsigned long type, const std::string &glsl_semantics)
{
	if (!out_shader)
	{
		std::cerr << "shader id is nullptr" << std::endl;
		return false;
	}

	*out_shader = glCreateShader(type);
	unsigned int shader = *out_shader;

	std::string shader_str{};
	if (!statics::read_file(shader_path, shader_str))
	{
		return false;
	}

	shader_str = GLSL_HEADER(glsl_semantics) + shader_str;

	const char *shader_src = shader_str.c_str();

	glShaderSource(shader, 1, &shader_src, nullptr);
	glCompileShader(shader);
	if (!statics::could_compile(shader, shader_path))
	{
		return false;
	}

	return true;
}

bool create_shader_program(unsigned int *out_shader_program,
                           const unsigned int vertex_shader,
                           const unsigned int fragment_shader,
                           const unsigned int geometry_shader)
{
	*out_shader_program = glCreateProgram();
	unsigned int program = *out_shader_program;

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	if (geometry_shader != -1)
	{
		glAttachShader(program, geometry_shader);
	}

	glLinkProgram(program);

	int success;
	char info_log[LOG_SIZE];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
		std::cerr << "Failed to compile shader program: " << info_log
		          << std::endl;
		return false;
	}

	return true;
}

triangle create_triangle(const glm::vec2 size)
{
	float vertices[]{0.0f, 0.0f, 0.0f, 0.0f, size.y, 0.0f, size.x, 0.0f, 0.0f};

	GLuint vbo, vao;
	glGenVertexArrays(1, &vao);

	// Create and get buffer handle
	// then 'upload' vertex data. 0 as flag because it shouldn't be read/written
	// to from cpu again
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

	glBindVertexArray(vao);

	constexpr int SLOT = 0;
	// Set attribute format
	// Enable attribute
	// Set slot to use
	// Define slot
	glVertexAttribFormat(ATTR_POS, 3, GL_FLOAT, false, 0);
	glEnableVertexAttribArray(ATTR_POS);
	glVertexAttribBinding(ATTR_POS, SLOT);
	glBindVertexBuffer(SLOT, vbo, 0, 3 * sizeof(float));

	glBindVertexArray(0);

	return triangle{.vao = vao, .size = size};
}

void draw_triangle(const unsigned int program, const unsigned int vao)
{
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void draw_rectangle(const unsigned int program, const triangle tri,
                    const glm::vec2 offset, const glm::vec2 scale,
                    const glm::vec3 color)
{
	int offset_location = glGetUniformLocation(program, "offset");
	glUseProgram(program);

	glUniform2fv(offset_location, 1, &offset[0]);

	int scale_location = glGetUniformLocation(program, "scale");
	glUniform2fv(scale_location, 1, &scale[0]);

	int color_location = glGetUniformLocation(program, "color");
	glUniform3fv(color_location, 1, &color[0]);

	draw_triangle(program, tri.vao);

	glUniform2fv(scale_location, 1, &glm::vec2{scale * -1.0f}[0]);
	glUniform2fv(offset_location, 1, &glm::vec2{tri.size * -1.0f - offset}[0]);

	draw_triangle(program, tri.vao);
}

void draw_house(const unsigned int program, const triangle tri,
                const glm::vec3 roof_color)
{
	// Wall
	statics::draw_rectangle(program, tri,
	                        glm::vec2{0.0f, -0.15f} - tri.size / 2.0f,
	                        glm::vec2{3.0f, 2.5f}, glm::vec3{0.0f});

	// Roof
	int offset_location = glGetUniformLocation(program, "offset");
	glUseProgram(program);

	glm::vec2 offset{0 - tri.size.x / 2.0f, 0.185f};
	glUniform2fv(offset_location, 1, &offset[0]);

	int scale_location = glGetUniformLocation(program, "scale");
	glm::vec2 scale{3.5f, 1.25f};
	glUniform2fv(scale_location, 1, &scale[0]);

	int color_location = glGetUniformLocation(program, "color");
	glUniform3fv(color_location, 1, &roof_color[0]);

	statics::draw_triangle(program, tri.vao);

	// Door
	statics::draw_rectangle(program, tri, glm::vec2{-.25f, -1.f},
	                        glm::vec2{0.50f, 1.}, glm::vec3{0, 0, 1});

	// Left Window
	statics::draw_rectangle(program, tri, glm::vec2{-1.0f, -0.65f},
	                        glm::vec2{0.50f, 0.5f}, glm::vec3{0, 0, 1});
	// Right Window
	statics::draw_rectangle(program, tri, glm::vec2{1.0f - tri.size.x, -0.65f},
	                        glm::vec2{0.50f, 0.5f}, glm::vec3{0, 0, 1});
}

cuboid create_cube(float size)
{
	glm::vec3 vertices[4 * 2]{{0.0f, 0.0f, 0.0f},
	                          {size, 0.0f, 0.0f},
	                          {0.0f, size, 0.0f},
	                          {size, size, 0.0f}};

	for (int i{0}; i < 4; ++i)
	{
		vertices[i + 4] = vertices[i] + glm::vec3{0, 0, -size};
	}

	unsigned int indices[]{
		0, 1, 2, 2, 1, 3, // Front side
		4, 0, 6, 6, 0, 2, // Left side
		1, 5, 3, 3, 5, 7, // Right side
		5, 4, 7, 7, 4, 6, // Back side
		2, 3, 6, 6, 3, 7, // Top side
		4, 5, 0, 0, 5, 1  // Bottom side
	};

	GLuint vbo, vao, ebo;
	glGenVertexArrays(1, &vao);

	// Create and get buffer handle
	// then 'upload' vertex data. 0 as flag because it shouldn't be read/written
	// to from cpu again
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

	glCreateBuffers(1, &ebo);
	glNamedBufferStorage(ebo, sizeof(indices), indices, 0);

	glBindVertexArray(vao);

	// The vao is created the first time it's bound.
	// Can't call this before the vao's first glBindVertexArray.
	glVertexArrayElementBuffer(vao, ebo);

	constexpr int SLOT = 0;
	// Set attribute format
	// Enable attribute
	// Set slot to use
	// Define slot
	glVertexAttribFormat(ATTR_POS, 3, GL_FLOAT, false, 0);
	glEnableVertexAttribArray(ATTR_POS);
	glVertexAttribBinding(ATTR_POS, SLOT);
	glBindVertexBuffer(SLOT, vbo, 0, 3 * sizeof(float));

	glBindVertexArray(0);

	return cuboid{.vao = vao, .size = glm::vec3{size}};
}

// By teacher(s), modified by me to use LOG macro and return bool
// It's not the nicest sphere, nor the nicest code, but it sort of works.
bool generate_sort_of_sphere(std::vector<float> &out_vertices,
                             std::vector<unsigned int> &out_indices, int rings,
                             int slices)
{
	if (rings < 3)
	{
		LOG("no sphere generated. rings value < 3");
		return false;
	}

	if (slices < 4)
	{
		LOG("no sphere generated. slices value < 4");
		return false;
	}

	float PIE = 3.1415f;

	out_vertices.clear();
	out_indices.clear();

	float angle_step = 360.0f / slices;
	float current_angle = 0.0f;

	// out_vertices
	float y_step = 2.0f / (rings - 1);
	float y_pos = 1.0f;

	for (int ring = 0; ring < rings; ring++)
	{
		int iteration_slices = slices;
		if (ring == 0 || ring == rings - 1)
		{
			iteration_slices = 1;
		}

		for (int i = 0; i < iteration_slices; i++)
		{
			float radius = sinf(acos(y_pos));

			float x_pos = cosf(current_angle * PIE / 180) * radius;
			float z_pos = sinf(current_angle * PIE / 180) * radius;

			out_vertices.push_back(x_pos);
			out_vertices.push_back(y_pos);
			out_vertices.push_back(z_pos);

			current_angle += angle_step;
		}

		y_pos -= y_step;
		current_angle = 0.0f;
	}

	LOG("Sphere vertex count:" << out_vertices.size() / 3);

	// out_indices
	unsigned int top_most_index = 0;
	unsigned int bottom_most_index = out_vertices.size() / 3 - 1;

	for (int i = 0; i < rings - 1; i++)
	{
		for (int j = 0; j < slices; j++)
		{
			unsigned int tl_index = (i - 1) * slices + j + 1;
			unsigned int tr_index = (i - 1) * slices + j + 1 + 1;
			unsigned int bl_index = (i + 0) * slices + j + 1;
			unsigned int br_index = (i + 0) * slices + j + 1 + 1;

			if (j == slices - 1)
			{ // For the seam
				tr_index = (i - 2) * slices + j + 1 + 1;
				br_index = (i - 1) * slices + j + 1 + 1;
			}

			if (i == 0)
			{ // First ring
				out_indices.push_back(top_most_index);
				out_indices.push_back(br_index);
				out_indices.push_back(bl_index);
			}
			else if (i == rings - 2)
			{ // Last ring
				out_indices.push_back(bottom_most_index);
				out_indices.push_back(tl_index);
				out_indices.push_back(tr_index);
			}
			else
			{
				out_indices.push_back(tl_index);
				out_indices.push_back(br_index);
				out_indices.push_back(bl_index);

				out_indices.push_back(tl_index);
				out_indices.push_back(tr_index);
				out_indices.push_back(br_index);
			}
		}
	}

	LOG("Sphere triangle count:" << out_indices.size() / 3);
	return true;
}

sphere create_sphere()
{
	std::vector<float> vertices{};
	std::vector<unsigned int> indices{};

	if (!generate_sort_of_sphere(vertices, indices, 10, 10))
	{
		throw new std::exception{"Failed generating sphere"};
	}

	GLuint vao, vbo, ebo;

	glGenVertexArrays(1, &vao);

	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(float) * vertices.size(), vertices.data(),
	                     0);

	glCreateBuffers(1, &ebo);
	glNamedBufferStorage(ebo, sizeof(unsigned int) * indices.size(),
	                     indices.data(), 0);

	glBindVertexArray(vao);

	glVertexArrayElementBuffer(vao, ebo);

	constexpr int SLOT = 0;
	glVertexAttribFormat(ATTR_POS, 3, GL_FLOAT, false, 0);
	glEnableVertexAttribArray(ATTR_POS);
	glVertexAttribBinding(ATTR_POS, SLOT);
	glBindVertexBuffer(SLOT, vbo, 0, 3 * sizeof(float));

	glBindVertexArray(0);

	return {.vao = vao, .nr_indices = indices.size()};
}

pyramid create_pyramid()
{
	glm::vec3 vertices[]{
		{0, 0, 0}, {1, 0, 0}, {1, 0, -1}, {0, 0, -1}, {0.5f, 1, -0.5f}};
	unsigned int indices[]{
		0, 1, 4, 1, 2, 4, 2, 3, 4, 3, 0, 4, // Sides
		0, 2, 1, 0, 3, 2,                   // Bottom square
	};

	GLuint vao, vbo, ebo;

	glGenVertexArrays(1, &vao);

	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

	glCreateBuffers(1, &ebo);
	glNamedBufferStorage(ebo, sizeof(indices), indices, 0);

	glBindVertexArray(vao);

	glVertexArrayElementBuffer(vao, ebo);

	constexpr int SLOT = 0;
	glVertexAttribFormat(ATTR_POS, 3, GL_FLOAT, false, 0);
	glEnableVertexAttribArray(ATTR_POS);
	glVertexAttribBinding(ATTR_POS, SLOT);
	glBindVertexBuffer(SLOT, vbo, 0, 3 * sizeof(float));

	glBindVertexArray(0);

	return {.vao = vao, .indices = (sizeof(indices) / sizeof(indices[0]))};
}

instanced_cube create_instanced_cubes()
{
	cuboid cube = create_cube(0.5f);

	glm::vec3 instance_offsets[] = {glm::vec3{-0.5f, 0.5f, 0.0f},
	                                glm::vec3{0.5f, -0.5f, 0.0f}};

	GLuint instance_offset_buffer;
	glCreateBuffers(1, &instance_offset_buffer);
	glNamedBufferStorage(instance_offset_buffer, sizeof(instance_offsets),
	                     instance_offsets, 0);

	glBindVertexArray(cube.vao);

	constexpr int iATTR_INST_OFFSET_idx = 1;
	constexpr int SLOT_NEW = 1;
	glVertexAttribFormat(iATTR_INST_OFFSET_idx, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexAttribBinding(iATTR_INST_OFFSET_idx, SLOT_NEW);
	glEnableVertexAttribArray(iATTR_INST_OFFSET_idx);
	glBindVertexBuffer(SLOT_NEW, instance_offset_buffer, 0, 3 * sizeof(float));

	glVertexArrayBindingDivisor(cube.vao, SLOT_NEW, 1);

	glBindVertexArray(0);

	return {.vao = cube.vao};
}

indirect_draw_info create_indirect()
{
#pragma region cube
	glm::vec3 cube_vertices[4 * 2]{{0.0f, 0.0f, 0.0f},
	                               {1.0f, 0.0f, 0.0f},
	                               {0.0f, 1.0f, 0.0f},
	                               {1.0f, 1.0f, 0.0f}};

	for (int i{0}; i < 4; ++i)
	{
		cube_vertices[i + 4] = cube_vertices[i] + glm::vec3{0, 0, -1.0f};
	}

	unsigned int cube_indices[]{
		0, 1, 2, 2, 1, 3, // Front side
		4, 0, 6, 6, 0, 2, // Left side
		1, 5, 3, 3, 5, 7, // Right side
		5, 4, 7, 7, 4, 6, // Back side
		2, 3, 6, 6, 3, 7, // Top side
		4, 5, 0, 0, 5, 1  // Bottom side
	};
#pragma endregion

#pragma region sphere
	std::vector<float> sphere_vertices{};
	std::vector<unsigned int> sphere_indices{};

	if (!generate_sort_of_sphere(sphere_vertices, sphere_indices, 10, 10))
	{
		throw new std::exception{"Failed generating sphere"};
	}
#pragma endregion

#pragma region draw-commands
	constexpr int nr_cube_indices =
		sizeof(cube_indices) / sizeof(cube_indices[0]);
	constexpr int nr_cube_vertices =
		sizeof(cube_vertices) / sizeof(cube_vertices[0]);
	constexpr int nr_cube_instances_to_draw = 1;

	const unsigned int nr_sphere_indices = sphere_indices.size();
	constexpr int nr_sphers_instances_to_draw = 1;
	indirect_command draw_commands[]{
		{
			// Cube
			.count = nr_cube_indices,
			.instance_count = nr_cube_instances_to_draw,
			.first_index = 0, // Offset to the first index stored in the ebo
			.base_vertex = 0, // The offset in the vbo
			.base_instance =
				0, // I like to think of this value as an element offset into
				   // the instance offset buffer. Buut it's actually how much to
				   // add to the result of the instance divisor result.
				   // (gl_InstanceID/divisor) + base_instance
		},
		{
			// Sphere
			.count = nr_sphere_indices,
			.instance_count = nr_sphers_instances_to_draw,
			.first_index =
				nr_cube_indices, // Indices for a cube is stored first
			.base_vertex =
				nr_cube_vertices, // Cube vertices are stored first. This is the
				                  // offset in the vbo to the sphere vertices
			.base_instance = nr_cube_instances_to_draw,
		},
	};
#pragma endregion
	GLuint vao;
	glCreateVertexArrays(1, &vao);

	constexpr int nr_handles = 4;
	GLuint buffer_handles[nr_handles];
	GLuint &vbo = buffer_handles[0];
	GLuint &ebo = buffer_handles[1];
	GLuint &instance_offsets = buffer_handles[2];
	GLuint &indirect_commands = buffer_handles[3];

	glCreateBuffers(nr_handles, buffer_handles);

	constexpr unsigned int vbo_slot{0}, offsets_slot{1};

	// vertices
	glNamedBufferStorage(vbo,
	                     sizeof(cube_vertices) + sizeof(sphere_vertices[0]) *
	                                                 sphere_vertices.size(),
	                     nullptr, GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayVertexBuffer(vao, vbo_slot, vbo, 0, 3 * sizeof(float));
	glNamedBufferSubData(vbo, 0, sizeof(cube_vertices), cube_vertices);
	glNamedBufferSubData(vbo, sizeof(cube_vertices),
	                     sphere_vertices.size() * sizeof(sphere_vertices[0]),
	                     sphere_vertices.data());

	glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, 0);
	glEnableVertexArrayAttrib(vao, ATTR_POS);
	glVertexArrayAttribBinding(vao, ATTR_POS, vbo_slot);

	// indices
	glNamedBufferStorage(ebo,
	                     sizeof(cube_indices) +
	                         sphere_indices.size() * sizeof(sphere_indices[0]),
	                     nullptr, GL_DYNAMIC_STORAGE_BIT);

	glVertexArrayElementBuffer(vao, ebo);

	glNamedBufferSubData(ebo, 0, sizeof(cube_indices), cube_indices);
	glNamedBufferSubData(ebo, sizeof(cube_indices),
	                     sphere_indices.size() * sizeof(sphere_indices[0]),
	                     sphere_indices.data());

	// instance offsets
	glm::vec3 offsets[] = {glm::vec3{0}, glm::vec3{-3.75f, 1.643f, 0}};
	glNamedBufferStorage(instance_offsets, sizeof(offsets), offsets, 0);

	glVertexArrayVertexBuffer(vao, offsets_slot, instance_offsets, 0,
	                          3 * sizeof(float));

	glVertexArrayAttribFormat(vao, ATTR_INST_OFFSET, 3, GL_FLOAT, false, 0);
	glEnableVertexArrayAttrib(vao, ATTR_INST_OFFSET);
	glVertexArrayAttribBinding(vao, ATTR_INST_OFFSET, offsets_slot);

	glVertexArrayBindingDivisor(vao, offsets_slot, 1);

	// draw commands
	glNamedBufferStorage(indirect_commands, sizeof(draw_commands),
	                     draw_commands, 0);

	return {.vao = vao,
	        .commands_handle = indirect_commands,
	        .draw_count = sizeof(draw_commands) / sizeof(draw_commands[0]),
	        .command_stride = sizeof(draw_commands[0])};
}

// @NOTE: Maybe it's better to return a list of indirect commands, so each
// sub-mesh could be its own command, because assimp resets the index per
// sub-mesh anyways
std::optional<indirect_command>
load_asset(const char *file, std::vector<glm::vec3> &out_vertices,
           std::vector<unsigned int> &out_indices)
{
	Assimp::Importer importer;
	// @Warning: Triangulate doesn't work on points and lines
	constexpr unsigned int process_flags =
		aiProcess_Triangulate | aiProcess_OptimizeMeshes |
		aiProcess_JoinIdenticalVertices | aiProcess_FindInvalidData |
		aiProcess_ValidateDataStructure | aiProcess_PreTransformVertices;
	if (!importer.ValidateFlags(process_flags))
	{
		std::cerr << "process flags for importing are invalid" << std::endl;
		return {};
	}

	const aiScene *scene_node = importer.ReadFile(file, process_flags);
	std::string error = importer.GetErrorString();
	if (scene_node == nullptr)
	{
		std::cerr << "Error reading file: " << error << std::endl;
		return {};
	}

	unsigned int index_count{0};
	unsigned int first_index{(unsigned int)out_indices.size()};
	int base_vertex{(int)out_vertices.size()};

	int model_base_index{0};
	for (int mesh_idx{0}; mesh_idx < scene_node->mNumMeshes; ++mesh_idx)
	{
		aiMesh *ai_mesh = scene_node->mMeshes[mesh_idx];
		index_count += ai_mesh->mNumFaces * 3;

		// Need this before resize so we know where to copy the new mem to
		int vertex_offset = out_vertices.size();
		int index_offset = out_indices.size();

		out_vertices.resize(out_vertices.size() +
		                    (size_t)ai_mesh->mNumVertices);
		out_indices.resize(out_indices.size() + ai_mesh->mNumFaces * 3);

		std::memcpy(out_vertices.data() + vertex_offset, ai_mesh->mVertices,
		            ai_mesh->mNumVertices * sizeof(out_vertices[0]));

		for (int face_idx{0}; face_idx < ai_mesh->mNumFaces; ++face_idx)
		{
			for (int index{0}; index < 3; ++index)
			{
				aiFace face = ai_mesh->mFaces[face_idx];
				out_indices[index_offset + face_idx * 3 + index] =
					model_base_index + face.mIndices[index];
			}
		}

		// If it has multiple meshes we need to count to offset the index value
		model_base_index += ai_mesh->mNumVertices;
	}

	constexpr int instance_count = 1;
	indirect_command command_info{index_count, instance_count, first_index,
	                              base_vertex, 0};

	return command_info;
}

dynamic_multi_draw upload_objects(std::span<indirect_command> commands,
                                  std::span<glm::vec3> vertices,
                                  std::span<GLuint> indices,
                                  std::optional<std::span<glm::vec3>> offsets)
{
	GLuint vao;
	glCreateVertexArrays(1, &vao);

	constexpr int nr_handles = 4;
	GLuint buffer_handles[nr_handles];
	GLuint &vbo = buffer_handles[0];
	GLuint &ebo = buffer_handles[1];
	GLuint &instance_offsets = buffer_handles[2];
	GLuint &indirect_commands = buffer_handles[3];
	glCreateBuffers(nr_handles, buffer_handles);

	constexpr unsigned int vbo_slot{0}, offsets_slot{1};

	// 0 = can't interact with it again from client side.
	constexpr GLbitfield no_client_side_access = 0;

	// vertices
	glNamedBufferStorage(vbo, vertices.size_bytes(), vertices.data(),
	                     no_client_side_access);

	constexpr GLintptr position_offset = 0;
	constexpr GLint positions_per_vertex = 3;
	constexpr GLsizei position_stride = positions_per_vertex * sizeof(float);
	glVertexArrayVertexBuffer(vao, vbo_slot, vbo, position_offset,
	                          position_stride);

	glEnableVertexArrayAttrib(vao, ATTR_POS);
	glVertexArrayAttribFormat(vao, ATTR_POS, positions_per_vertex, GL_FLOAT,
	                          GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, ATTR_POS, vbo_slot);

	// indices
	glNamedBufferStorage(ebo, indices.size_bytes(), indices.data(),
	                     no_client_side_access);
	glVertexArrayElementBuffer(vao, ebo);

	if (offsets.has_value())
	{
		glNamedBufferStorage(instance_offsets, offsets->size_bytes(),
		                     offsets->data(), 0);
		glVertexArrayVertexBuffer(vao, offsets_slot, instance_offsets, 0,
		                          3 * sizeof(float));

		constexpr GLuint divisor = 1; // Update offset every new instance
		glVertexArrayBindingDivisor(vao, offsets_slot, divisor);

		glVertexArrayAttribFormat(vao, ATTR_INST_OFFSET, 3, GL_FLOAT, false, 0);
		glEnableVertexArrayAttrib(vao, ATTR_INST_OFFSET);
		glVertexArrayAttribBinding(vao, ATTR_INST_OFFSET, offsets_slot);
	}

	// upload draw commands
	glNamedBufferStorage(indirect_commands, commands.size_bytes(),
	                     commands.data(),
	                     GL_DYNAMIC_STORAGE_BIT); // Dynamic so the client can
	                                              // change the draw commands

	return {.vao = vao, .command_buffer_handle = indirect_commands};
}

bool load_obj(const char *file)
{
	LOG("Load obj file: " << file);

	std::ifstream file_stream{file, std::ios::binary};
	if (!file_stream.is_open())
	{
		std::cerr << "Failed to open obj file: " << file << std::endl;
		return false;
	}

	// Create file buffer window to avoid storing the whole file in mem if big
	uintmax_t file_size = std::filesystem::file_size(file);
	constexpr size_t cache_line_size = 64;
	constexpr size_t buf_size = cache_line_size;
	char file_window_buffer[cache_line_size];

	size_t buf_base{0};
	size_t size_to_fill = buf_size - buf_base;
	file_stream.read(file_window_buffer + buf_base, size_to_fill);
	// returns how much it managed to read, we try to read size_to_fill, but it
	// can't read that many if it encountered the file end
	bool reached_eof = file_stream.gcount() < size_to_fill;

	char *first{0}, second{0};
	char c{}; // @Note: Can probably remove this later if you want, but might be
	          // nice to have when debugging

	while (second < buf_size && (c = file_window_buffer[second++]) != 'o')
	{
	}

	std::string_view buffer_view{file_window_buffer,
	                             file_window_buffer + second};

	LOG("Found: " << c << "Buffer: " << buffer_view);

	return true;
}

} // namespace statics
