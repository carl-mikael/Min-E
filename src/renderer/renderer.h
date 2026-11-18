#pragma once
#include <cstdint>
#include <glm/fwd.hpp>
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

bool create_texture(const char *texture_path, uint32_t &out_texture) noexcept;

} // namespace renderer
