#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_texcoord;

layout(set = 0, binding = 1) uniform SceneData {
    vec4 fog_color;// w is for exponent
    vec4 fog_distances;//x for min, y for max, zw unused.
    vec4 ambient_color;
    vec4 sunlight_direction;//w for sun power
    vec4 sunlight_color;
} sceneData;

//output write
layout (location = 0) out vec4 out_frag_color;

layout(set = 2, binding = 0) uniform sampler2D tex1;

void main()
{
    vec3 color = texture(tex1, in_texcoord).rgb;
    float opacity = 1.0;

    if (color.r < 0.1)
    {
        opacity = 0.0;
    }

    out_frag_color = vec4(color.rrr, opacity) * vec4(1 - sceneData.ambient_color.r, 0.0f, 1 - sceneData.ambient_color.b, 1.0f);
}
