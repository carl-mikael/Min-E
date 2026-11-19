#pragma once
#include <cstdint>
#include <string_view>

namespace renderer
{
struct shader_program
{
	uint32_t program;
};

bool create_program(const std::string_view &glsl_semantics,
                    const char *vertex_shader_path,
                    const char *fragment_shader_path,
                    shader_program &out_program) noexcept;

bool create_shader(unsigned int *out_shader, const char *shader_path,
                   const unsigned long type,
                   const std::string_view &glsl_semantics) noexcept;

struct matrix_ubo
{
	uint32_t handle;
};

void upload_projection(matrix_ubo mubo, const float *m44) noexcept;
void upload_view(matrix_ubo mubo, const float *m44) noexcept;
[[nodiscard]]
matrix_ubo create_matrix_ubo() noexcept;

struct texture_data
{
	uint32_t handle;
};

bool create_texture(const char *texture_path,
                    texture_data &out_texture_info) noexcept;

void bind_texture(const texture_data &texture_info) noexcept;

struct vertices_view
{
	const float *data;
	const size_t bytes;
};

struct indices_view
{
	const uint32_t *data;
	const size_t bytes;
};

struct handle
{
	uint32_t id;
};

[[nodiscard]]
handle upload_objects(const vertices_view &vertices,
                      const indices_view &indices);

struct draw_command
{
	uint32_t index_count;
	uint32_t instance_count;
	size_t first_index;
	size_t base_vertex;
	uint32_t base_instance;
};

struct command_view
{
	const draw_command *data;
	const size_t bytes;
};

void upload_draw_commands(const handle &handle,
                          const command_view &commands) noexcept;
} // namespace renderer
