#pragma once

#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
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

	GLuint vao, shader_program, vertex_shader, fragment_shader;

	glm::vec2 triangle_size{0.5f, 0.5f};
	constexpr float cube_size = 0.5f;
	auto cube = statics::create_cube(cube_size);

	const char *vertex_shader_path{"shader/basic.vert"};
	const char *fragment_shader_path{"shader/basic.frag"};
	if (!statics::create_shader(&vertex_shader, vertex_shader_path,
	                            GL_VERTEX_SHADER) ||
	    !statics::create_shader(&fragment_shader, fragment_shader_path,
	                            GL_FRAGMENT_SHADER) ||
	    !statics::create_shader_program(&shader_program, vertex_shader,
	                                    fragment_shader))
	{
		return -1;
	}

	glDetachShader(shader_program, vertex_shader);
	glDetachShader(shader_program, fragment_shader);

	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glfwSetFramebufferSizeCallback(window,
	                               [](GLFWwindow *window, int width, int height)
	                               { glViewport(0, 0, width, height); });

	glClearColor(1, 1, 1, 1);

	constexpr float roof_swap_cooldown = 0.2f;
	double last_roof_swap_time{-roof_swap_cooldown};

#pragma region view
	glm::vec3 view_pos{0, 0, 3.0f};
	glm::vec3 view_target{0, 0, 0};
	glm::vec3 view_up_axis{0, 1, 0};
	glm::mat4 view = glm::lookAt(view_pos, view_target, view_up_axis);
#pragma endregion

#pragma region uniform-matrices-buffer
	glm::mat4 projection = glm::perspective(
		glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f,
		100.0f);

	GLuint uniform_matrices_object;
	glCreateBuffers(1, &uniform_matrices_object);
	glNamedBufferStorage(uniform_matrices_object, 2 * sizeof(glm::mat4),
	                     nullptr, GL_DYNAMIC_STORAGE_BIT);

	glNamedBufferSubData(uniform_matrices_object, 0, sizeof(glm::mat4),
	                     &projection[0]);

	glNamedBufferSubData(uniform_matrices_object, sizeof(glm::mat4),
	                     sizeof(glm::mat4), &view[0]);

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniform_matrices_object);
#pragma endregion

#pragma region cube_uniforms
	glUseProgram(shader_program);

	glm::mat4 cube_model{1};
	cube_model = glm::rotate(cube_model, 3.14f / 4.0f, glm::vec3(1, 1, 0));
	int model_location = glGetUniformLocation(shader_program, "model");
	glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(cube_model));

	int color_location = glGetUniformLocation(shader_program, "color");
	glUniform3fv(color_location, 1, &glm::vec3{1, 0, 1}[0]);
#pragma endregion

	glm::vec3 roof_color1{1, 0, 0}, roof_color2{0, 1, 0};
	glm::vec3 roof_color = roof_color1;
	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			// this would close after this frame, is that what we want?
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE))
		{
			if ((glfwGetTime() - last_roof_swap_time) > roof_swap_cooldown)
			{
				LOG("Space!");

				roof_color =
					roof_color == roof_color1 ? roof_color2 : roof_color1;

				last_roof_swap_time = glfwGetTime();
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		// statics::draw_triangle(shader_program, cube.vao);
		glBindVertexArray(cube.vao);
		glUseProgram(shader_program);
		constexpr void *offset = (void *)0;
		constexpr int indices = 6 * 6;
		glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, offset);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
