#pragma once

#include "glm/ext/vector_float3.hpp"
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <string>

namespace statics
{
struct triangle
{
	unsigned int vao;
	glm::vec2 size;
};

struct cuboid
{
	unsigned int vao;
	glm::vec3 size;
};

struct sphere
{
	unsigned int vao;
	unsigned long long nr_indices;
};

struct pyramid
{
	unsigned int vao;
	unsigned int indices;
};

struct instanced_cube
{
	unsigned int vao;
};

bool read_file(const char *file_path, std::string &out_content);

bool could_compile(unsigned int shader, const char *shader_path);

bool create_shader(unsigned int *out_shader_id, const char *shader_path,
                   const unsigned long type);

bool create_shader_program(unsigned int *out_program_id,
                           const unsigned int vertex_shader_id,
                           const unsigned int fragment_shader_id,
                           const unsigned int geometry_shader = -1);

triangle create_triangle(const glm::vec2 size);

void draw_triangle(const unsigned int program, const unsigned int vao);
void draw_rectangle(const unsigned int program, const triangle,
                    const glm::vec2 offset, const glm::vec2 scale,
                    const glm::vec3 color);

void draw_house(const unsigned int program, const triangle tri,
                const glm::vec3 roof_color);

cuboid create_cube(float size);

bool generate_sort_of_sphere(std::vector<float> &vertices,
                             std::vector<unsigned int> &indices, int rings,
                             int slices);

sphere create_sphere();

pyramid create_pyramid();

instanced_cube create_instanced_cubes();

}; // namespace statics
