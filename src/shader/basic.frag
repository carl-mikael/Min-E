out vec4 frag_color;
in vec3 world_pos;
in vec3 normal;
in vec2 uv;
flat in int texture_idx;

layout(binding = TEX3D) uniform sampler2DArray textures;

vec3 light_pos = vec3(0, 10, 5);
vec3 light_color = vec3(1);

void main()
{
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(normal);
    vec3 light_dir = normalize(light_pos - world_pos);
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 diffuse = diff * light_color;
    vec3 result = (ambient + diffuse) * texture(textures, vec3(uv, texture_idx)).xyz;
    frag_color = vec4(result, 1.0);
}
