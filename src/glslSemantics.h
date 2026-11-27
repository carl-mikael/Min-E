// This file is included by c++ files, and inserted before shaders. To avoid
// hardcoding the values on both sides.

// "Lucky"! the pragma once is ignored by GLSL
#pragma once

#define UBO_MAX_SIZE 16384
#define MAT4_SIZE 64

#define MAX_TEXTURE_LAYERS 2048

// Attributes, OpenGL guarantees that 0-15 exists. Reminder: Mat4 takes up 4.
#define ATTR_POS 0
#define ATTR_NORMAL 1
#define ATTR_UV 2
#define ATTR_INST_OFFSET 15

// Uniform block bindings points, OpenGL guarantees that 0-83 exists.
// Reminders:
//  1. A shader can max reference 14 different blocks.
//  2. A program can max reference a tot of 70 blocks.
//  3. The same block reference in different shaders of the same program count
//  as different references towards the tot.
// Binding point
#define BLOCK_MATRICES 0
#define BLOCK_MODELS 1

// 16 guaranteed texture slots per stage
#define TEX2D 0
#define TEX3D 1
