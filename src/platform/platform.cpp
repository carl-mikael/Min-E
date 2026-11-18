#include "platform.h"
#include "../glError.h"
#include "../macros.h"
#include "GLFW/glfw3.h"
#include <glad/glad.h>

#ifdef DEBUG
#include <iostream>
#endif

bool platform::init(int window_width, int window_height,
                    window_data &out_window)
{
	if (!glfwInit())
	{
		LOG("glfwInit not working");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR_V);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR_V);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	out_window.platform_window =
		glfwCreateWindow(800, 600, "MinE", nullptr, nullptr);
	glfwGetWindowSize(out_window.platform_window, &out_window.width,
	                  &out_window.height);

	if (!out_window.platform_window)
	{
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(out_window.platform_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		if (glfwGetCurrentContext() == nullptr)
		{
			LOG("You need to call glfwMakeContextCurrent before trying to load "
			    "function pointers");
		}
		else
		{
			LOG("gladLoadGLLoader couldn't load OpenGL function pointers");
		}
		return false;
	}

#ifdef DEBUG
	// Yoinked from: https://learnopengl.com/In-Practice/Debugging
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0,
		                      nullptr, GL_TRUE);
	}
#endif

	return true;
}
