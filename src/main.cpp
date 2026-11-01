#pragma once

#include "glm/ext/vector_float3.hpp"
#include "macros.h"
#include "statics.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

int main(int argc, char **argv)
{
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

#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

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
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

#ifdef DEBUG
// Yoinked from: https://learnopengl.com/In-Practice/Debugging
#pragma region debug_callback
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
		                      nullptr, GL_TRUE);
	}
#pragma endregion
#endif

#pragma region triangle
	GLuint vao, shader_program, vertex_shader, fragment_shader;

	glm::vec2 triangle_size{0.5f, 0.5f};
	auto basic_triangle = statics::create_triangle(triangle_size);

	const char *vertex_shader_path{"shader/basic.vs"};
	const char *fragment_shader_path{"shader/basic.fs"};
	if (!statics::create_shader(&vertex_shader, vertex_shader_path,
	                            GL_VERTEX_SHADER) ||
	    !statics::create_shader(&fragment_shader, fragment_shader_path,
	                            GL_FRAGMENT_SHADER) ||
	    !statics::create_shader_program(&shader_program, vertex_shader,
	                                    fragment_shader))
	{
		return -1;
	}

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);
#pragma endregion

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetFramebufferSizeCallback(window,
	                               [](GLFWwindow *window, int width, int height)
	                               { glViewport(0, 0, width, height); });

	glClearColor(1, 1, 1, 1);

	glm::vec3 roof_color{1, 0, 0};

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			// this would close after this frame, is that what we want?
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE))
		{
			LOG("Space!");
			roof_color = {0, 1, 0};
		}

		glClear(GL_COLOR_BUFFER_BIT);

		statics::draw_house(shader_program, basic_triangle, roof_color);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
