#include "renderer.h"
#include "../glslSemantics.h"
#include "../macros.h"
#include "../util/util.h"
#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <glm/mat4x4.hpp>
#include <stb_image.h>
#include <string>

#ifdef DEBUG
#include <iostream>
#endif

bool renderer::create_program(const std::string_view glsl_semantics,
                              const char *vertex_shader_path,
                              const char *fragment_shader_path,
                              uint32_t &out_program)
{
	GLuint vertex_shader, fragment_shader, geometry_shader;

	if (!renderer::create_shader(&vertex_shader, vertex_shader_path,
	                             GL_VERTEX_SHADER, glsl_semantics) ||
	    !renderer::create_shader(&fragment_shader, fragment_shader_path,
	                             GL_FRAGMENT_SHADER, glsl_semantics))
	{
		return false;
	}

	out_program = glCreateProgram();

	glAttachShader(out_program, vertex_shader);
	glAttachShader(out_program, fragment_shader);

	glLinkProgram(out_program);

	int success;
	char info_log[LOG_SIZE];
	glGetProgramiv(out_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(out_program, sizeof(info_log), nullptr, info_log);
		LOG("Failed to compile shader program: " << info_log);
		return false;
	}

	glDetachShader(out_program, vertex_shader);
	glDetachShader(out_program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	return true;
}

bool renderer::create_shader(unsigned int *out_shader, const char *shader_path,
                             const unsigned long type,
                             const std::string_view glsl_semantics)
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
	if (!renderer::could_compile(shader, shader_path))
	{
		return false;
	}

	return true;
}

bool renderer::could_compile(unsigned int shader, const char *shader_path)
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

void renderer::set_perspective(matrix_ubo &ubo, matrix44 perspective)
{
	glNamedBufferSubData(ubo.handle, 0, sizeof(matrix44), &perspective);
}

void renderer::set_view(matrix_ubo &ubo, matrix44 view)
{
	glNamedBufferSubData(ubo.handle, M44Size, sizeof(matrix44), &view);
}

renderer::matrix_ubo renderer::test()
{
	GLuint ubo;
	glCreateBuffers(1, &ubo);
	matrix_ubo mubo{.handle = ubo};
	glNamedBufferStorage(ubo, sizeof(mubo.opaque), nullptr,
	                     GL_DYNAMIC_STORAGE_BIT);

	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MATRICES, ubo);

	return mubo;
}

uint32_t renderer::create_matrix_ubo(void *initial_data, size_t bytes)
{
	GLuint ubo;
	glCreateBuffers(1, &ubo);
	glNamedBufferStorage(ubo, bytes, initial_data, GL_DYNAMIC_STORAGE_BIT);

	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MATRICES, ubo);

	return ubo;
}

bool renderer::create_texture(const char *texture_path, uint32_t &out_texture)
{
	int width, height, nr_channels;
	unsigned char *tex_data =
		stbi_load(texture_path, &width, &height, &nr_channels, 0);

	if (tex_data == nullptr)
	{
		LOG("Couldn't load texture at: " << texture_path);
		return false;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &out_texture);

	glTextureParameteri(out_texture, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(out_texture, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(out_texture, GL_TEXTURE_MIN_FILTER,
	                    GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(out_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

	glTextureStorage2D(out_texture, 1, format, width, height);
	glTextureSubImage2D(out_texture, 0, 0, 0, width, height, GL_RGB,
	                    GL_UNSIGNED_BYTE, tex_data);

	return true;
}
