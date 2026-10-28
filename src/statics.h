#pragma once

#include <fstream>
#include <iostream>
#include <iterator>

static std::string read_file(const std::string &file_path)
{
#ifndef MINE_LOG
	std::cout << "Opening file" << std::endl;
#endif

	std::ifstream file(file_path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << file_path << std::endl;
		return "";
	}

	std::istreambuf_iterator<char> file_start{file}, end{};
	std::string content{file_start, end};

	return content;
}
