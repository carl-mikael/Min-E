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

struct texture_map_ubo
{
	uint32_t handle;
};

struct texture2D
{
	uint32_t handle;
};

struct texture3D
{
	uint32_t handle;
	int width;
	int height;
	int format;
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

[[nodiscard]]
texture_map_ubo create_texture_map_ubo() noexcept;

void upload_texture_map_data(const texture_map_ubo tex_map_ubo,
                             const size_t offset, const size_t bytes,
                             const uint32_t *data) noexcept;

bool create_texture2D(const char *texture_path,
                      texture2D &out_texture_info) noexcept;

void bind_texture2D(const texture2D &texture_info) noexcept;

void create_texture3D(const int width, const int height, const int format,
                      texture3D &out_tex3d) noexcept;

bool upload_texture(const texture3D tex3d, const uint32_t index,
                    const unsigned char *tex_data) noexcept;

bool upload_textures(const texture3D tex3d, const uint32_t offset,
                     const unsigned char **tex_data_arr,
                     const uint32_t count) noexcept;

void bind_texture3D(const texture3D tex3d) noexcept;

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
