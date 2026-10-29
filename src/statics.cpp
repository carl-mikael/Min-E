#include "statics.h"

#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <iterator>
#include <string>

bool statics::read_file(const char *file_path, std::string &out_content)
{
#ifdef MINE_LOG
	std::cout << "Opening file" << std::endl;
#endif

	std::ifstream file(file_path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << file_path << std::endl;
		return false;
	}

	std::istreambuf_iterator<char> file_start{file}, end{};
	out_content = {file_start, end};

	return true;
}

bool statics::could_compile(unsigned int shader, const char *shader_path)
{
	int success;
	char info_log[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, sizeof(info_log), nullptr, info_log);
		std::cerr << "Failed to compile shader: " << shader_path << std::endl;

		return false;
	}

#ifdef MINE_LOG
	std::cout << "Successfully compiled: " << shader_path << std::endl;
#endif

	return true;
}
