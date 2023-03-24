#version 450

layout (location = 0) in vec3 in_color;

//output write
layout (location = 0) out vec4 out_frag_color;

void main()
{
    //return red
    out_frag_color = vec4(in_color, 1.0f);
}
