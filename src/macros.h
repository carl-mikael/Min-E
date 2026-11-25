#pragma once

// Turns x into string
#define STR_DEF_(x) #x
// Gets x's value, passes to STR_DEF_ (with only STR_DEF_ it would make the name
// a string, not the value)
#define STR_DEF(x) STR_DEF_(x)

#define GLSL_SEMANTICS_STR STR_DEF(GLSL_SEMANTICS)

#define GLSL_HEADER(semantics)                                                 \
	"#version " STR_DEF(GLSL_VERSION) " core\n" + semantics + "#line 1\n"

#ifdef MINE_LOG
#include <iostream> // IWYU pragma: export
#define LOG(msg)                                                               \
	std::cout << __func__ << " " << __FILE__ << "::" << __LINE__ << ": "       \
			  << msg << std::endl

#define LOG_ARR(msg, base)                                                     \
	std::cout << __func__ << " " << __FILE__ << "::" << __LINE__ << ": "       \
			  << msg << ": ";                                                  \
	for (size_t i{0}; i < sizeof(base) / sizeof(base[0]); ++i)                 \
	{                                                                          \
		std::cout << base[i];                                                  \
	}                                                                          \
	std::cout << std::endl

#define LOG_MEM(msg, base, len)                                                \
	std::cout << __func__ << " " << __FILE__ << "::" << __LINE__ << ": "       \
			  << msg << ": ";                                                  \
	for (int i{0}; i < len; ++i)                                               \
	{                                                                          \
		std::cout << base[i];                                                  \
	}                                                                          \
	std::cout << std::endl

#else
#define LOG(msg)
#define LOG_ARR(msg, base)
#define LOG_MEM(msg, base, len)
#endif

#ifdef DEBUG
#include <intrin.h>
#define GOODERT(condition, msg) /* As in not assert, you feel me? */           \
	if (!condition)                                                            \
	{                                                                          \
		std::cout << __FILE__ << "::" << __LINE__ << " - " << msg              \
				  << std::endl;                                                \
		__debugbreak();                                                        \
	}
#else
#define GOODERT(condition, msg)
#endif

#ifdef DEBUG
#include "glError.h" // IWYU pragma: keep
/*
 * Not really needed, configured debug output for error callbacks:
 * https://learnopengl.com/In-Practice/Debugging#:~:text=your%20debugging%20toolkit.-,Debug%20output,-A%20less%20common
 */
#define glCheckError() glCheckError_(__FILE__, __LINE__)
#else
#define glCheckError()
#endif
