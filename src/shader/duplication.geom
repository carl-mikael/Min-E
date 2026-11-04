layout(triangles) in;
layout(triangle_strip, max_vertices = 6) out;
layout(std140, binding = 0) uniform Matrices {
    mat4 projection;
    mat4 view;
};

uniform mat4 model;
uniform mat4 duplication_model_matrix;

void main()
{
    // Original triangle
    for (int i = 0; i < 3; i++)
    {
        gl_Position = projection * view * model * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();

    // Copied with dub_offset
    for (int i = 0; i < 3; i++)
    {
        gl_Position = projection * view * duplication_model_matrix * gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
