#pragma once
#include <cstddef>
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

struct camera_ubo
{
	uint32_t handle;
};

struct texture_data
{
	uint32_t handle;
};

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
	slots used;
};

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
	size_t capacity;
};

struct matrices_handle
{
	uint32_t handle;
	size_t capacity;
	size_t size;
};

struct instance_offsets_handle
{
	uint32_t handle;
};

bool create_program(const std::string_view &glsl_semantics,
                    const char *vertex_shader_path,
                    const char *fragment_shader_path,
                    shader_program &out_program) noexcept;

// m44 should be 64 bytes
void upload_projection(camera_ubo mubo, const float *m44) noexcept;

// m44 should be 64 bytes
void upload_view(camera_ubo mubo, const float *m44) noexcept;

[[nodiscard]]
camera_ubo create_matrix_ubo() noexcept;

bool create_texture(const char *texture_path,
                    texture_data &out_texture_info) noexcept;

void bind_texture(const texture_data &texture_info) noexcept;

bool upload_objects(const float_view &vertices, const indices_view &indices,
                    slots &slots, handle &out_handle) noexcept;

[[nodiscard]]
commands_handle upload_draw_commands(const command_view &commands) noexcept;
bool update_draw_commands(
	const command_view &commands, const size_t bytes_offset,
	const renderer::commands_handle &commands_handle) noexcept;

void use_draw_commands(const commands_handle &handle) noexcept;

bool upload_matrices(const float_view &models,
                     matrices_handle &out_models_handle) noexcept;

void use_matrices(const matrices_handle &models_handle) noexcept;

bool add_instance_offset_attr(const handle &handle, slots &slots_used,
                              slots &out_instance_offsets_slot) noexcept;

bool upload_instance_offsets(const float_view &offsets,
                             instance_offsets_handle &out_handle) noexcept;

void use_instance_offsets(
	const handle &handle, const slots &instance_offset_slot,
	const instance_offsets_handle &instance_offsets_handle) noexcept;

bool update_matrices(const matrices_handle &models_handle,
                     const size_t byte_offset,
                     const float_view &new_data) noexcept;

bool upload_instance_offsets(const float_view &offsets,
                             instance_offsets_handle &out_handle) noexcept;

} // namespace renderer
