#version 450

//output write
layout (location = 0) out vec4 out_frag_color;

void main()
{
    //return red
    out_frag_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}
