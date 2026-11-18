#pragma once
#include <cstdint>
#include <glm/fwd.hpp>
#include <string_view>

namespace renderer
{
bool create_program(const std::string_view glsl_semantics,
                    const char *vertex_shader_path,
                    const char *fragment_shader_path, uint32_t &out_program);

bool create_shader(unsigned int *out_shader, const char *shader_path,
                   const unsigned long type,
                   const std::string_view glsl_semantics);

bool could_compile(unsigned int shader, const char *shader_path);
// Technique from: https://iquilezles.org/articles/cppencapsulation/

#define M44Size 64
struct matrix_ubo
{
	uint32_t handle;
	alignas(16) char opaque[M44Size * 2];
};
struct matrix44
{
	alignas(16) char opaque[M44Size];
};

void set_perspective(matrix_ubo &ubo, matrix44 perspective);
void set_view(matrix_ubo &ubo, matrix44 view);
matrix_ubo test();

// void set_view(matrix_ubo &ubo, glm::mat4 view);

uint32_t create_matrix_ubo(void *initial_data, size_t bytes);

bool create_texture(const char *texture_path, uint32_t &out_texture);

} // namespace renderer
