#include "statics.h"
#include "macros.h"

#include <fstream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <iterator>
#include <string>

namespace statics
{
bool read_file(const char *file_path, std::string &out_content)
{
#ifdef MINE_LOG
	std::cout << "Opening file" << std::endl;
#endif

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
		std::cerr << "Failed to compile shader: " << shader_path << std::endl;

		return false;
	}

#ifdef MINE_LOG
	std::cout << "Successfully compiled: " << shader_path << std::endl;
#endif

	return true;
}

bool create_shader(unsigned int *out_shader, const char *shader_path,
                   const unsigned long type)
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

	shader_str = GLSL_HEADER + shader_str;

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
                           const unsigned int fragment_shader)
{
	*out_shader_program = glCreateProgram();
	unsigned int program = *out_shader_program;

	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);
	glLinkProgram(program);

	int success;
	char info_log[LOG_SIZE];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(program, sizeof(info_log), nullptr, info_log);
		std::cerr << "Failed to compile shader program" << std::endl;
		return false;
	}

	return true;
}

void create_triangle_vao(unsigned int *out_vao, const glm::vec2 size)
{
	float vertices[]{0.0f, 0.0f, 0.0f, 0.0f, size.y, 0.0f, size.x, 0.0f, 0.0f};

	glGenVertexArrays(1, out_vao);

	GLuint vbo, vao{*out_vao};

	// Create and get buffer handle
	// then 'upload' vertex data. 0 as flag because it shouldn't be read/written
	// to from cpu again
	glCreateBuffers(1, &vbo);
	glNamedBufferStorage(vbo, sizeof(vertices), vertices, 0);

	glBindVertexArray(vao);

	constexpr int POS_ATTRIB = 0;
	constexpr int SLOT = 0;
	// Set attribute format
	// Enable attribute
	// Set slot to use
	// Define slot
	glVertexAttribFormat(POS_ATTRIB, 3, GL_FLOAT, false, 0);
	glEnableVertexAttribArray(POS_ATTRIB);
	glVertexAttribBinding(POS_ATTRIB, SLOT);
	glBindVertexBuffer(SLOT, vbo, 0, 3 * sizeof(float));

	glBindVertexArray(0);
}

void draw_triangle(const unsigned int program, const unsigned int vao)
{
	glUseProgram(program);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void draw_rectangle(const unsigned int program, const unsigned int vao,
                    const glm::vec2 triangle_size)
{
	int offset_location = glGetUniformLocation(program, "offset");
	glUseProgram(program);

	glm::vec2 offset{0, 0};
	glUniform2fv(offset_location, 1, &offset[0]);

	int scale_location = glGetUniformLocation(program, "scale");
	glm::vec2 scale{1.f, 1.f};
	glUniform2fv(scale_location, 1, &scale[0]);

	int color_location = glGetUniformLocation(program, "color");
	glm::vec3 color{1.f, 1.f, 1.f};
	glUniform3fv(color_location, 1, &color[0]);

	draw_triangle(program, vao);

	glUniform2fv(scale_location, 1, &glm::vec2{-1.f, -1.f}[0]);
	glUniform2fv(offset_location, 1, &glm::vec2{triangle_size * -1.0f}[0]);
	glUniform3fv(color_location, 1, &glm::vec3{0.15f, 0.65f, 0.84f}[0]);

	draw_triangle(program, vao);
}

} // namespace statics
