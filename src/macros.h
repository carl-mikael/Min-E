#pragma once
// Turns x into string
#define STR_DEF_(x) #x
// Gets x's value, passes to STR_DEF_ (with only STR_DEF_ it would make the name
// a string, not the value)
#define STR_DEF(x) STR_DEF_(x)

#define GLSL_HEADER                                                            \
	"#version " STR_DEF(GLSL_VERSION) " core\n"                                \
									  "#line 1\n"

#ifdef MINE_LOG
#define LOG(msg) std::cout << msg << std::endl
#else
#define LOG(msg)
#endif
