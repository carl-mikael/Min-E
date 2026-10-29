#pragma once

#include <string>

namespace statics
{
bool read_file(const char *file_path, std::string &out_content);

bool could_compile(unsigned int shader, const char *shader_path);
}; // namespace statics
