#include "renderer.h"
#include "../glslSemantics.h"
#include "../macros.h"
#include "../util/util.h"
#include <bit>
#include <cstdint>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <stb_image.h>
#include <string>

#ifdef DEBUG
#include <iostream>
#endif

#define M44Size 64

namespace
{
bool could_compile(unsigned int shader, const char *shader_path) noexcept
{
	int success;
	char info_log[LOG_SIZE];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);
		LOG("Failed to compile shader: " << shader_path
		                                 << " info_log: " << info_log);

		return false;
	}

	return true;
}

bool set_unused_slot(renderer::slots &slots, renderer::slots &out_slot) noexcept
{
	uint32_t slots_left = std::countl_zero(slots.used);
	if (slots_left == 0)
	{
		LOG("No free slot, setting out_slot to 0");
		out_slot.used = 0;
		return false;
	}

	LOG("slots_left: " << slots_left);
	out_slot.used = 16 - slots_left;
	uint32_t mask = uint32_t{1} << out_slot.used;
	LOG("Taking slot: " << out_slot.used << " (mask: " << mask << ")");

	slots.used |= mask;
	return true;
}

bool create_shader(unsigned int *out_shader, const char *shader_path,
                   const unsigned long type,
                   const std::string_view &glsl_semantics) noexcept
{
	if (!out_shader)
	{
		LOG("out_shader is nullptr, returning");
		return false;
	}

	*out_shader = glCreateShader(type);
	unsigned int shader = *out_shader;

	std::string shader_str{};
	if (!util::read_file(shader_path, shader_str))
	{
		return false;
	}

	std::string semantics_str{glsl_semantics};
	shader_str = GLSL_HEADER(semantics_str) + shader_str;

	const char *shader_src = shader_str.c_str();

	glShaderSource(shader, 1, &shader_src, nullptr);
	glCompileShader(shader);
	if (!could_compile(shader, shader_path))
	{
		return false;
	}

	return true;
}
} // namespace

bool renderer::create_program(const std::string_view &glsl_semantics,
                              const char *vertex_shader_path,
                              const char *fragment_shader_path,
                              shader_program &out_program) noexcept
{
	GLuint vertex_shader, fragment_shader;

	if (!create_shader(&vertex_shader, vertex_shader_path, GL_VERTEX_SHADER,
	                   glsl_semantics) ||
	    !create_shader(&fragment_shader, fragment_shader_path,
	                   GL_FRAGMENT_SHADER, glsl_semantics))
	{
		return false;
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	glLinkProgram(program);

	int success;
	char info_log[LOG_SIZE];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
		LOG("Failed to compile shader program: " << info_log);
		return false;
	}

	glDetachShader(program, vertex_shader);
	glDetachShader(program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	out_program.program = program;
	return true;
}

void renderer::upload_projection(camera_ubo mubo, const float *m44) noexcept
{
	glNamedBufferSubData(mubo.handle, 0, M44Size, m44);
}

void renderer::upload_view(camera_ubo mubo, const float *m44) noexcept
{
	glNamedBufferSubData(mubo.handle, M44Size, M44Size, m44);
}

renderer::camera_ubo renderer::create_matrix_ubo() noexcept
{
	GLuint ubo;
	glCreateBuffers(1, &ubo);
	camera_ubo mubo{.handle = ubo};
	glNamedBufferStorage(ubo, 2 * M44Size, nullptr, GL_DYNAMIC_STORAGE_BIT);

	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MATRICES, ubo);

	return mubo;
}

bool renderer::create_texture(const char *texture_path,
                              texture_data &out_texture_info) noexcept
{
	int width, height, nr_channels;
	unsigned char *tex_data =
		stbi_load(texture_path, &width, &height, &nr_channels, 0);

	if (tex_data == nullptr)
	{
		LOG("Couldn't load texture at: " << texture_path);
		return false;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &out_texture_info.handle);

	glTextureParameteri(out_texture_info.handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(out_texture_info.handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(out_texture_info.handle, GL_TEXTURE_MIN_FILTER,
	                    GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(out_texture_info.handle, GL_TEXTURE_MAG_FILTER,
	                    GL_LINEAR);

	GLenum format;
	if (nr_channels == 1)
	{
		format = GL_RED;
	}
	else if (nr_channels == 3)
	{
		format = GL_RGB8;
	}
	else if (nr_channels == 4)
	{
		format = GL_RGBA;
	}
	else
	{
		format = GL_RED;
		LOG("Unknown format with ' " << nr_channels << "' using GL_RED");
	}

	glTextureStorage2D(out_texture_info.handle, 1, format, width, height);
	glTextureSubImage2D(out_texture_info.handle, 0, 0, 0, width, height, GL_RGB,
	                    GL_UNSIGNED_BYTE, tex_data);

	return true;
}

void renderer::bind_texture(const texture_data &texture_info) noexcept
{
	glBindTextureUnit(TEX2D, texture_info.handle);
}

bool renderer::upload_objects(const float_view &vertices,
                              const indices_view &indices, slots &used_slots,
                              handle &out_handle) noexcept
{
	GLuint vao;
	glCreateVertexArrays(1, &vao);

	constexpr int nr_handles = 2;
	GLuint buffer_handles[nr_handles];
	GLuint &vbo = buffer_handles[0];
	GLuint &ebo = buffer_handles[1];
	glCreateBuffers(nr_handles, buffer_handles);

	slots slot;
	if (!set_unused_slot(used_slots, slot))
	{
		LOG("No free slot, returning false");
		return false;
	}
	LOG("USING SLOT: " << slot.used);

	// 0 = can't interact with it again from client side.
	constexpr GLbitfield no_client_side_access = GL_DYNAMIC_STORAGE_BIT;

	// vertices
	glNamedBufferStorage(vbo, vertices.bytes, vertices.data,
	                     no_client_side_access);

	constexpr GLintptr position_offset = 0;
	constexpr GLsizei stride = 8 * sizeof(float);
	glVertexArrayVertexBuffer(vao, slot.used, vbo, position_offset, stride);

	glEnableVertexArrayAttrib(vao, ATTR_POS);
	glVertexArrayAttribFormat(vao, ATTR_POS, 3, GL_FLOAT, GL_FALSE, 0);
	glVertexArrayAttribBinding(vao, ATTR_POS, slot.used);

	glEnableVertexArrayAttrib(vao, ATTR_NORMAL);
	glVertexArrayAttribFormat(vao, ATTR_NORMAL, 3, GL_FLOAT, GL_FALSE,
	                          3 * sizeof(float));
	glVertexArrayAttribBinding(vao, ATTR_NORMAL, slot.used);

	glEnableVertexArrayAttrib(vao, ATTR_UV);
	glVertexArrayAttribFormat(vao, ATTR_UV, 2, GL_FLOAT, GL_FALSE,
	                          6 * sizeof(float));
	glVertexArrayAttribBinding(vao, ATTR_UV, slot.used);

	// indices
	glNamedBufferStorage(ebo, indices.bytes, indices.data,
	                     no_client_side_access);
	glVertexArrayElementBuffer(vao, ebo);

	out_handle.id = vao;
	out_handle.used = slot;
	return true;
}

renderer::commands_handle
renderer::upload_draw_commands(const command_view &commands) noexcept
{
	// upload draw commands
	GLuint indirect_commands;
	glCreateBuffers(1, &indirect_commands);
	glNamedBufferStorage(indirect_commands, commands.bytes, commands.data,
	                     GL_DYNAMIC_STORAGE_BIT);

	return {.handle = indirect_commands, .capacity = commands.bytes};
}

bool renderer::update_draw_commands(
	const command_view &commands, const size_t bytes_offset,
	const renderer::commands_handle &commands_handle) noexcept
{
	if (commands.bytes > commands_handle.capacity)
	{
		LOG("Can't fit that");
		return false;
	}

	glNamedBufferSubData(commands_handle.handle, bytes_offset, commands.bytes,
	                     commands.data);
	return true;
}

void renderer::use_draw_commands(const commands_handle &handle) noexcept
{
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, handle.handle);
}

bool renderer::upload_matrices(const float_view &models,
                               matrices_handle &out_handle) noexcept
{
	GLuint models_handle;
	glCreateBuffers(1, &models_handle);
	// Need nullptr when allocating more than that's being uploaded.
	// Otherwise it keeps reading past the data end of the data.
	glNamedBufferStorage(models_handle, UBO_MAX_SIZE, nullptr,
	                     GL_DYNAMIC_STORAGE_BIT);

	if (models.bytes > UBO_MAX_SIZE)
	{
		LOG("Can't fit that");
		return false;
	}

	glNamedBufferSubData(models_handle, 0, models.bytes, models.data);

	out_handle.handle = models_handle;
	out_handle.capacity = UBO_MAX_SIZE;
	out_handle.size = models.bytes;

	return true;
}

bool renderer::update_matrices(const matrices_handle &models_handle,
                               const size_t byte_offset,
                               const float_view &new_data) noexcept
{
	const size_t end = byte_offset + new_data.bytes;
	if (end > models_handle.capacity)
	{
		LOG("This would write over capactity");
		return false;
	}

	glNamedBufferSubData(models_handle.handle, byte_offset, new_data.bytes,
	                     new_data.data);

	return true;
}

void renderer::use_matrices(const matrices_handle &models_handle) noexcept
{
	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MODELS, models_handle.handle);
}

bool renderer::add_instance_offset_attr(
	const handle &handle, slots &slots_used,
	slots &out_instance_offsets_slot) noexcept
{
	slots slot;
	if (!set_unused_slot(slots_used, slot))
	{
		LOG("No free slot returning");
		return false;
	}

	glVertexArrayAttribFormat(handle.id, ATTR_INST_OFFSET, 3, GL_FLOAT, false,
	                          0);
	glVertexArrayBindingDivisor(handle.id, slot.used, 1);
	glEnableVertexArrayAttrib(handle.id, ATTR_INST_OFFSET);
	glVertexArrayAttribBinding(handle.id, ATTR_INST_OFFSET, slot.used);

	out_instance_offsets_slot = slot;
	return true;
}

bool renderer::upload_instance_offsets(
	const float_view &offsets, instance_offsets_handle &out_handle) noexcept
{
	GLuint instance_offsets;
	glCreateBuffers(1, &instance_offsets);
	glNamedBufferStorage(instance_offsets, offsets.bytes, offsets.data, 0);
	out_handle.handle = instance_offsets;
	return true;
}

void renderer::use_instance_offsets(
	const handle &handle, const slots &instance_offset_slot,
	const instance_offsets_handle &instance_offsets_handle) noexcept
{
	glVertexArrayVertexBuffer(handle.id, instance_offset_slot.used,
	                          instance_offsets_handle.handle, 0,
	                          3 * sizeof(float));
}
