layout (location=0) in vec3 pos;
uniform vec2 offset;
uniform vec2 scale;

void main()
{
    gl_Position = vec4((pos.xy + offset) * scale, pos.z, 1.0);
}
