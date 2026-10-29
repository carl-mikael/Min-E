#pragma once

#include "macros.h"
#include "statics.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>

int main(int argc, char **argv)
{
	std::cout << "hello world!" << std::endl;
	std::cout << "GLSL Header: " << GLSL_HEADER << std::endl;

	if (!glfwInit())
	{
		std::cerr << "Failed Init" << std::endl;
		return -1;
	}

	constexpr int WINDOW_WIDTH = 800;
	constexpr int WINDOW_HEIGHT = 600;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR_V);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR_V);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window =
		glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "MinE", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed creating window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

#pragma region triangle
#ifdef MINE_LOG
	std::cout << "Making Triangle" << std::endl;
#endif

	float vertices[]{0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f};

	GLuint vao, vbo;

	glGenVertexArrays(1, &vao);

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

	unsigned int vertex_shader, fragment_shader, shader_program;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	const char *vertex_shader_path = "shader/basic.vs";
	const char *fragment_shader_path = "shader/basic.fs";

	std::string vertex_shader_str{};
	if (!statics::read_file(vertex_shader_path, vertex_shader_str))
	{
		return -1;
	}

	vertex_shader_str = GLSL_HEADER + vertex_shader_str;

	std::string fragment_shader_str{};
	if (!statics::read_file(fragment_shader_path, fragment_shader_str))
	{
		return -1;
	}
	fragment_shader_str = GLSL_HEADER + fragment_shader_str;

	const char *vertex_shader_src = vertex_shader_str.c_str();
	const char *fragment_shader_src = fragment_shader_str.c_str();

	glShaderSource(vertex_shader, 1, &vertex_shader_src, nullptr);
	glCompileShader(vertex_shader);
	if (!statics::could_compile(vertex_shader, vertex_shader_path))
	{
		return -1;
	}

	glShaderSource(fragment_shader, 1, &fragment_shader_src, nullptr);
	glCompileShader(fragment_shader);
	if (!statics::could_compile(fragment_shader, fragment_shader_path))
	{
		return -1;
	}

	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);

	int success;
	char info_log[512];
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shader_program, sizeof(info_log), nullptr,
		                    info_log);
		std::cerr << "Failed to compile shader program" << std::endl;
		return -1;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
#pragma endregion

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetFramebufferSizeCallback(window,
	                               [](GLFWwindow *window, int width, int height)
	                               { glViewport(0, 0, width, height); });

	glClearColor(.85, 0.2, 0.205, 1);

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			// this would close after this frame, is that what we want?
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE))
		{
			std::cout << "Space!" << std::endl;
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shader_program);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
