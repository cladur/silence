#version 450

layout (location = 0) in vec3 in_color;
layout (location = 1) in vec2 in_texcoord;

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

    out_frag_color = vec4(color.rrr, opacity);
}
