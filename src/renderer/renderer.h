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

bool could_compile(unsigned int shader, const char *shader_path) noexcept;

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
} // namespace renderer
