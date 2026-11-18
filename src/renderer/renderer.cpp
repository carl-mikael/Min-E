#include "renderer.h"
#include "../macros.h"
#include "../statics.h"
#include <glad/glad.h>
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
	if (!statics::read_file(shader_path, shader_str))
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
