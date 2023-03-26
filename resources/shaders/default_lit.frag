#version 450

layout (location = 0) in vec3 in_color;

//output write
layout (location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 1) uniform SceneData {
    vec4 fog_color;// w is for exponent
    vec4 fog_distances;//x for min, y for max, zw unused.
    vec4 ambient_color;
    vec4 sunlight_direction;//w for sun power
    vec4 sunlight_color;
} sceneData;

void main()
{
    //return red
    out_frag_color = vec4(in_color + sceneData.ambient_color.xyz, 1.0f);
}
