// layout(location = ATTR_POS) in vec3 pos;
layout(location = ATTR_POS) in vec4 pos;
layout(location = ATTR_INST_OFFSET) in vec3 instance_offset;
layout(std140, binding = BLOCK_MATRICES) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

void main()
{
    // gl_Position = projection * view * model * vec4(pos + instance_offset, 1.0);
    gl_Position = projection * view * model * pos;
}
