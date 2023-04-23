#version 450

layout (location = 0) in vec2 in_texcoords;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 1) uniform SceneData {
    vec4 fog_color;// w is for exponent
    vec4 fog_distances;//x for min, y for max, zw unused.
    vec4 ambient_color;
    vec4 sunlight_direction;//w for sun power
    vec4 sunlight_color;
} sceneData;

layout(set = 1, binding = 0) uniform sampler2D font_atlas_map;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(font_atlas_map, in_texcoords).r);
    out_frag_color = vec4(in_color, 1.0) * sampled;
}