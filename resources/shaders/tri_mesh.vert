#version 450

layout (location = 0) in vec3 v_position;
layout (location = 1) in vec3 v_normal;
layout (location = 2) in vec3 v_color;

layout (location = 0) out vec3 out_color;

void main()
{
    gl_Position = vec4(v_position, 1.0f);
    out_color = v_color;
}