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

	GLuint vao, shader_program, vertex_shader, fragment_shader;

	statics::create_triangle_vao(&vao);

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

		statics::draw_triangle(shader_program, vao);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
