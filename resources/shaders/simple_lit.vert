#version 330 core

layout (location = 0) in vec3 in_pos;

layout (location = 1) in vec3 in_normal;

out vec2 text_coord;
out vec3 out_normal;
out vec3 frag_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(in_pos, 1.0);
    out_normal = in_normal;

    frag_pos = vec3(model * vec4(in_pos, 1.0));
}