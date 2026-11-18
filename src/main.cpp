#pragma once
#include "glm/geometric.hpp"
#include "renderer/renderer.h"
#include <cstdint>
#include <string>
#define STB_IMAGE_IMPLEMENTATION

#include "dirty.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "glslSemantics.h"
#include "macros.h"
#include "platform/platform.h"
#include "util/util.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <stb_image.h>

int main(int argc, char **argv)
{
	bool is_two_players = false;

	platform::window_data window{};
	if (!platform::init(800, 600, window))
	{
		std::cerr << "platform::init failed" << std::endl;
		return -1;
	}

	renderer::shader_program shader{};

	std::string glsl_semantics{};
	util::read_file(GLSL_SEMANTICS_STR, glsl_semantics);

	const char *vertex_shader_path{"shader/basic.vert"};
	const char *fragment_shader_path{"shader/basic.frag"};
	if (!renderer::create_program(glsl_semantics, vertex_shader_path,
	                              fragment_shader_path, shader))
	{
		LOG("Failed creating program, returning");
		return -1;
	}

#pragma region view
	constexpr float camera_height = 5.0f;
	glm::vec3 view_pos{0, camera_height, 30.0f};
	glm::vec3 view_target{0, camera_height, 0}; // Look forward
	glm::vec3 view_up_axis{0, 1, 0};
	glm::mat4 camera1 = glm::lookAt(view_pos, view_target, view_up_axis);
	glm::mat4 camera2 = glm::lookAt(view_pos, view_target, view_up_axis);
#pragma endregion

	constexpr float near{0.1f}, far{100.0f};
	constexpr float fov{glm::radians(45.0f)};
	auto get_aspect_ratio = [&]()
	{
		return (static_cast<float>(window.width) / (is_two_players ? 2 : 1)) /
		       static_cast<float>(window.height);
	};
	auto get_perspective_matrix = [&]()
	{ return glm::perspective(fov, get_aspect_ratio(), near, far); };

#pragma region uniform-matrices-buffer
	glm::mat4 perspective = get_perspective_matrix();

	renderer::matrix_ubo mubo = renderer::create_matrix_ubo();
	renderer::upload_projection(mubo, glm::value_ptr(perspective));
	renderer::upload_view(mubo, glm::value_ptr(camera1));

#pragma endregion

	dirty::models models = dirty::upload_models();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	bool line_mode = false;

#pragma region input
	// User would defines. Meta meaning to input slots.
	// enum class gameplay_action : MINE_input::input_slots
	// {
	// 	NONE,
	// 	FORWARD,
	// 	LEFT,
	// 	RIGHT,
	// 	DOWN,
	// };

	// std::vector<MINE_input::token_binding> bindings{
	// 	{MINE_input::KEY::W,
	// (MINE_input::input_slots)gameplay_action::FORWARD},
	// 	{MINE_input::KEY::UP,
	// 	 (MINE_input::input_slots)gameplay_action::FORWARD}};

	// MINE_input::bind(bindings);

	// LOG("Forward: " << forward);
	// LOG("Left: " << left);

	float movement_speed{30};

	constexpr float space_cooldown = 0.2f;
	double last_space_click{-space_cooldown};
	constexpr float f_cooldown = 0.2f;
	double last_f_click{-f_cooldown};
#pragma endregion

	int color_loc = glGetUniformLocation(shader.program, STR_DEF(COLOR));
	glm::vec3 color = {0.5, 0.5, 0.5};
	glProgramUniform3fv(shader.program, color_loc, 1, &color[0]);

#pragma region texture
	const char *texture_file = "asset/plank.jpg";
	uint32_t tex;
	if (!renderer::create_texture(texture_file, tex))
	{
		return -1;
	}
	glBindTextureUnit(TEX2D, tex);
#pragma endregion

	// First player
	auto scissor_first_player = [&]()
	{ glScissor(0, 0, window.width / 2, window.height); };
	// Second player
	auto scissor_second_player = [&]()
	{ glScissor(window.width / 2, 0, window.width / 2, window.height); };

	double delta_time{0};
	// Updating last time at the end of the loop. So it reflects what is
	// says throughout the loop.
	double last_time{glfwGetTime()};

	glUseProgram(shader.program);
	glBindVertexArray(models.vao);
	while (!glfwWindowShouldClose(window.platform_window))
	{
		double time = glfwGetTime();
		delta_time = time - last_time;

		if (glfwGetKey(window.platform_window, GLFW_KEY_ESCAPE))
		{
			// this would close after this frame, is that what we want?
			glfwSetWindowShouldClose(window.platform_window, true);
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_SPACE))
		{
			if ((glfwGetTime() - last_space_click) > space_cooldown)
			{
				LOG("Space! - Changing player count");
				last_space_click = time;
				is_two_players = !is_two_players;
				// ubo_data.projection = get_perspective_matrix();
				// glNamedBufferSubData(uniform_matrices_object, 0,
				//                      sizeof(glm::mat4),
				//                      glm::value_ptr(ubo_data.projection));
			}
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_F))
		{
			if ((glfwGetTime() - last_f_click) > f_cooldown)
			{
				LOG("f!");
				line_mode = !line_mode;
				glPolygonMode(GL_FRONT_AND_BACK, line_mode ? GL_LINE : GL_FILL);
				last_f_click = glfwGetTime();
			}
		}

		glm::vec3 movement_input{0};

		if (glfwGetKey(window.platform_window, GLFW_KEY_W))
		{
			movement_input.z += 1;
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_S))
		{
			movement_input.z -= 1;
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_A))
		{
			movement_input.x += 1;
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_D))
		{
			movement_input.x -= 1;
		}

		if (movement_input != glm::vec3(0))
		{
			movement_input = glm::normalize(movement_input);
			glm::vec3 movement =
				movement_input *
				static_cast<float>(movement_speed * delta_time);
			camera1 = glm::translate(camera1, movement);
		}

		// First player
		if (is_two_players)
		{
			scissor_first_player();
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}
		// ubo_data.view = camera1;
		// glNamedBufferSubData(uniform_matrices_object, sizeof(glm::mat4),
		//                      sizeof(glm::mat4),
		//                      glm::value_ptr(ubo_data.view));

		glClearColor(0.8, 0.12, 0.12, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, window.width / (is_two_players ? 2 : 1),
		           window.height);
		constexpr GLenum index_typ =
			GL_UNSIGNED_INT; // @TODO: Replace with GL_UNSIGNED_SHORT
		constexpr void *command_buffer_offset =
			(void *)0; // @TODO: Change this based on which mesh to draw
		GLsizei commands_to_execute = models.nr_commands;
		constexpr GLsizei command_stride = sizeof(dirty::indirect_command);
		glMultiDrawElementsIndirect(GL_TRIANGLES, index_typ,
		                            command_buffer_offset, commands_to_execute,
		                            command_stride);
		if (is_two_players)
		{
			glDisable(GL_SCISSOR_TEST);
		}

		// Second player
		if (is_two_players)
		{
			// ubo_data.view = camera2;
			// glNamedBufferSubData(uniform_matrices_object, sizeof(glm::mat4),
			//                      sizeof(glm::mat4),
			//                      glm::value_ptr(ubo_data.view));

			scissor_second_player();
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.12, 0.8, 0.12, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glViewport(window.width / 2, 0, window.width / 2, window.height);
			glMultiDrawElementsIndirect(GL_TRIANGLES, index_typ,
			                            command_buffer_offset,
			                            commands_to_execute, command_stride);

			glDisable(GL_SCISSOR_TEST);
		}

		// Platform stuff
		glfwSwapBuffers(window.platform_window);
		glfwPollEvents();
		last_time = time;
	}

	glfwTerminate();
	return 0;
}
