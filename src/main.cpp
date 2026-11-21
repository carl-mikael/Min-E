#pragma once
#include "assetManager.h"
#include "glm/ext/vector_float4.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "input/input.h"
#include "renderer/renderer.h"
#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION

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

int main(int, char **)
{
	bool is_two_players = false;
	bool line_mode = false;

	bool is_first_scene{true};

	platform::window_data window{};
	if (!platform::init(800, 600, window))
	{
		LOG("platform::init failed");
		return -1;
	}

	const char *files[]{
		"asset/model/cube.obj",
		"asset/model/tree.obj",
		"asset/model/StoneGolem.obj",
	};

	am::models_data m_data{};
	am::model_handles m_handles{};
	if (!am::load_models(files, std::size(files), m_data, m_handles))
	{
		return -1;
	}

	const float *vertex_data = am::get_data(m_data.vertices.front());
	am::model_data_sizes mds = am::get_bytes(m_data);

	renderer::float_view v_vertices{vertex_data, mds.vertices_size};
	renderer::indices_view v_indices{m_data.indices.data(), mds.indices_size};

	renderer::slots slots{};
	renderer::handle handle{};
	if (!renderer::upload_objects(v_vertices, v_indices, slots, handle))
	{
		return -1;
	}

	struct draw_info
	{
		renderer::commands_handle commands_handle;
		renderer::models_handle models_handle;
		renderer::instance_offsets_handle offsets_handle;
		int nr_commands;
	};

	struct test_scene
	{
		draw_info di;
		glm::vec3 start;
		glm::vec3 goal;
	};
	test_scene test_scene1, test_scene2;

	std::vector<renderer::draw_command> draw_commands{};
	am::scene_data scene{};
	if (!am::load_scene("asset/scene/test.sid", scene))
	{
		return -1;
	}

	std::memcpy(&test_scene1.start, scene.start_pos, sizeof(scene.start_pos));
	std::memcpy(&test_scene1.goal, scene.goal_pos, sizeof(scene.goal_pos));

	uint32_t tot_instances{0};
	for (size_t i{0}; i < scene.object_data.ids.size(); ++i)
	{
		renderer::draw_command command{};
		uint32_t model_idx = scene.object_data.ids[i];
		assert(model_idx < m_handles.handles.size());
		const auto &m_handle = m_handles.handles[model_idx];
		command.index_count = m_handle.index_count;
		command.first_index = static_cast<uint32_t>(m_handle.first_index);
		command.base_vertex = static_cast<uint32_t>(m_handle.base_vertex);
		command.base_instance = tot_instances;
		// Using i instead of model_idx allows for creating seperate
		// instances of the same mesh data
		command.instance_count = scene.object_data.instances[i];
		tot_instances += command.instance_count;
		draw_commands.push_back(command);
	}

	test_scene1.di.nr_commands = static_cast<int>(draw_commands.size());

	std::vector<glm::mat4> models(draw_commands.size(), glm::mat4{1.0f});
	renderer::float_view models_view{
		.data = glm::value_ptr(models.front()),
		.bytes = sizeof(glm::mat4) * models.size(),
	};

	renderer::command_view cv{.data = draw_commands.data(),
	                          .bytes = sizeof(draw_commands[0]) *
	                                   draw_commands.size()};

	test_scene1.di.commands_handle = renderer::upload_draw_commands(cv);

	if (!renderer::upload_instance_offsets(
			handle,
			{.data = scene.object_data.positions.data(),
	         .bytes = scene.object_data.positions.size() *
	                  sizeof(scene.object_data.positions[0])},
			slots, test_scene1.di.offsets_handle))
	{
		return -1;
	}

	test_scene1.di.models_handle = renderer::upload_models(models_view);

	std::vector<renderer::draw_command> draw_commands2{};
	am::scene_data scene2{};
	if (!am::load_scene("asset/scene/scene2.sid", scene2))
	{
		return -1;
	}

	std::memcpy(&test_scene2.start, scene2.start_pos, sizeof(scene2.start_pos));
	std::memcpy(&test_scene2.goal, scene2.goal_pos, sizeof(scene2.goal_pos));

	uint32_t tot_instances2{0};
	for (size_t i{0}; i < scene2.object_data.ids.size(); ++i)
	{
		renderer::draw_command command{};
		uint32_t model_idx = scene2.object_data.ids[i];
		assert(model_idx < m_handles.handles.size());
		const auto &m_handle = m_handles.handles[model_idx];
		command.index_count = m_handle.index_count;
		command.first_index = static_cast<uint32_t>(m_handle.first_index);
		command.base_vertex = static_cast<uint32_t>(m_handle.base_vertex);
		command.base_instance = tot_instances2;
		// Using i instead of model_idx allows for creating seperate
		// instances of the same mesh data
		command.instance_count = scene2.object_data.instances[i];
		tot_instances2 += command.instance_count;
		draw_commands2.push_back(command);
	}

	test_scene2.di.nr_commands = (int)draw_commands2.size();

	renderer::command_view cv2{.data = draw_commands2.data(),
	                           .bytes = sizeof(draw_commands2[0]) *
	                                    draw_commands2.size()};

	test_scene2.di.commands_handle = renderer::upload_draw_commands(cv2);

	renderer::upload_instance_offsets(
		handle,
		{.data = scene2.object_data.positions.data(),
	     .bytes = scene2.object_data.positions.size() *
	              sizeof(scene2.object_data.positions[0])},
		slots, test_scene2.di.offsets_handle);

	std::vector<glm::mat4> models2(draw_commands2.size(), glm::mat4{1.0f});
	renderer::float_view models_view2{
		.data = glm::value_ptr(models2.front()),
		.bytes = sizeof(glm::mat4) * models2.size(),
	};

	test_scene2.di.models_handle = renderer::upload_models(models_view2);

	test_scene *current;
	current = &test_scene1;
	renderer::use_models(current->di.models_handle);
	renderer::use_instance_offsets(handle, current->di.offsets_handle);
	renderer::use_draw_commands(current->di.commands_handle);

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
	constexpr glm::vec3 view_up_axis{0, 1, 0};

	glm::mat4 camera1;
	glm::mat4 camera2;
	auto set_cameras = [&]()
	{
		camera1 = glm::lookAt(current->start, current->goal, view_up_axis);
		camera2 = glm::lookAt(current->start, current->goal, view_up_axis);
	};
	set_cameras();

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

	float movement_speed{20};

	constexpr double key_cooldown{0.2f};
	double last_space_click{-key_cooldown};
	double last_f_click{-key_cooldown};
	double last_c_click{-key_cooldown};

	input::input_owner player_1{};
	input::input_owner player_2{};
	input::get_owner(player_1);
	input::get_owner(player_2);

	struct tdata
	{
		test_scene **current_pp;
		test_scene *first;
		test_scene *second;
		bool *first_scene;
		renderer::handle rend_obj_handle;
	} td;

	td.current_pp = (&current);
	td.first = &test_scene1;
	td.second = &test_scene2;
	td.first_scene = &is_first_scene;
	td.rend_obj_handle = handle;

	input::bind_key(
		player_1, GLFW_KEY_1, &td,
		[](void *data)
		{
			LOG("YUP: ");
			tdata *td = reinterpret_cast<tdata *>(data);

			*td->first_scene = !(*td->first_scene);
			test_scene *current_p = (*td->current_pp);
			current_p = (*td->first_scene) ? td->first : td->second;
			(*td->current_pp) = current_p;

			renderer::use_models(current_p->di.models_handle);
			renderer::use_instance_offsets(td->rend_obj_handle,
		                                   current_p->di.offsets_handle);
			renderer::use_draw_commands(current_p->di.commands_handle);
		});
	// input::bind_key(player_2, GLFW_KEY_2, [](void *) { LOG("TTWWWWWo"); });

	glfwSetKeyCallback(
		window.platform_window,
		[](GLFWwindow *window, int key, int scancode, int action, int mods)
		{
			if (action != GLFW_PRESS)
			{
				return;
			}
			input::execute_action(key);
		});
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
	glBindVertexArray(handle.id);
	while (platform::update(window, timer))
	{
#pragma region get-input
		if (glfwGetKey(window.platform_window, GLFW_KEY_ESCAPE))
		{
			// this would close after this frame, is that what we want?
			glfwSetWindowShouldClose(window.platform_window, true);
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_C))
		{
			if ((glfwGetTime() - last_c_click) > key_cooldown)
			{
				current = is_first_scene ? &test_scene2 : &test_scene1;
				is_first_scene = !is_first_scene;
				renderer::use_models(current->di.models_handle);
				renderer::use_instance_offsets(handle,
				                               current->di.offsets_handle);
				renderer::use_draw_commands(current->di.commands_handle);
				set_cameras();
				last_c_click = glfwGetTime();
			}
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_SPACE))
		{
			if ((glfwGetTime() - last_space_click) > key_cooldown)
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
			if ((glfwGetTime() - last_f_click) > key_cooldown)
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
			movement_input.z -= 1;
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_S))
		{
			movement_input.z += 1;
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_A))
		{
			movement_input.x -= 1;
		}

		if (glfwGetKey(window.platform_window, GLFW_KEY_D))
		{
			movement_input.x += 1;
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

			// glm::vec3 forward = camera1 * glm::vec4{0, 0, -1, 0};
			// glm::mat4 forward_mat = glm::translate(glm::mat4{1}, forward);
			// movement = glm::vec3(forward_mat * glm::vec4(movement, 0));
			// movement =
			movement =
				glm::vec3(glm::inverse(camera1) * glm::vec4(movement, 0));
			camera1 = glm::translate(camera1, -movement);
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
		// glNamedBufferSubData(mubo.handle, sizeof(glm::mat4),
		// sizeof(glm::mat4),
		//                      glm::value_ptr(camera1));
		renderer::upload_view(mubo, glm::value_ptr(camera1));

		glClearColor(0.8f, 0.12f, 0.12f, 1.0f);
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
		                            current->di.nr_commands,
		                            sizeof(renderer::draw_command));

		if (is_two_players)
		{
			glDisable(GL_SCISSOR_TEST);
		}
#pragma endregion

#pragma region second player
		if (is_two_players)
		{
			// glNamedBufferSubData(mubo.handle, sizeof(glm::mat4),
			//                      sizeof(glm::mat4), glm::value_ptr(camera2));
			renderer::upload_view(mubo, glm::value_ptr(camera2));

			glScissor(window.width / 2, 0, window.width / 2, window.height);
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.12f, 0.8f, 0.12f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glViewport(window.width / 2, 0, window.width / 2, window.height);
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0,
			                            current->di.nr_commands,
			                            sizeof(renderer::draw_command));

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
