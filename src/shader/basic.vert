out vec3 world_pos;
out vec3 normal;
out vec2 uv;

// layout(location = ATTR_POS) in vec4 pos;
layout(location = ATTR_POS) in vec3 vpos;
layout(location = ATTR_NORMAL) in vec3 vnormal;
layout(location = ATTR_UV) in vec2 auv;
layout(location = ATTR_INST_OFFSET) in vec3 vinstance_offset;
layout(std140, binding = BLOCK_MATRICES) uniform Matrices {
    mat4 projection;
    mat4 view;
};

layout(std140, binding = BLOCK_MODELS) uniform Models {
    mat4 models[UBO_MAX_SIZE / MAT4_SIZE];
};

void main()
{
    // gl_Position = projection * view * model * pos;
    gl_Position = projection * view * models[gl_DrawID + gl_InstanceID] * vec4(vpos + vinstance_offset, 1.0);
    world_pos = vec3(models[gl_DrawID + gl_InstanceID] * vec4(vpos, 1.0));
    normal = vnormal;
    uv = auv;
}
