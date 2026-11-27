#pragma once
#include "assetManager.h"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "input/input.h"
#include "renderer/renderer.h"
#include "util/arena.h"
#include "util/ecs.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>
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
	glm::vec3 movement_input{0};
	int yaw_input{0};
	int yaw_input2{0};
	glm::vec<2, double> last_cursor_pos{0};
	glm::vec<2, double> current_cursor_pos{0};
	glm::vec<2, double> mouse_delta{0};
	glm::vec3 movement_input2{0};
	platform::tick_timer timer{};

	platform::window_data window{};
	if (!platform::init(800, 600, window))
	{
		LOG("platform::init failed");
		return -1;
	}

#pragma region game_scene
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

	renderer::slots instance_offsets_slot{};
	renderer::add_instance_offset_attr(handle, slots, instance_offsets_slot);

	struct game_scene
	{
		struct draw_info
		{
			renderer::commands_handle commands_handle;
			renderer::matrices_handle matrices_handle;
			renderer::instance_offsets_handle offsets_handle;
			renderer::slots instance_offsets_slot;
			int nr_commands;
		} draw_info;
		glm::vec3 start;
		glm::vec3 goal;
	} game_scene1, game_scene2;

	game_scene1.draw_info.instance_offsets_slot = instance_offsets_slot;
	game_scene2.draw_info.instance_offsets_slot = instance_offsets_slot;

	am::scene_data scene{};
	if (!am::load_scene("asset/scene/scene1.sid", scene))
	{
		return -1;
	}

	std::memcpy(&game_scene1.start, scene.start_pos, sizeof(scene.start_pos));
	std::memcpy(&game_scene1.goal, scene.goal_pos, sizeof(scene.goal_pos));

	constexpr size_t arena_size{mem::MIN_ARENA_CAPACITY};
	ecs::storage mesh_storage{};
	if (!ecs::create_storage(arena_size, arena_size, mesh_storage))
	{
		LOG("Failed creating mesh_storage");
		return -1;
	}
	ecs::storage transform_storage{};
	if (!ecs::create_storage(arena_size, arena_size, transform_storage))
	{
		LOG("Failed creating transform_storage");
		return -1;
	}

	uint32_t tot_instances{0};
	const int nr_commands1{static_cast<int>(scene.objects_data.ids.size())};

	std::vector<ecs::entity> entities(nr_commands1);

	for (int i{0}; i < nr_commands1; ++i)
	{
		// Create entity id
		ecs::entity &entity = entities[i];
		entity.id = i;

		// Create command/"mesh" inplace using mesh_storage
		auto command =
			ecs::add_raw(mesh_storage, entity, renderer::draw_command{});

		// Build the draw command
		uint32_t model_idx = scene.objects_data.ids[i];
		assert(model_idx < m_handles.handles.size());
		const auto &m_handle = m_handles.handles[model_idx];
		command->index_count = m_handle.index_count;
		command->first_index = static_cast<uint32_t>(m_handle.first_index);
		command->base_vertex = static_cast<uint32_t>(m_handle.base_vertex);
		command->base_instance = tot_instances;
		// Using i instead of model_idx allows for creating seperate
		// instances of the same mesh data
		command->instance_count = scene.objects_data.instance_counts[i];
		tot_instances += command->instance_count;

		// Create transform
		auto _ = ecs::add_raw(transform_storage, entity, glm::mat4{1});
	}
	game_scene1.draw_info.nr_commands = nr_commands1;

	renderer::float_view models_matrices_view{
		.data = reinterpret_cast<float *>(transform_storage.components.buffer),
		.bytes = transform_storage.components.base,
	};

	renderer::upload_matrices(models_matrices_view,
	                          game_scene1.draw_info.matrices_handle);

	renderer::command_view cv{.data =
	                              reinterpret_cast<renderer::draw_command *>(
									  mesh_storage.components.buffer),
	                          .bytes = mesh_storage.components.base};

	game_scene1.draw_info.commands_handle = renderer::upload_draw_commands(cv);

	if (!renderer::upload_instance_offsets(
			{.data = scene.objects_data.positions.data(),
	         .bytes = scene.objects_data.positions.size() *
	                  sizeof(scene.objects_data.positions[0])},
			game_scene1.draw_info.offsets_handle))
	{
		return -1;
	}

	game_scene *current;
	current = &game_scene1;
	// Also changed on keybind event
	renderer::use_matrices(current->draw_info.matrices_handle);
	renderer::use_instance_offsets(handle, instance_offsets_slot,
	                               current->draw_info.offsets_handle);
	renderer::use_draw_commands(current->draw_info.commands_handle);
#pragma endregion

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
	static constexpr glm::vec3 view_up_axis{0, 1, 0};

	glm::mat4 camera1;
	glm::mat4 camera2;
	auto set_cameras = [&]()
	{
		camera1 = glm::inverse(
			glm::lookAt(current->start, current->goal, view_up_axis));
		camera2 = glm::inverse(
			glm::lookAt(current->start, current->goal, view_up_axis));
	};
	set_cameras();

	constexpr float near{0.1f}, far{100.0f};
	constexpr float fov{glm::radians(45.0f)};
	auto get_perspective_matrix = [&]()
	{
		return glm::perspective(
			fov, platform::get_aspect(window, is_two_players), near, far);
	};
#pragma endregion

#pragma region uniform-matrices-buffer
	glm::mat4 perspective = get_perspective_matrix();

	renderer::camera_ubo mubo = renderer::create_matrix_ubo();
	renderer::upload_projection(mubo, glm::value_ptr(perspective));
#pragma endregion

#pragma region input_ctx
	constexpr float movement_speed{20};
	constexpr float yaw_speed{1};

	input::input_owner player_1{};
	input::input_owner player_2{};
	input::get_owner(player_1);
	input::get_owner(player_2);

	struct tdata
	{
		game_scene **current_pp;
		game_scene *first;
		game_scene *second;
		bool *bfirst_scene;
		renderer::handle rend_obj_handle;
		glm::mat4 *cam1;
		glm::mat4 *cam2;
	} td;

	td.current_pp = (&current);
	td.first = &game_scene1;
	td.second = &game_scene2;
	td.bfirst_scene = &is_first_scene;
	td.rend_obj_handle = handle;
	td.cam1 = &camera1;
	td.cam2 = &camera2;
	struct switch_player_count_context
	{
		bool *is_two_players;
		renderer::camera_ubo *mubo;
		platform::window_data *window;
	} spcc;

	spcc.is_two_players = &is_two_players;
	spcc.mubo = &mubo;
	spcc.window = &window;

	struct switch_mesh_ctx
	{
		ecs::storage *mesh_storage;
		renderer::commands_handle *commands_handle;
	} smc;
	smc.mesh_storage = &mesh_storage;
	smc.commands_handle = &current->draw_info.commands_handle;

#pragma endregion

#pragma region admin_input
	input::bind_key(player_1, GLFW_KEY_ESCAPE, GLFW_PRESS, &window,
	                [](void *data, int)
	                {
						glfwSetWindowShouldClose(
							((platform::window_data *)data)->platform_window,
							true);
					});

	input::bind_key(
		player_1, GLFW_KEY_SPACE, GLFW_PRESS, &spcc,
		[](void *data, int new_state)
		{
			if (new_state != GLFW_PRESS)
			{
				return;
			}

			LOG("Space! - Changing player count");
			auto spcc = (switch_player_count_context *)data;
			*spcc->is_two_players = !*spcc->is_two_players;

			glm::mat4 perspective = glm::perspective(
				fov, platform::get_aspect(*spcc->window, *spcc->is_two_players),
				near, far);

			glNamedBufferSubData(spcc->mubo->handle, 0, sizeof(glm::mat4),
		                         glm::value_ptr(perspective));
		});

	input::bind_key(player_1, GLFW_KEY_F, GLFW_PRESS, &line_mode,
	                [](void *data, int new_state)
	                {
						if (new_state != GLFW_PRESS)
						{
							return;
						}
						LOG("f!");
						bool *line_mode = (bool *)data;
						*line_mode = !*line_mode;
						glPolygonMode(GL_FRONT_AND_BACK,
		                              *line_mode ? GL_LINE : GL_FILL);
					});

	input::bind_key(
		player_1, GLFW_KEY_C, GLFW_PRESS, &td,
		[](void *data, int new_state)
		{
			if (new_state != GLFW_PRESS)
			{
				return;
			}

			LOG("YUP: ");
			tdata *td = reinterpret_cast<tdata *>(data);

			*td->bfirst_scene = !(*td->bfirst_scene);
			game_scene *current_p = (*td->current_pp);
			current_p = (*td->bfirst_scene) ? td->first : td->second;
			(*td->current_pp) = current_p;

			*td->cam1 = glm::inverse(
				glm::lookAt(current_p->start, current_p->goal, view_up_axis));
			*td->cam2 = glm::inverse(
				glm::lookAt(current_p->start, current_p->goal, view_up_axis));

			renderer::use_matrices(current_p->draw_info.matrices_handle);
			renderer::use_instance_offsets(
				td->rend_obj_handle, current_p->draw_info.instance_offsets_slot,
				current_p->draw_info.offsets_handle);
			renderer::use_draw_commands(current_p->draw_info.commands_handle);
		});

	input::bind_key(
		player_1, GLFW_KEY_M, GLFW_PRESS, (void *)&smc,
		[](void *data, int new_state)
		{
			if (new_state != GLFW_PRESS)
			{
				return;
			}

			switch_mesh_ctx &smc = *(switch_mesh_ctx *)data;

			renderer::draw_command *command1{};
			mem::get_obj_at_chill(smc.mesh_storage->components,
		                          sizeof(renderer::draw_command), 0,
		                          (void **)&command1);

			const size_t last_comp_index = mem::get_last_e_idx(
				smc.mesh_storage->components, sizeof(renderer::draw_command));

			renderer::draw_command *command2{};
			mem::get_obj_at_chill(smc.mesh_storage->components,
		                          sizeof(renderer::draw_command),
		                          last_comp_index, (void **)&command2);

			std::swap(*command1, *command2);

			renderer::command_view cv1{
				.data = reinterpret_cast<renderer::draw_command *>(
					smc.mesh_storage->components.buffer),
				.bytes = sizeof(renderer::draw_command)};

			const size_t last_comp_base = smc.mesh_storage->components.base -
		                                  sizeof(renderer::draw_command);

			renderer::command_view cv2{
				.data = reinterpret_cast<renderer::draw_command *>(
					smc.mesh_storage->components.buffer + last_comp_base),
				.bytes = sizeof(renderer::draw_command)};

			renderer::update_draw_commands(cv1, 0, *smc.commands_handle);
			renderer::update_draw_commands(cv2, last_comp_base,
		                                   *smc.commands_handle);
		});
#pragma endregion

#pragma region player_1_input
	input::bind_key(player_1, GLFW_KEY_W, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_S) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = 0;
							}
							else
							{
								v3->z = -1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_S) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = 1;
							}
							else
							{
								v3->z = 0;
							}
						}
					});

	input::bind_key(player_1, GLFW_KEY_S, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_W) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = 0;
							}
							else
							{
								v3->z = 1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_W) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = -1;
							}
							else
							{
								v3->z = 0;
							}
						}
					});

	input::bind_key(player_1, GLFW_KEY_A, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_D) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = 0;
							}
							else
							{
								v3->x = -1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_D) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = 1;
							}
							else
							{
								v3->x = 0;
							}
						}
					});

	input::bind_key(player_1, GLFW_KEY_D, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_A) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = 0;
							}
							else
							{
								v3->x = 1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_A) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = -1;
							}
							else
							{
								v3->x = 0;
							}
						}
					});

	input::bind_key(player_1, GLFW_KEY_Q, GLFW_PRESS | GLFW_RELEASE, &yaw_input,
	                [](void *data, int new_state)
	                {
						auto &yaw_input = *(int *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_E) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = 0;
							}
							else
							{
								yaw_input = 1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_E) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = -1;
							}
							else
							{
								yaw_input = 0;
							}
						}
					});

	input::bind_key(player_1, GLFW_KEY_E, GLFW_PRESS | GLFW_RELEASE, &yaw_input,
	                [](void *data, int new_state)
	                {
						auto &yaw_input = *(int *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_Q) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = 0;
							}
							else
							{
								yaw_input = -1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_Q) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = 1;
							}
							else
							{
								yaw_input = 0;
							}
						}
					});
#pragma endregion

#pragma region player_2_input
	input::bind_key(player_2, GLFW_KEY_RIGHT, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input2,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_LEFT) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = 0;
							}
							else
							{
								v3->x = 1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_LEFT) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = -1;
							}
							else
							{
								v3->x = 0;
							}
						}
					});

	input::bind_key(player_2, GLFW_KEY_LEFT, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input2,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_RIGHT) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = 0;
							}
							else
							{
								v3->x = -1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_RIGHT) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->x = 1;
							}
							else
							{
								v3->x = 0;
							}
						}
					});

	input::bind_key(player_2, GLFW_KEY_UP, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input2,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_DOWN) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = 0;
							}
							else
							{
								v3->z = -1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_DOWN) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = 1;
							}
							else
							{
								v3->z = 0;
							}
						}
					});

	input::bind_key(player_2, GLFW_KEY_DOWN, GLFW_PRESS | GLFW_RELEASE,
	                &movement_input2,
	                [](void *data, int new_state)
	                {
						auto v3 = (glm::vec3 *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_UP) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = 0;
							}
							else
							{
								v3->z = 1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_UP) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								v3->z = -1;
							}
							else
							{
								v3->z = 0;
							}
						}
					});

	input::bind_key(player_2, GLFW_KEY_RIGHT_SHIFT, GLFW_PRESS | GLFW_RELEASE,
	                &yaw_input2,
	                [](void *data, int new_state)
	                {
						auto &yaw_input = *(int *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_END) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = 0;
							}
							else
							{
								yaw_input = 1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_END) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = -1;
							}
							else
							{
								yaw_input = 0;
							}
						}
					});

	input::bind_key(player_2, GLFW_KEY_END, GLFW_PRESS | GLFW_RELEASE,
	                &yaw_input2,
	                [](void *data, int new_state)
	                {
						auto &yaw_input = *(int *)data;
						if (new_state == GLFW_PRESS)
						{
							if ((input::get_key_state(GLFW_KEY_RIGHT_SHIFT) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = 0;
							}
							else
							{
								yaw_input = -1;
							}
						}

						if (new_state == GLFW_RELEASE)
						{
							if ((input::get_key_state(GLFW_KEY_RIGHT_SHIFT) &
			                     (GLFW_PRESS | GLFW_REPEAT)) != 0)
							{
								yaw_input = 1;
							}
							else
							{
								yaw_input = 0;
							}
						}
					});
#pragma endregion

	glfwSetKeyCallback(window.platform_window,
	                   [](GLFWwindow *, int key, int, int action, int)
	                   { input::execute_action(key, action); });

	glfwSetCursorPos(window.platform_window, (double)window.width / 2,
	                 (double)window.height / 2);
	glfwGetCursorPos(window.platform_window, &current_cursor_pos.x,
	                 &current_cursor_pos.y);

#pragma region texture
	const char *texture_file = "asset/plank.jpg";
	const char *texture_file2 = "asset/bamboo.jpg";

	int width, height, nr_channels;
	const unsigned char *texture_data_arr[]{
		stbi_load(texture_file, &width, &height, &nr_channels, 0),
		stbi_load(texture_file2, &width, &height, &nr_channels, 0),
	};

	renderer::texture3D tex3d{};
	renderer::create_texture3D(width, height, GL_RGB8, tex3d);

	renderer::upload_textures(tex3d, 0, texture_data_arr,
	                          std::size(texture_data_arr));

	renderer::bind_texture3D(tex3d);
#pragma endregion

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glUseProgram(shader.program);
	glBindVertexArray(handle.id);
	while (platform::update(window, timer))
	{
		last_cursor_pos = current_cursor_pos;
		glfwGetCursorPos(window.platform_window, &current_cursor_pos.x,
		                 &current_cursor_pos.y);

		mouse_delta = current_cursor_pos - last_cursor_pos;

#pragma region handle-input
		if (mouse_delta != glm::vec<2, double>(0))
		{
			const glm::vec<2, double> rotation =
				mouse_delta * (double)yaw_speed * platform::delta_time(timer) *
				-1.;

			camera1 *= glm::rotate(glm::mat4{1}, (float)rotation.y,
			                       glm::vec3{1, 0, 0});

			glm::vec3 cam1up = glm::inverse(camera1) * glm::vec4(0, 1, 0, 0);
			camera1 *= glm::rotate(glm::mat4{1}, (float)rotation.x, cam1up);
		}

		if (yaw_input2 != 0)
		{
			const float rotation =
				(float)yaw_input2 * yaw_speed * platform::delta_time(timer);

			camera2 = glm::rotate(camera2, rotation, glm::vec3(0, 1, 0));
		}

		if (movement_input != glm::vec3(0))
		{
			movement_input = glm::normalize(movement_input);
			glm::vec3 movement =
				movement_input *
				static_cast<float>(movement_speed *
			                       platform::delta_time(timer));

			camera1 = glm::translate(camera1, movement);
		}

		if (movement_input2 != glm::vec3(0))
		{
			movement_input2 = glm::normalize(movement_input2);
			glm::vec3 movement2 =
				movement_input2 *
				static_cast<float>(movement_speed *
			                       platform::delta_time(timer));

			camera2 = glm::translate(camera2, movement2);
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

		renderer::upload_view(mubo, glm::value_ptr(glm::inverse(camera1)));

		renderer::update_matrices(
			current->draw_info.matrices_handle, 0,
			{.data = glm::value_ptr(camera2), .bytes = sizeof(glm::mat4)});

		glClearColor(0.8f, 0.12f, 0.12f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, window.width / (is_two_players ? 2 : 1),
		           window.height);

		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0,
		                            current->draw_info.nr_commands,
		                            sizeof(renderer::draw_command));

		if (is_two_players)
		{
			glDisable(GL_SCISSOR_TEST);
		}
#pragma endregion

#pragma region second player
		if (is_two_players)
		{
			renderer::upload_view(mubo, glm::value_ptr(glm::inverse(camera2)));

			renderer::update_matrices(
				current->draw_info.matrices_handle, 0,
				{.data = glm::value_ptr(camera1), .bytes = sizeof(glm::mat4)});

			glScissor(window.width / 2, 0, window.width / 2, window.height);
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.12f, 0.8f, 0.12f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glViewport(window.width / 2, 0, window.width / 2, window.height);
			glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0,
			                            current->draw_info.nr_commands,
			                            sizeof(renderer::draw_command));

			glDisable(GL_SCISSOR_TEST);
		}
#pragma endregion
	}

	glfwTerminate();
	return 0;
}
