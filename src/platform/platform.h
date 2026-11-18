#pragma once

struct GLFWwindow;

namespace platform
{
struct window_data
{
	int width, height;
	GLFWwindow *platform_window;
};

bool init(int width, int height, window_data &out_window);
} // namespace platform
