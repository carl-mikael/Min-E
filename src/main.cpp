#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glslSemantics.h"
#include "macros.h"
#include "statics.h"
#include <GLFW/glfw3.h>
#include <cmath>
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

	GLuint shader_program, vertex_shader, fragment_shader, geometry_shader;

	constexpr float cube_size = 0.5f;
	auto cube = statics::create_cube(cube_size);
	auto sphere = statics::create_sphere();
	auto pyramid = statics::create_pyramid();

	std::string glsl_semantics{};
	statics::read_file(GLSL_SEMANTICS_STR, glsl_semantics);

	const char *vertex_shader_path{"shader/basic.vert"};
	const char *fragment_shader_path{"shader/basic.frag"};

	if (!statics::create_shader(&vertex_shader, vertex_shader_path,
	                            GL_VERTEX_SHADER, glsl_semantics) ||
	    !statics::create_shader(&fragment_shader, fragment_shader_path,
	                            GL_FRAGMENT_SHADER, glsl_semantics) ||
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

	// Breaks aspect ratio
	glfwSetFramebufferSizeCallback(window,
	                               [](GLFWwindow *window, int width, int height)
	                               { glViewport(0, 0, width, height); });

	glClearColor(1, 1, 1, 1);

#pragma region view
	glm::vec3 view_pos{0, 0, 3.0f};
	glm::vec3 view_target{0, 0, 0};
	glm::vec3 view_up_axis{0, 1, 0};
	glm::mat4 camera = glm::lookAt(view_pos, view_target, view_up_axis);
#pragma endregion

#pragma region uniform-matrices-buffer
	constexpr float aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;

	constexpr float near{0.1f}, far{100.0f};
	glm::mat4 perspective =
		glm::perspective(glm::radians(45.0f), aspect, near, far);

	constexpr float ortho_near_height = 5.0f;
	constexpr float ortho_near_width = ortho_near_height * aspect;
	glm::mat4 orthographic =
		glm::ortho(-ortho_near_width, ortho_near_width, -ortho_near_height,
		           ortho_near_height, near, far);

	GLuint uniform_matrices_object;
	glCreateBuffers(1, &uniform_matrices_object);

	struct
	{
		alignas(16) glm::mat4 projection;
		alignas(16) glm::mat4 view;
	} ubo_data;

	ubo_data.projection = perspective;
	ubo_data.view = camera;

	glNamedBufferStorage(uniform_matrices_object, 2 * sizeof(glm::mat4),
	                     &ubo_data, GL_DYNAMIC_STORAGE_BIT);

	// This is the binding index specified in the uniform block which must match
	// each other
	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MATRICES,
	                 uniform_matrices_object);
#pragma endregion

	int model_location = glGetUniformLocation(shader_program, "model");
	int color_location = glGetUniformLocation(shader_program, "color");

	glm::vec3 color = glm::vec3{0.5f, 0.f, 0.945f};
	glProgramUniform3fv(shader_program, color_location, 1, &color[0]);

	glm::mat4 cube_model{1};
	cube_model = glm::translate(cube_model, glm::vec3{-0.25f, 0, 0});

	glm::mat4 sphere_model{1};
	sphere_model = glm::scale(sphere_model, glm::vec3{0.25});
	sphere_model = glm::translate(sphere_model, glm::vec3{-4.f, 0, 0});

	glm::mat4 original_pyramid_model{1};
	original_pyramid_model =
		glm::scale(original_pyramid_model, glm::vec3{0.5f});
	original_pyramid_model =
		glm::translate(original_pyramid_model, glm::vec3{1.85f, -0.25f, 0});

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	bool line_mode = false;

	constexpr float space_cooldown = 0.2f;
	double last_space_click{-space_cooldown};
	constexpr float f_cooldown = 0.2f;
	double last_f_click{-f_cooldown};

	glUseProgram(shader_program);

	while (!glfwWindowShouldClose(window))
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			// this would close after this frame, is that what we want?
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE))
		{
			if ((glfwGetTime() - last_space_click) > space_cooldown)
			{
				LOG("Space!");

				ubo_data.projection = ubo_data.projection == perspective
				                          ? orthographic
				                          : perspective;
				glNamedBufferSubData(uniform_matrices_object, 0,
				                     sizeof(glm::mat4),
				                     &ubo_data.projection[0]);

				last_space_click = glfwGetTime();
			}
		}
		if (glfwGetKey(window, GLFW_KEY_F))
		{
			if ((glfwGetTime() - last_f_click) > f_cooldown)
			{
				LOG("f!");

				line_mode = !line_mode;

				glPolygonMode(GL_FRONT_AND_BACK, line_mode ? GL_LINE : GL_FILL);

				last_f_click = glfwGetTime();
			}
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		constexpr void *offset = (void *)0;

#pragma region render-cube
		constexpr float rotation_rate = 0.01f;
		cube_model = glm::rotate(cube_model, rotation_rate, glm::vec3{1, 1, 0});

		glUniformMatrix4fv(model_location, 1, GL_FALSE,
		                   glm::value_ptr(cube_model));

		constexpr int indices = 6 * 6;
		glBindVertexArray(cube.vao);
		glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, offset);

		glUniformMatrix4fv(model_location, 1, GL_FALSE,
		                   glm::value_ptr(glm::translate(
							   cube_model, glm::vec3{-0.5f, -0.15f, 0.0f})));

		glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, offset);
#pragma endregion

#pragma region render-sphere
		sphere_model = glm::translate(
			sphere_model, glm::vec3{0, sin(glfwGetTime() * 2.5f), 0} * 0.025f);
		glUniformMatrix4fv(model_location, 1, GL_FALSE,
		                   glm::value_ptr(sphere_model));

		glBindVertexArray(sphere.vao);
		glDrawElements(GL_TRIANGLES, sphere.nr_indices, GL_UNSIGNED_INT,
		               offset);

		glUniformMatrix4fv(model_location, 1, GL_FALSE,
		                   glm::value_ptr(glm::translate(
							   sphere_model, glm::vec3{3.33f, -3.25f, 0.0f})));

		glDrawElements(GL_TRIANGLES, sphere.nr_indices, GL_UNSIGNED_INT,
		               offset);
#pragma endregion

#pragma region render-pyramid
		glBindVertexArray(pyramid.vao);
		const float scale = std::lerp(0.15f, 2.0f, sin(glfwGetTime()));
		glm::mat4 pyramid_model =
			glm::scale(original_pyramid_model, glm::vec3{scale});

		glUniformMatrix4fv(model_location, 1, GL_FALSE,
		                   glm::value_ptr(pyramid_model));

		glDrawElements(GL_TRIANGLES, pyramid.indices, GL_UNSIGNED_INT, offset);

		glUniformMatrix4fv(model_location, 1, GL_FALSE,
		                   glm::value_ptr(glm::scale(original_pyramid_model,
		                                             glm::vec3{-scale})));

		glDrawElements(GL_TRIANGLES, pyramid.indices, GL_UNSIGNED_INT, offset);
#pragma endregion

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
