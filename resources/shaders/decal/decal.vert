#version 330 core
layout (location = 0) in vec3 aPos;

out vec4 clip_pos;

uniform mat4 view_proj;
uniform mat4 decal_inv_view_proj;

void main()
{
    vec4 world_pos = decal_inv_view_proj * vec4(aPos, 1.0);
    gl_Position = view_proj * world_pos;
    clip_pos = gl_Position;
}