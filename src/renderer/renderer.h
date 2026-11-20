#pragma once
#include <cstdint>
#include <string_view>

namespace renderer
{
struct slots
{
	uint16_t used;
};

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

struct float_view
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
	uint32_t slot;
};

bool upload_objects(const float_view &vertices, const indices_view &indices,
                    slots &slots, handle &out_handle) noexcept;

struct draw_command
{
	uint32_t index_count;
	uint32_t instance_count;
	uint32_t first_index;
	uint32_t base_vertex;
	uint32_t base_instance;
};

struct command_view
{
	const draw_command *data;
	const size_t bytes;
};

struct commands_handle
{
	uint32_t handle;
};

[[nodiscard]]
commands_handle upload_draw_commands(const command_view &commands) noexcept;

void use_draw_commands(const commands_handle &handle) noexcept;

struct models_handle
{
	uint32_t handle;
};
[[nodiscard]]
models_handle upload_models(const float_view &models) noexcept;

void use_models(const models_handle &models_handle) noexcept;

struct instance_offsets_handle
{
	uint32_t handle;
	uint32_t slot;
};

bool upload_instance_offsets(const handle &handle, const float_view &offsets,
                             slots &slots,
                             instance_offsets_handle &out_handle) noexcept;

void use_instance_offsets(
	const handle &handle,
	const instance_offsets_handle &instance_offsets_handle) noexcept;

} // namespace renderer
