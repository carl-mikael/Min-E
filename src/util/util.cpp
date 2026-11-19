#include "util.h"
#include "../macros.h"
#include <fstream>

bool util::read_file(const char *file_path, std::string &out_content)
{
	std::ifstream file(file_path);
	if (!file.is_open())
	{
		LOG("Failed to open file: " << file_path);
		return false;
	}

	std::istreambuf_iterator<char> file_start{file}, end{};
	out_content = {file_start, end};

	return true;
}
