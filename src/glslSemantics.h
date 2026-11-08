// This file is included by c++ files, and inserted before shaders. To avoid
// hardcoding the values on both sides.

// "Lucky"! the pragma once is ignored by GLSL
#pragma once

// Attributes, OpenGL guarantees that 0-15 exists. Reminder: Mat4 takes up 4.
#define ATTR_POS 0
#define ATTR_INST_OFFSET 1

// Uniform block bindings points, OpenGL guarantees that 0-83 exists.
// Reminders:
//  1. A shader can max reference 14 different blocks.
//  2. A program can max reference a tot of 70 blocks.
//  3. The same block reference in different shaders of the same program count
//  as different references towards the tot.
#define BLOCK_MATRICES 0
