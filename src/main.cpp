#pragma once
#include "assetManager.h"
#include "glm/geometric.hpp"
#include "renderer/renderer.h"
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION

#include "dirty.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "macros.h"
#include "platform/platform.h"
#include "util/util.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <stb_image.h>

#ifdef DEBUG
#include <iostream> // For LOG
#endif

int main(int argc, char **argv)
{
	bool is_two_players = false;
	bool line_mode = false;

	platform::window_data window{};
	if (!platform::init(800, 600, window))
	{
		LOG("platform::init failed");
		return -1;
	}

	const char *files[]{
		"asset/model/cube.obj",
		"asset/model/tree.obj",
	};

	am::models_data m_data{};
	am::model_handles m_handles{};
	if (!am::load_models(files, std::size(files), m_data, m_handles))
	{
		return -1;
	}
	for (const auto &mh : m_handles.handles)
	{
		LOG("mh - index count: " << mh.index_count
		                         << " first index: " << mh.first_index
		                         << " base vertex: " << mh.base_vertex);
	}

	dirty::scene_data scene{};
	if (!dirty::load_scene("asset/scene/test.sid", scene))
	{
		return -1;
	}
	const float *vertex_data = am::get_data(m_data.vertices.front());
	am::model_data_sizes mds = am::get_bytes(m_data);

	renderer::vertices_view v_vertices{vertex_data, mds.vertices_size};
	renderer::indices_view v_indices{m_data.indices.data(), mds.indices_size};

	renderer::handle handle = renderer::upload_objects(v_vertices, v_indices);

	std::vector<renderer::draw_command> draw_commands{};
	uint32_t tot_instances{0};
	for (int i{0}; i < scene.object_data.ids.size(); ++i)
	{
		renderer::draw_command command{};
		uint32_t model_idx = scene.object_data.ids[i];
		const auto &m_handle = m_handles.handles[model_idx];
		command.index_count = m_handle.index_count;
		command.first_index = m_handle.first_index;
		command.base_vertex = m_handle.base_vertex;
		command.base_instance = tot_instances;
		command.instance_count =
			scene.object_data.instances[model_idx]; //.positions.size() / 3;
		tot_instances += command.instance_count;
		draw_commands.push_back(command);
	}

	renderer::command_view cv{.data = draw_commands.data(),
	                          .bytes = sizeof(draw_commands[0]) *
	                                   draw_commands.size()};
	renderer::upload_draw_commands(handle, cv);

#pragma region create-shader
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
#pragma endregion

#pragma region view
	constexpr float camera_height = 5.0f;
	constexpr glm::vec3 view_pos{0, camera_height, 30.0f};
	constexpr glm::vec3 view_target{0, camera_height, 0}; // Look forward
	constexpr glm::vec3 view_up_axis{0, 1, 0};
	glm::mat4 camera1 = glm::lookAt(view_pos, view_target, view_up_axis);
	glm::mat4 camera2 = glm::lookAt(view_pos, view_target, view_up_axis);

	constexpr float near{0.1f}, far{100.0f};
	constexpr float fov{glm::radians(45.0f)};
	auto get_perspective_matrix = [&]()
	{
		return glm::perspective(fov, get_aspect(window, is_two_players), near,
		                        far);
	};
#pragma endregion

#pragma region uniform-matrices-buffer
	glm::mat4 perspective = get_perspective_matrix();

	renderer::matrix_ubo mubo = renderer::create_matrix_ubo();
	renderer::upload_projection(mubo, glm::value_ptr(perspective));
	renderer::upload_view(mubo, glm::value_ptr(camera1));
#pragma endregion

	// dirty::models models = dirty::upload_models();

#pragma region input-info
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

#pragma region texture
	const char *texture_file = "asset/plank.jpg";
	renderer::texture_data tex{};
	if (!renderer::create_texture(texture_file, tex))
	{
		return -1;
	}
	renderer::bind_texture(tex);
#pragma endregion
	platform::tick_timer timer{};

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glUseProgram(shader.program);
	// glBindVertexArray(models.vao);
	glBindVertexArray(handle.id);
	while (platform::update(window, timer))
	{
#pragma region get-input
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
				last_space_click = timer.tick_time;
				is_two_players = !is_two_players;
				glNamedBufferSubData(mubo.handle, 0, sizeof(glm::mat4),
				                     glm::value_ptr(get_perspective_matrix()));
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
#pragma endregion

#pragma region handle-input
		if (movement_input != glm::vec3(0))
		{
			movement_input = glm::normalize(movement_input);
			glm::vec3 movement =
				movement_input *
				static_cast<float>(movement_speed *
			                       platform::delta_time(timer));
			camera1 = glm::translate(camera1, movement);
		}
#pragma endregion

#pragma region first-player
		if (is_two_players)
		{
			glScissor(0, 0, window.width / 2, window.height);
			glEnable(GL_SCISSOR_TEST);
		}
		else
		{
			glDisable(GL_SCISSOR_TEST);
		}
		glNamedBufferSubData(mubo.handle, sizeof(glm::mat4), sizeof(glm::mat4),
		                     glm::value_ptr(camera1));

		glClearColor(0.8, 0.12, 0.12, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, window.width / (is_two_players ? 2 : 1),
		           window.height);

		// constexpr GLenum index_typ =
		// 	GL_UNSIGNED_INT; // @TODO: Replace with GL_UNSIGNED_SHORT
		// constexpr void *command_buffer_offset =
		// 	(void *)0; // @TODO: Change this based on which mesh to draw
		// GLsizei commands_to_execute = models.nr_commands;
		// constexpr GLsizei command_stride = sizeof(dirty::indirect_command);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0,
		                            draw_commands.size(),
		                            sizeof(renderer::draw_command));

		if (is_two_players)
		{
			glDisable(GL_SCISSOR_TEST);
		}
#pragma endregion

#pragma region second player
		if (is_two_players)
		{
			glNamedBufferSubData(mubo.handle, sizeof(glm::mat4),
			                     sizeof(glm::mat4), glm::value_ptr(camera2));

			glScissor(window.width / 2, 0, window.width / 2, window.height);
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.12, 0.8, 0.12, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// glViewport(window.width / 2, 0, window.width / 2, window.height);
			// glMultiDrawElementsIndirect(GL_TRIANGLES, index_typ,
			//                             command_buffer_offset,
			//                             commands_to_execute, command_stride);

			glDisable(GL_SCISSOR_TEST);
		}
#pragma endregion
	}

	glfwTerminate();
	return 0;
}
