#pragma once

#include <string>

namespace statics
{
bool read_file(const char *file_path, std::string &out_content);

bool could_compile(unsigned int shader, const char *shader_path);

bool create_shader(unsigned int *out_shader_id, const char *shader_path,
                   const unsigned long type);

bool create_shader_program(unsigned int *out_program_id,
                           const unsigned int vertex_shader_id,
                           const unsigned int fragment_shader_id);

void create_triangle_vao(unsigned int *out_vao);

void draw_triangle(const unsigned int program, const unsigned int vao);

}; // namespace statics
