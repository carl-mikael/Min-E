#pragma once
#include <cstdint>
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
} // namespace renderer
