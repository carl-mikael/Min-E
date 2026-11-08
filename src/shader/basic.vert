layout(location = ATTR_POS) in vec3 pos;
layout(location = ATTR_INST_OFFSET) in vec3 instance_offset;
layout(std140, binding = BLOCK_MATRICES) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

void main()
{
    gl_Position = vec4(pos + instance_offset, 1.0);
}
