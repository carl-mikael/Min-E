#pragma once

#include "assimp/defs.h"
#include "assimp/vector3.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "glslSemantics.h"
#include "macros.h"
#include "statics.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
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

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// Breaks aspect ratio
	glfwSetFramebufferSizeCallback(window,
	                               [](GLFWwindow *window, int width, int height)
	                               { glViewport(0, 0, width, height); });

	glClearColor(1, 1, 1, 1);

#pragma region view
	constexpr float camera_height = 5.0f;
	glm::vec3 view_pos{0, camera_height, 10.0f};
	glm::vec3 view_target{0, camera_height, 0}; // Look forward
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

#pragma region load-objects
	std::vector<glm::vec3> vertices{};
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
	// auto o_command = statics::load_asset(tree_file, vertices, indices);
	// if (!o_command)
	// {
	// 	return -1;
	// }
	// statics::indirect_command command = o_command.value();

	const char *badcube =
		"asset/badcube.obj"; // @WARNING: It's broken to test system
	statics::load_obj(badcube);

#pragma endregion
	statics::indirect_command commands[]{
		statics::load_asset(cube_file, vertices, indices).value(),
		statics::load_asset(tree_file, vertices, indices).value(),
		statics::load_asset(golem_file, vertices, indices).value(),
		statics::load_asset(eyeball_file, vertices, indices).value(),
		statics::load_asset(penguin_file, vertices, indices).value(),
	};

	statics::dynamic_multi_draw multi_draw_info =
		statics::upload_objects(commands, {vertices}, {indices}, {});

	constexpr int num_models = 5;
	glm::mat4 models[num_models];

	constexpr int far_left_pos = -15.0f;
	constexpr int model_offset = 10.0f;
	for (int i{0}; i < num_models; ++i)
	{
		models[i] = glm::translate(
			{1}, glm::vec3{far_left_pos + model_offset * i, 0, -20.0f});
	}

	models[num_models - 1] = glm::translate({1}, glm::vec3{0, 0, -10.0f});

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
	             multi_draw_info.command_buffer_handle);

	GLuint models_handle;
	glCreateBuffers(1, &models_handle);
	glNamedBufferStorage(models_handle, UBO_MAX_SIZE, nullptr,
	                     GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_UNIFORM_BUFFER, BLOCK_MODELS, models_handle);

	glNamedBufferSubData(models_handle, 0, sizeof(models),
	                     glm::value_ptr(models[0]));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	bool line_mode = false;

	constexpr float space_cooldown = 0.2f;
	double last_space_click{-space_cooldown};
	constexpr float f_cooldown = 0.2f;
	double last_f_click{-f_cooldown};

	glUseProgram(shader_program);

	glBindVertexArray(multi_draw_info.vao);

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

		constexpr GLenum index_typ =
			GL_UNSIGNED_INT; // @TODO: Replace with GL_UNSIGNED_SHORT
		constexpr void *command_buffer_offset =
			(void *)0; // @TODO: Change this based on which mesh to draw
		constexpr GLsizei commands_to_execute =
			sizeof(commands) / sizeof(commands[0]);
		constexpr GLsizei command_stride = sizeof(statics::indirect_command);
		glMultiDrawElementsIndirect(GL_TRIANGLES, index_typ,
		                            command_buffer_offset, commands_to_execute,
		                            command_stride);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
