#pragma once
#include "glm/ext/vector_float2.hpp"
#include "glm/geometric.hpp"
#include <cstring>
#include <string>
#include <string_view>
#include <unordered_map>
#define STB_IMAGE_IMPLEMENTATION

#include "assimp/defs.h"
#include "assimp/vector3.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "glslSemantics.h"
#include "input/input.h"
#include "macros.h"
#include "statics.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include <stb_image.h>
#include <vector>

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

	bool is_two_players = false;

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
	constexpr float cube_size = 0.5f;
	auto cube = statics::create_cube(cube_size);
	auto sphere = statics::create_sphere();
	auto pyramid = statics::create_pyramid();

	GLuint shader_program, vertex_shader, fragment_shader, geometry_shader;

#pragma region shader-setup
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
#pragma endregion

#pragma region view
	constexpr float camera_height = 5.0f;
	glm::vec3 view_pos{0, camera_height, 30.0f};
	glm::vec3 view_target{0, camera_height, 0}; // Look forward
	glm::vec3 view_up_axis{0, 1, 0};
	glm::mat4 camera1 = glm::lookAt(view_pos, view_target, view_up_axis);
	glm::mat4 camera2 = glm::lookAt(view_pos, view_target, view_up_axis);
#pragma endregion

#pragma region uniform-matrices-buffer
	float window_width{WINDOW_WIDTH}, window_height{WINDOW_HEIGHT};
	auto get_aspect_ratio = [&]()
	{
		return (static_cast<float>(window_width) / (is_two_players ? 2 : 1)) /
		       static_cast<float>(window_height);
	};

	// @TODO: Have this in platform layer where it has access to window
	// width/height
	glfwSetFramebufferSizeCallback(
		window,
		[](GLFWwindow *window, int width, int height)
		{
			LOG("TODO: Put FramebufferSizeCallback in platform layer where it "
		        "has access to window width/height");
		});

	constexpr float near{0.1f}, far{100.0f};
	constexpr float fov{glm::radians(45.0f)};
	auto get_perspective_matrix = [&]()
	{ return glm::perspective(fov, get_aspect_ratio(), near, far); };
	glm::mat4 perspective = get_perspective_matrix();

	float ortho_near_height = 5.0f;
	float ortho_near_width = ortho_near_height * get_aspect_ratio();
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
	ubo_data.view = camera1;

	glNamedBufferStorage(uniform_matrices_object, 2 * sizeof(glm::mat4),
	                     &ubo_data, GL_DYNAMIC_STORAGE_BIT);

	// This is the binding index specified in the uniform block which must
	// match each other
	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MATRICES,
	                 uniform_matrices_object);
#pragma endregion

#pragma region create-model-data
	// These should not store the values after they have been uploaded
	std::vector<statics::vertex> vertices{};
	std::vector<unsigned int> indices{};

	static_assert(std::is_same_v<ai_real, float>,
	              "Assimp isn't built using float");
	static_assert(sizeof(aiVector3t<ai_real>) == 3 * sizeof(float),
	              "Assimp vector doesn't match 3 floats");

	const char *cube_file = "asset/cube.obj";
	const char *tree_file = "asset/tree.obj";
	const char *golem_file = "asset/StoneGolem.obj";
	const char *eyeball_file = "asset/eyeball.obj";
	const char *penguin_file = "asset/penguin.obj";

	const char *badcube =
		"asset/badcube.obj"; // @WARNING: It's broken to test system

	std::unordered_map<std::string, statics::indirect_instanced> objects{};

	struct
	{
		std::string file;
		glm::vec3 pos;
	} obj_render_data[]{
		cube_file,    glm::vec3{0}, eyeball_file, glm::vec3{-3, 0, 5},
		tree_file,    {0, 0, -30},  golem_file,   {-10, 0, 0},
		penguin_file, {4, 0, 15},
	};

	for (int i{0}; i < std::size(obj_render_data); ++i)
	{
		if (!statics::render_obj(obj_render_data[i].file,
		                         obj_render_data[i].pos, objects, vertices,
		                         indices))
		{
			std::cerr << "Failed to add command from file: "
					  << obj_render_data[i].file << std::endl;
			return -1;
		}
	}

	// if (!statics::render_obj(badcube, glm::vec3{0}, objects, vertices,
	// indices))
	// {
	// 	return -1;
	// }
	// statics::render_obj(cube_file, glm::vec3{5.0f, 0.0f, 0.0f}, objects,
	//                     vertices, indices);
	// statics::render_obj(tree_file, {0, 0, -30}, objects, vertices,
	// indices); statics::render_obj(golem_file, {-10, 0, 0}, objects,
	// vertices, indices); statics::render_obj(golem_file, {8, 0, -5},
	// objects, vertices, indices);
#pragma endregion
	GLuint vao = statics::upload_objects({vertices}, {indices});

#pragma region upload-commands-and-offsets
	std::vector<statics::indirect_command> commands{};
	std::vector<glm::vec3> offsets{};
	unsigned instances{0};
	for (auto pair : objects)
	{
		// Command
		statics::indirect_command &command = pair.second.command;
		command.base_instance = instances;
		std::vector<glm::vec3> &model_offsets = pair.second.offsets;
		unsigned model_instances = model_offsets.size();
		command.instance_count = model_instances;
		instances += model_instances;
		commands.push_back(command);

		// Offsets
		const size_t offset_base = offsets.size();
		offsets.resize(offset_base + model_instances);
		std::memcpy(offsets.data() + offset_base, model_offsets.data(),
		            model_offsets.size() * sizeof(offsets[0]));
	}

	GLuint offsets_id, offsets_slot{1};
	glCreateBuffers(1, &offsets_id);

	glNamedBufferStorage(offsets_id, offsets.size() * sizeof(offsets[0]),
	                     offsets.data(), 0);
	glVertexArrayVertexBuffer(vao, offsets_slot, offsets_id, 0,
	                          3 * sizeof(float));

	constexpr GLuint divisor = 1; // Update offset every new instance
	glVertexArrayBindingDivisor(vao, offsets_slot, divisor);

	glVertexArrayAttribFormat(vao, ATTR_INST_OFFSET, 3, GL_FLOAT, false, 0);
	glEnableVertexArrayAttrib(vao, ATTR_INST_OFFSET);
	glVertexArrayAttribBinding(vao, ATTR_INST_OFFSET, offsets_slot);

	// upload draw commands
	GLuint indirect_commands;
	glCreateBuffers(1, &indirect_commands);
	glNamedBufferStorage(indirect_commands,
	                     std::size(commands) * sizeof(commands[0]),
	                     commands.data(), 0);

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_commands);
#pragma endregion

#pragma region model-matrices
	std::vector<glm::mat4> models(instances, {1});

	constexpr int far_left_pos = -15.0f;
	constexpr int model_offset = 10.0f;
	for (int i{0}; i < models.size(); ++i)
	{
		// models[i] = glm::translate(
		// {1}, glm::vec3{far_left_pos + model_offset * i, 0, -20.0f});
	}

	// models.back() = glm::translate({1}, glm::vec3{0, 0, -10.0f});

	GLuint models_handle;
	glCreateBuffers(1, &models_handle);
	glNamedBufferStorage(models_handle, UBO_MAX_SIZE, nullptr,
	                     GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MODELS, models_handle);

	glNamedBufferSubData(models_handle, 0, models.size() * sizeof(models[0]),
	                     models.data());
#pragma endregion

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

	int color_loc = glGetUniformLocation(shader_program, STR_DEF(COLOR));
	glm::vec3 color = {0.5, 0.5, 0.5};
	glProgramUniform3fv(shader_program, color_loc, 1, &color[0]);

#pragma region texture
	const char *texture_file = "asset/plank.jpg";
	int width, height, nr_channels;
	unsigned char *tex_data =
		stbi_load(texture_file, &width, &height, &nr_channels, 0);

	if (tex_data == nullptr)
	{
		std::cerr << "Couldn't load texture at: " << texture_file << std::endl;
		return -1;
	}

	LOG("Loaded texture: " << texture_file << " with width: " << width
	                       << " height: " << height
	                       << " channels: " << nr_channels);

	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D, 1, &tex);

	glTextureParameteri(tex, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(tex, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLenum format;
	if (nr_channels == 1)
	{
		format = GL_RED;
	}
	else if (nr_channels == 3)
	{
		format = GL_RGB8;
	}
	else if (nr_channels == 4)
	{
		format = GL_RGBA;
	}
	else
	{
		format = GL_RED;
		std::cerr << "Unknown format with ' " << nr_channels << "' using GL_RED"
				  << std::endl;
	}

	glTextureStorage2D(tex, 1, format, width, height);
	glTextureSubImage2D(tex, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE,
	                    tex_data);
	glBindTextureUnit(TEX2D, tex);
#pragma endregion

	// First player
	auto scissor_first_player = [&]()
	{ glScissor(0, 0, window_width / 2, window_height); };
	// Second player
	auto scissor_second_player = [&]()
	{ glScissor(window_width / 2, 0, window_width / 2, window_height); };

	double delta_time{0};
	// Updating last time at the end of the loop. So it reflects what is
	// says throughout the loop.
	double last_time{glfwGetTime()};

	glUseProgram(shader_program);
	glBindVertexArray(vao);
	while (!glfwWindowShouldClose(window))
	{
		double time = glfwGetTime();
		delta_time = time - last_time;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
		{
			// this would close after this frame, is that what we want?
			glfwSetWindowShouldClose(window, true);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE))
		{
			if ((glfwGetTime() - last_space_click) > space_cooldown)
			{
				LOG("Space! - Changing player count");
				last_space_click = time;
				is_two_players = !is_two_players;
				ubo_data.projection = get_perspective_matrix();
				glNamedBufferSubData(uniform_matrices_object, 0,
				                     sizeof(glm::mat4),
				                     glm::value_ptr(ubo_data.projection));
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

		glm::vec3 movement_input{0};

		if (glfwGetKey(window, GLFW_KEY_W))
		{
			movement_input.z += 1;
		}

		if (glfwGetKey(window, GLFW_KEY_S))
		{
			movement_input.z -= 1;
		}

		if (glfwGetKey(window, GLFW_KEY_A))
		{
			movement_input.x += 1;
		}

		if (glfwGetKey(window, GLFW_KEY_D))
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
		ubo_data.view = camera1;
		glNamedBufferSubData(uniform_matrices_object, sizeof(glm::mat4),
		                     sizeof(glm::mat4), glm::value_ptr(ubo_data.view));

		glClearColor(0.8, 0.12, 0.12, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, window_width / (is_two_players ? 2 : 1),
		           WINDOW_HEIGHT);
		constexpr GLenum index_typ =
			GL_UNSIGNED_INT; // @TODO: Replace with GL_UNSIGNED_SHORT
		constexpr void *command_buffer_offset =
			(void *)0; // @TODO: Change this based on which mesh to draw
		GLsizei commands_to_execute = commands.size();
		constexpr GLsizei command_stride = sizeof(statics::indirect_command);
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
			ubo_data.view = camera2;
			glNamedBufferSubData(uniform_matrices_object, sizeof(glm::mat4),
			                     sizeof(glm::mat4),
			                     glm::value_ptr(ubo_data.view));

			scissor_second_player();
			glEnable(GL_SCISSOR_TEST);
			glClearColor(0.12, 0.8, 0.12, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glViewport(window_width / 2, 0, window_width / 2, window_height);
			glMultiDrawElementsIndirect(GL_TRIANGLES, index_typ,
			                            command_buffer_offset,
			                            commands_to_execute, command_stride);

			glDisable(GL_SCISSOR_TEST);
		}

		// Platform stuff
		glfwSwapBuffers(window);
		glfwPollEvents();
		last_time = time;
	}

	glfwTerminate();
	return 0;
}
