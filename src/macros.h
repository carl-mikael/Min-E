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
#define LOG(msg)                                                               \
	std::cout << __FILE__ << "::" << __func__ << ": " << msg << std::endl
#else
#define LOG(msg)
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
